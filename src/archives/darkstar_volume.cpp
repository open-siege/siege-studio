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
  std::size_t get_file_list_offsets(std::basic_ifstream<std::byte>& raw_data)
  {
    volume_header header;

    raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

    if (header.file_tag == vol_file_tag)
    {
      normal_footer footer;
      raw_data.seekg(header.footer_offset, std::ios_base::beg);
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      return footer.file_list_size;
    }
    else if (header.file_tag == alt_vol_file_tag)
    {
      alternative_footer footer;
      raw_data.seekg(header.footer_offset, std::ios_base::beg);
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      return footer.file_list_size;
    }
    else
    {
      throw std::invalid_argument("The file provided is not a valid Darkstar VOL file.");
    }
  }


  std::vector<std::string> get_file_names(std::basic_ifstream<std::byte>& raw_data)
  {
    auto buffer_size = get_file_list_offsets(raw_data);
    std::vector<char> raw_chars(buffer_size);

    if ((buffer_size) % 2 != 0)
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

    return results;
  }

  std::vector<file_info> get_file_metadata(std::basic_ifstream<std::byte>& raw_data)
  {
    auto filenames = get_file_names(raw_data);
    file_index_header header;
    raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

    std::vector<std::byte> raw_bytes(header.index_size);
    raw_data.read(raw_bytes.data(), raw_bytes.size());

    std::size_t index = 0;

    std::vector<file_info> results;
    results.reserve(filenames.size());

    while (index < raw_bytes.size())
    {
      file_header file;
      std::copy(raw_bytes.data() + index, raw_bytes.data() + index + sizeof(file_header), reinterpret_cast<std::byte*>(&file));

      file_info info;

      info.filename = std::move(filenames[results.size()]);
      info.size = file.size;
      info.offset = file.offset;
      info.compression_type = file.compression_type;
      results.emplace_back(info);
      index += sizeof(file_header);
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

      if (std::filesystem::file_size(new_path) > some_file.size)
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