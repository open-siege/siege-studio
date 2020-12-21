#include <fstream>
#include <filesystem>
#include <vector>
#include <array>
#include <utility>
#include <string>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "archives/darkstar_volume.hpp"

namespace darkstar::vol
{
  std::tuple<volume_version, std::size_t, std::optional<std::size_t>> get_file_list_offsets(std::basic_ifstream<std::byte>& raw_data)
  {
    volume_header header{};

    raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

    if (header.file_tag == vol_file_tag)
    {
      normal_footer footer{};
      raw_data.seekg(header.footer_offset, std::ios_base::beg);
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      return std::make_tuple(volume_version::darkstar_vol, footer.file_list_size, std::nullopt);
    }
    else if (header.file_tag == alt_vol_file_tag)
    {
      alternative_footer footer{};
      raw_data.seekg(header.footer_offset, std::ios_base::beg);
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      return std::make_tuple(volume_version::darkstar_pvol, footer.file_list_size, std::nullopt);
    }
    else if (header.file_tag == old_vol_file_tag)
    {
      old_footer footer{};
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      auto amount_to_skip = footer.buffer_size - sizeof(int32_t) - footer.file_list_size;

      return std::make_tuple(volume_version::three_space_vol, footer.file_list_size, amount_to_skip);
    }
    else
    {
      throw std::invalid_argument("The file provided is not a valid Darkstar VOL file.");
    }
  }


  std::pair<volume_version, std::vector<std::string>> get_file_names(std::basic_ifstream<std::byte>& raw_data)
  {
    auto [volume_type, buffer_size, amount_to_skip] = get_file_list_offsets(raw_data);
    std::vector<char> raw_chars(buffer_size);

    if (volume_type != volume_version::three_space_vol && (buffer_size) % 2 != 0)
    {
      raw_data.seekg(1, std::ios::cur);
    }

    raw_data.read(reinterpret_cast<std::byte*>(raw_chars.data()), raw_chars.size());

    std::vector<std::string> results;

    std::size_t index = 0;

    while (index < raw_chars.size())
    {
      results.emplace_back(raw_chars.data() + index);

      index += results.back().size() + 1;
    }

    if (amount_to_skip.has_value())
    {
      raw_data.seekg(amount_to_skip.value(), std::ios::cur);
    }

    return std::make_pair(volume_type, results);
  }

  std::vector<file_info> get_file_metadata(std::basic_ifstream<std::byte>& raw_data)
  {
    auto [volume_type, filenames] = get_file_names(raw_data);
    file_index_header header{};

    if (volume_type == volume_version::three_space_vol)
    {
      old_volume_header raw_header{};
      raw_data.read(reinterpret_cast<std::byte*>(&raw_header), sizeof(raw_header));
      header.index_tag = raw_header.file_tag;
      header.index_size = raw_header.footer_offset;
    }
    else
    {
      raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));
    }

    std::vector<std::byte> raw_bytes(header.index_size);
    raw_data.read(raw_bytes.data(), raw_bytes.size());

    std::size_t index = 0;

    std::vector<file_info> results;
    results.reserve(filenames.size());

    while (index < raw_bytes.size())
    {
      endian::little_uint32_t offset;
      endian::little_uint32_t size;
      compression_type compression_type;

      if (volume_type == volume_version::three_space_vol)
      {
        old_file_header file{};

        std::copy(raw_bytes.data() + index,
                  raw_bytes.data() + index + sizeof(old_file_header),
                  reinterpret_cast<std::byte*>(&file));

        offset = file.offset;
        size = file.size;
        compression_type = file.compression_type == 1 ? darkstar::vol::compression_type::none : darkstar::vol::compression_type::lz;

        index += sizeof(old_file_header);
      }
      else
      {
        file_header file{};
        std::copy(raw_bytes.data() + index,
                  raw_bytes.data() + index + sizeof(file_header),
                  reinterpret_cast<std::byte*>(&file));
        offset = file.offset;
        size = file.size;
        compression_type = file.compression_type;

        index += sizeof(file_header);
      }

      file_info info;

      info.filename = std::move(filenames[results.size()]);
      info.offset = offset;
      info.size = size;
      info.compression_type = compression_type;
      results.emplace_back(info);

      if (results.size() == filenames.size())
      {
        break;
      }
    }

    return results;
  }

  void extract_files(std::basic_ifstream<std::byte>& volume, std::string_view volume_filename, std::string_view output_dir, file_info& some_file)
  {
    auto new_path = std::filesystem::path(output_dir) / some_file.filename;

    if (some_file.compression_type == darkstar::vol::compression_type::none)
    {
      std::cout << "Extracting " << some_file.filename << " to " << new_path << '\n';
      auto new_file = std::basic_ofstream<std::byte>{ new_path, std::ios::binary };

      volume.seekg(some_file.offset + sizeof(darkstar::vol::volume_header), std::ios::beg);
      std::copy_n(std::istreambuf_iterator<std::byte>(volume),
                  some_file.size,
                  std::ostreambuf_iterator<std::byte>(new_file));
    }
    else
    {
      std::stringstream command;

      command << "extract.exe" << ' ' << volume_filename << ' ' << some_file.filename << ' ' << new_path;

      std::cout << "Executing " << command.str() << '\n';
      std::system(command.str().c_str());

      if (std::filesystem::exists(new_path) && std::filesystem::file_size(new_path) > some_file.size)
      {
        auto old_path = new_path.string() + ".old";
        std::filesystem::rename(new_path, old_path);

        {
          auto old_file = std::basic_ifstream<std::byte>{ old_path, std::ios::binary };

          auto new_file = std::basic_ofstream<std::byte>{ new_path, std::ios::binary };

          std::copy_n(std::istreambuf_iterator<std::byte>(old_file),
                      some_file.size,
                      std::ostreambuf_iterator<std::byte>(new_file));
        }

        std::filesystem::remove(old_path);
      }
    }
  }
}