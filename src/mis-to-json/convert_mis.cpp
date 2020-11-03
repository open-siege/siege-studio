#include "archives/darkstar_volume.hpp"
#include <fstream>
#include <variant>
#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>
#include <functional>

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template<std::size_t Length>
std::string hex_str(std::array<std::byte, Length>& data)
{
  std::stringstream ss;
  ss << std::hex;

  for (auto item : data)
  {
    ss << std::setw(2) << std::setfill('0') << static_cast<std::uint16_t>(item);
  }

  return ss.str();
}

namespace darkstar
{
  constexpr auto sim_group_tag = to_tag({ 'S', 'I', 'M', 'G' });
  constexpr auto sim_set_tag = to_tag({ 'S', 'I', 'M', 'S' });
  constexpr auto sim_vol_tag = to_tag({ 'S', 'V', 'o', 'l' });
  constexpr auto sim_terrain_tag = to_tag({ 'S', 'T', 'E', 'R' });
  constexpr auto es_palette_tag = to_tag({ 'E', 'S', 'p', 't' });
  constexpr auto interior_shape_tag = to_tag({ 0x0c, 0x01, 0x00, 0x00 });
  constexpr auto sim_structure_tag = to_tag({ 0x00, 0x01, 0x00, 0x00 });

  struct sim_set;
  struct sim_group;
  struct raw_item;
  struct sim_volume;
  struct sim_terrain;
  struct sim_palette;
  struct interior_shape;
  struct sim_structure;

  using sim_item = std::variant<sim_set, sim_group, sim_volume, sim_terrain, sim_palette, interior_shape, sim_structure, raw_item>;

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
    endian::little_uint32_t version;
    endian::little_uint32_t children_count;
    sim_items children;
    std::vector<std::string> names;
  };

  struct address
  {
    endian::little_uint32_t reserved;
    endian::little_uint32_t object_id;
    endian::little_uint32_t manager_id;
  };

  struct sim_network_object
  {
    endian::little_uint32_t version;
    endian::little_uint32_t id;
    endian::little_uint32_t flags;
    address object_address;
  };

  struct sim_volume
  {
    volume_header header;
    sim_network_object base;
    std::string filename;
  };

  struct sim_terrain
  {
    volume_header header;
    sim_network_object base;
    std::array<std::byte, 48> data;
    std::string dtf_file;
    std::array<std::byte, 20> footer;
  };

  struct sim_palette
  {
    volume_header header;
    sim_network_object base;
    std::string palette_file;
  };


  struct interior_shape
  {
    volume_header header;
    endian::little_uint32_t version;
    std::array<std::byte, 124> data;
    endian::little_uint32_t string_length;
    std::string filename;
    std::array<std::byte, 22> footer;
  };

  struct sim_structure
  {
    volume_header header;
    endian::little_uint32_t version;
    std::array<std::byte, 117> data;
    endian::little_uint32_t string_length;
    std::string filename;
  };

  struct raw_item
  {
    volume_header header;
    std::vector<std::byte> raw_bytes;
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

    file.read(reinterpret_cast<std::byte*>(&child_header), sizeof(child_header));

    auto reader = readers.find(child_header.file_tag);

    if (reader != readers.end())
    {
      children.emplace_back(reader->second.read(file, child_header, readers));
      continue;
    }

    darkstar::raw_item item;

    item.header = child_header;
    item.raw_bytes = std::vector<std::byte>(item.header.footer_offset);
    file.read(item.raw_bytes.data(), item.header.footer_offset);

    children.emplace_back(std::move(item));

    skip_alignment_bytes(file, item.header.footer_offset);
  }

  return children;
}

std::string read_string(std::basic_ifstream<std::byte>& file)
{
  std::uint8_t size;
  file.read(reinterpret_cast<std::byte*>(&size), sizeof(std::uint8_t));

  std::string name(size, '\0');

  file.read(reinterpret_cast<std::byte*>(name.data()), size);

  return name;
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

darkstar::sim_network_object read_sim_network_object(std::basic_ifstream<std::byte>& file)
{
  darkstar::sim_network_object object;

  file.read(reinterpret_cast<std::byte*>(&object), sizeof(object));

  return object;
}

darkstar::sim_volume read_sim_volume(std::basic_ifstream<std::byte>& file, darkstar::volume_header& header, darkstar::sim_item_reader_map&)
{
  darkstar::sim_volume volume;
  volume.header = header;
  volume.base = read_sim_network_object(file);
  volume.filename = read_string(file);

  skip_alignment_bytes(file, header.footer_offset);

  return volume;
}

darkstar::sim_terrain read_sim_terrain(std::basic_ifstream<std::byte>& file, darkstar::volume_header& header, darkstar::sim_item_reader_map&)
{
  darkstar::sim_terrain volume;
  volume.header = header;
  volume.base = read_sim_network_object(file);

  file.read(volume.data.data(), volume.data.size());
  volume.dtf_file = read_string(file);
  file.read(volume.footer.data(), volume.footer.size());

  skip_alignment_bytes(file, header.footer_offset);

  return volume;
}

darkstar::sim_palette read_sim_palette(std::basic_ifstream<std::byte>& file, darkstar::volume_header& header, darkstar::sim_item_reader_map&)
{
  darkstar::sim_palette palette;
  palette.header = header;
  palette.base = read_sim_network_object(file);

  palette.palette_file = read_string(file);

  skip_alignment_bytes(file, header.footer_offset);

  return palette;
}

darkstar::interior_shape read_interior_shape(std::basic_ifstream<std::byte>& file, darkstar::volume_header& header, darkstar::sim_item_reader_map&)
{
  darkstar::interior_shape shape;

  shape.header = header;
  file.read(reinterpret_cast<std::byte*>(&shape.version), sizeof(shape.version));
  file.read(shape.data.data(), shape.data.size());
  file.read(reinterpret_cast<std::byte*>(&shape.string_length), sizeof(shape.string_length));

  shape.filename = std::string(shape.string_length, '\0');

  file.read(reinterpret_cast<std::byte*>(shape.filename.data()), shape.filename.size());

  file.read(shape.footer.data(), shape.footer.size());

  skip_alignment_bytes(file, header.footer_offset);

  return shape;
}

darkstar::sim_group read_sim_group(std::basic_ifstream<std::byte>& file, darkstar::volume_header& header, darkstar::sim_item_reader_map& readers)
{
  darkstar::sim_group group;
  group.header = header;

  file.read(reinterpret_cast<std::byte*>(&group.version), sizeof(group.version));
  file.read(reinterpret_cast<std::byte*>(&group.children_count), sizeof(group.children_count));

  group.children = read_children(file, group.children_count, readers);
  group.names = read_strings(file, group.children_count);
  skip_alignment_bytes(file, header.footer_offset);

  return group;
}

darkstar::sim_set read_sim_set(std::basic_ifstream<std::byte>& file, darkstar::volume_header& header, darkstar::sim_item_reader_map& readers)
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
    { sim_group_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_group(file, header, readers); } } },
    { sim_set_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_set(file, header, readers); } } },
    { sim_vol_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_volume(file, header, readers); } } },
    { sim_terrain_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_terrain(file, header, readers); } } },
    { es_palette_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_palette(file, header, readers); } } },
    { interior_shape_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_interior_shape(file, header, readers); } } }
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
                 [&](darkstar::sim_volume& item) {
                   std::cout << "Volume file is has: " << item.filename << '\n';
                 },
                 [&](darkstar::sim_terrain& item) {
                   std::cout << "Volume file is has: " << item.dtf_file << '\n';
                 },
                 [&](darkstar::sim_palette& item) {
                   std::cout << "Volume file is has: " << item.palette_file << '\n';
                 },
                 [&](darkstar::interior_shape& item) {
                   std::cout << "Volume file is has: " << item.filename << '\n';
                 },
                 [&](darkstar::sim_group& item) {
                   for (auto i = 0u; i < item.children.size(); ++i)
                   {
                     std::cout << item.names[i] << " ";

                     std::visit([&](auto& value) {
                       std::cout
                         << hex_str(value.header.file_tag) << " "
                         << std::string_view(reinterpret_cast<char*>(value.header.file_tag.data()), value.header.file_tag.size());
                     },
                       item.children[i]);

                     std::cout << "\n";
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