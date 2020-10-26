#include "archives/darkstar_volume.hpp"
#include <fstream>
#include <variant>
#include <iostream>
#include <map>
#include <functional>

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace darkstar
{
  constexpr auto sim_group_tag = to_tag({ 'S', 'I', 'M', 'G' });
  constexpr auto sim_set_tag = to_tag({ 'S', 'I', 'M', 'S' });

  struct sim_set;
  struct sim_group;


  using sim_item = std::variant<sim_set, sim_group, std::vector<std::byte>>;

  using sim_items = std::vector<sim_item>;

  struct sim_item_reader;

  using sim_item_reader_map = std::map<std::array<std::byte, 4>, darkstar::sim_item_reader>;

  struct sim_item_reader
  {
    darkstar::sim_item (*read)(std::basic_ifstream<std::byte>&, darkstar::volume_header&, sim_item_reader_map&);
  };


  struct sim_set
  {
    volume_header header;
    endian::little_uint32_t item_id;
    endian::little_uint32_t children_count;
    sim_items children;
  };

  struct sim_group
  {
    volume_header header;
    endian::little_uint32_t item_id;
    endian::little_uint32_t children_count;
    sim_items children;
    std::vector<std::string> names;
  };
}// namespace darkstar


void skip_alignment_bytes(std::basic_ifstream<std::byte>& file, std::uint32_t base)
{
  int offset = 0;
  while ((base + offset) % 2 != 0)
  {
    offset++;
  }

  if (offset > 0)
  {
    file.seekg(offset, std::ios_base::cur);
  }
}

darkstar::sim_items read_children(std::basic_ifstream<std::byte>& file, std::uint32_t children_count, darkstar::sim_item_reader_map& readers)
{
  using volume_header = darkstar::volume_header;
  namespace endian = boost::endian;
  darkstar::sim_items children;

  children.reserve(children_count);

  for (auto i = 0u; i < children_count; ++i)
  {
    volume_header child_header;
    std::byte* header_raw = reinterpret_cast<std::byte*>(&child_header);

    file.read(header_raw, sizeof(child_header));

    auto reader = readers.find(child_header.file_tag);

    if (reader != readers.end())
    {
      children.emplace_back(reader->second.read(file, child_header, readers));
      continue;
    }

    std::vector<std::byte> bytes(child_header.footer_offset + sizeof(child_header));

    std::copy(header_raw, header_raw + sizeof(child_header), bytes.data());

    file.read(bytes.data() + sizeof(child_header), child_header.footer_offset);

    children.emplace_back(std::move(bytes));

    skip_alignment_bytes(file, child_header.footer_offset);
  }

  return children;
}

std::vector<std::string> read_strings(std::basic_ifstream<std::byte>& file, std::uint32_t children_count)
{
  std::vector<std::string> results;
  results.reserve(children_count);

  for (auto i = 0u; i < children_count; ++i)
  {
    std::uint8_t size;
    file.read(reinterpret_cast<std::byte*>(&size), sizeof(std::uint8_t));

    std::string& name = results.emplace_back(size, '\0');

    file.read(reinterpret_cast<std::byte*>(name.data()), size);
  }

  return results;
}

darkstar::sim_item read_sim_group(std::basic_ifstream<std::byte>& file, darkstar::volume_header& header, darkstar::sim_item_reader_map& readers)
{
  darkstar::sim_group group;
  group.header = header;

  file.read(reinterpret_cast<std::byte*>(&group.item_id), sizeof(group.item_id));
  file.read(reinterpret_cast<std::byte*>(&group.children_count), sizeof(group.children_count));

  group.children = read_children(file, group.children_count, readers);
  group.names = read_strings(file, group.children_count);
  skip_alignment_bytes(file, header.footer_offset);

  return group;
}

darkstar::sim_item read_sim_set(std::basic_ifstream<std::byte>& file, darkstar::volume_header& header, darkstar::sim_item_reader_map& readers)
{
  darkstar::sim_set set;
  set.header = header;
  file.read(reinterpret_cast<std::byte*>(&set.item_id), sizeof(set.item_id));
  file.read(reinterpret_cast<std::byte*>(&set.children_count), sizeof(set.children_count));
  set.children = read_children(file, set.children_count, readers);
  skip_alignment_bytes(file, header.footer_offset);

  return set;
}

int main(int, const char** argv)
{
  using namespace darkstar;
  namespace endian = boost::endian;
  auto file = std::basic_ifstream<std::byte>{ argv[1], std::ios::binary };

  static sim_item_reader_map readers = {
    { sim_group_tag, { read_sim_group } },
    { sim_set_tag, { read_sim_set } }
  };

  auto results = read_children(file, 1, readers);

  std::function<void(darkstar::sim_item&)> visit_item = [&](auto& result) {
    std::visit(overloaded{ [&](darkstar::sim_set& item) {
                            std::cout << "set has: " << item.children_count;

                            for (auto& child : item.children)
                            {
                              visit_item(child);
                            }
                          },
                 [&](darkstar::sim_group& item) {
                   for (auto& name : item.names)
                   {
                     std::cout << name << "\n";
                   }

                   for (auto& child : item.children)
                   {
                     visit_item(child);
                   }
                 },
                 [](auto&) {

                 } },
      result);
  };

  for (auto& result : results)
  {
    visit_item(result);
  }
}