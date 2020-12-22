#ifndef DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
#define DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>
#include <string>

#include "endian_arithmetic.hpp"

namespace three_space::vol
{
  namespace endian = boost::endian;

  void get_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::array<std::byte, 6> header{};
    raw_data.read(header.data(), sizeof(header));

    auto volume_count = static_cast<int>(header[header.size() - 2]);

    std::basic_string<std::byte> filename;
    filename.reserve(12);
    std::cout << volume_count << " volumes are present" << '\n';

    for (auto i = 0; i < volume_count; ++i)
    {
      std::getline(raw_data, filename, std::byte{ '\0' });

      raw_data.seekg(1, std::ios::cur);

      endian::little_uint16_t count_chunk{};

      raw_data.read(reinterpret_cast<std::byte*>(&count_chunk), sizeof(count_chunk));

      auto file_count = int(reinterpret_cast<const std::uint8_t*>(count_chunk.data())[1]);

      if (header[0] == std::byte{ 1 } && header[1] == std::byte{ 0 })
      {
        file_count = count_chunk;
      }

      auto volume = std::basic_ifstream<std::byte>{ std::string_view{ reinterpret_cast<const char*>(filename.c_str()), filename.size() }, std::ios::binary };

      std::cout << reinterpret_cast<const char*>(filename.c_str()) << " has " << int(file_count) << " files" << '\n';

      std::array<char, 13> child_filename{ '\0' };

      for (auto x = 0; x < file_count; ++x)
      {
        volume.read(reinterpret_cast<std::byte*>(child_filename.data()), child_filename.size() - 1);

        volume.seekg(1, std::ios::cur);

        endian::little_uint32_t file_size{};

        volume.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));

        auto new_path = std::filesystem::path("volume") / child_filename.data();
        std::cout << std::string(child_filename.data()) << '\n';

        auto new_file = std::basic_ofstream<std::byte>{ new_path, std::ios::binary };

        auto current_position = volume.tellg();
        std::copy_n(std::istreambuf_iterator<std::byte>(volume),
          file_size,
          std::ostreambuf_iterator<std::byte>(new_file));

        volume.seekg(static_cast<int>(current_position) + file_size, std::ios::beg);
      }

      raw_data.seekg(file_count * sizeof(std::array<std::byte, 8>) + 1, std::ios::cur);
      filename.clear();
    }
  }

  void get_dyn_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::array<char, 20> header{};
    raw_data.read(reinterpret_cast<std::byte*>(header.data()), sizeof(header));
    raw_data.seekg(12, std::ios::cur);

    endian::little_uint32_t file_count{};

    raw_data.read(reinterpret_cast<std::byte*>(&file_count), sizeof(file_count));

    std::cout << int(file_count) << " files present." << '\n';

    raw_data.seekg(file_count * sizeof(std::array<std::byte, 4>), std::ios::cur);

    std::array<char, 13> child_filename{ '\0' };

    for (auto x = 0u; x < file_count; ++x)
    {
      raw_data.read(reinterpret_cast<std::byte*>(child_filename.data()), child_filename.size());

      endian::little_uint32_t file_size{};

      raw_data.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));

      auto new_path = std::filesystem::path("volume") / child_filename.data();
      std::cout << std::string(child_filename.data()) << '\n';

      auto new_file = std::basic_ofstream<std::byte>{ new_path, std::ios::binary };

      auto current_position = static_cast<int>(raw_data.tellg());
      std::copy_n(std::istreambuf_iterator<std::byte>(raw_data),
        file_size,
        std::ostreambuf_iterator<std::byte>(new_file));

      current_position = current_position + file_size;

      raw_data.seekg(current_position, std::ios::beg);

      while ((current_position % 4) != 0)
      {
        current_position++;
        raw_data.seekg(1, std::ios::cur);
      }
    }
  }

  void get_vol_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::array<char, 4> header{};
    raw_data.read(reinterpret_cast<std::byte*>(header.data()), sizeof(header));

    raw_data.seekg(6, std::ios::cur);

    endian::little_uint16_t num_chars{};

    raw_data.read(reinterpret_cast<std::byte*>(&num_chars), sizeof(num_chars));

    std::vector<char> raw_folders(num_chars, '\0');
    raw_data.read(reinterpret_cast<std::byte*>(raw_folders.data()), raw_folders.size());

    std::vector<std::string_view> folders;
    folders.reserve(raw_folders.size() / 5);

    size_t i = 0u;
    while (i < raw_folders.size())
    {
      folders.emplace_back(raw_folders.data() + i);
      i += strlen(raw_folders.data() + i) + 1;
    }

    std::array<endian::little_uint16_t, 3> info{};

    raw_data.read(reinterpret_cast<std::byte*>(info.data()), sizeof(info));

    const auto start_index = int(raw_data.tellg()) - sizeof(endian::little_uint16_t);
    const auto end_index = start_index + info[1];

    std::vector<std::tuple<std::array<char, 14>, std::uint8_t, endian::little_uint32_t>> files;

    std::array<char, 14> filename{'\0'};
    while (int(raw_data.tellg()) < end_index)
    {
      raw_data.read(reinterpret_cast<std::byte*>(filename.data()), filename.size() - 1);

      std::cout << "Filename is " << filename.data() << '\n';

      std::uint8_t folder_index;
      raw_data.read(reinterpret_cast<std::byte*>(&folder_index), sizeof(folder_index));

      endian::little_uint32_t file_index;
      raw_data.read(reinterpret_cast<std::byte*>(&file_index), sizeof(file_index));

      files.emplace_back(filename, folder_index, file_index);
    }

    raw_data.seekg(1, std::ios::cur);

    std::cout << "Number of files are " << files.size() << '\n';

    for (auto& file : files)
    {
      auto& [child_filename, folder_index, file_index] = file;
      auto& folder_name = folders[folder_index];

      raw_data.seekg(file_index, std::ios::beg);

      std::byte entry{};
      raw_data.read(&entry, sizeof(entry));

      std::array<endian::little_uint32_t, 2> file_info{};
      raw_data.read(reinterpret_cast<std::byte*>(&file_info), sizeof(file_info));

      auto new_path = std::filesystem::path("volume") / folder_name / child_filename.data();

      std::filesystem::create_directory(std::filesystem::path("volume") / folder_name);

      std::cout << "Extracting " << child_filename.data() << " to " << new_path << '\n';
      auto new_file = std::basic_ofstream<std::byte>{ new_path, std::ios::binary };

      std::copy_n(std::istreambuf_iterator<std::byte>(raw_data),
                  file_info[0],
                  std::ostreambuf_iterator<std::byte>(new_file));
    }
  }


}// namespace three_space::vol

#endif//DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
