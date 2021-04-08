#ifndef INC_3SPACESTUDIO_TAGGED_DATA_HPP
#define INC_3SPACESTUDIO_TAGGED_DATA_HPP

#include <array>
#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include "endian_arithmetic.hpp"

namespace studio
{
  namespace endian = boost::endian;

  struct object_header
  {
    std::array<std::byte, 4> object_tag;
    endian::little_uint32_t object_size;
  };

  struct raw_item
  {
    object_header header;
    std::vector<std::byte> raw_bytes;
  };

  inline void skip_alignment_bytes(std::basic_istream<std::byte>& file, std::uint32_t base)
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

  inline std::string read_string(std::basic_istream<std::byte>& file, std::size_t size)
  {
    std::string name(size, '\0');

    file.read(reinterpret_cast<std::byte*>(name.data()), size);

    return name;
  }

  inline std::string read_string(std::basic_istream<std::byte>& file)
  {
    std::uint8_t size;
    file.read(reinterpret_cast<std::byte*>(&size), sizeof(std::uint8_t));

    std::string name(size, '\0');

    file.read(reinterpret_cast<std::byte*>(name.data()), size);

    return name;
  }

  inline std::vector<std::string> read_strings(std::basic_istream<std::byte>& file, std::uint32_t children_count)
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

  template <typename ChildType>
  struct tagged_item_reader;

  template <typename ChildType>
  struct tagged_item_map
  {
    using tagged_item_reader_map = std::map<std::array<std::byte, 4>, tagged_item_reader<ChildType>>;
  };

  template <typename ChildType>
  struct tagged_item_reader
  {
    ChildType (*read)(std::basic_istream<std::byte>&, object_header&, typename tagged_item_map<ChildType>::tagged_item_reader_map&);
  };

  template <typename ChildType>
  std::vector<ChildType> read_children(std::basic_istream<std::byte>& file,
    std::uint32_t children_count,
    typename tagged_item_map<ChildType>::tagged_item_reader_map& readers)
  {
    namespace endian = boost::endian;
    std::vector<ChildType> children;

    children.reserve(children_count);

    for (auto i = 0u; i < children_count; ++i)
    {
      object_header child_header;

      file.read(reinterpret_cast<std::byte*>(&child_header), sizeof(child_header));

      auto reader = readers.find(child_header.object_tag);

      if (reader != readers.end())
      {
        children.emplace_back(reader->second.read(file, child_header, readers));
        continue;
      }

      raw_item item;

      std::cout << "Read " << std::string_view(reinterpret_cast<char*>(child_header.object_tag.data()), 4) << "@" << int(file.tellg()) - sizeof(object_header) << '\n';

      item.header = child_header;
      item.raw_bytes = std::vector<std::byte>(item.header.object_size);
      file.read(item.raw_bytes.data(), item.header.object_size);

      children.emplace_back(std::move(item));

      skip_alignment_bytes(file, item.header.object_size);
    }

    return children;
  }
}

#endif//INC_3SPACESTUDIO_TAGGED_DATA_HPP
