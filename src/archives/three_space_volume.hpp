#ifndef DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
#define DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>
#include <string>

#include "archive.hpp"
#include "endian_arithmetic.hpp"

namespace three_space::vol
{
  namespace endian = boost::endian;

  constexpr auto rmf_tags = std::array<std::array<std::byte, 4>, 8>{
    shared::to_tag<4>({ 0x00, 0x01, 0x05, 0x07 }),
    shared::to_tag<4>({ 0x00, 0x01, 0x06, 0x07 }),
    shared::to_tag<4>({ 0x00, 0x02, 0x04, 0x07 }),
    shared::to_tag<4>({ 0x01, 0x00, 0x00, 0x00 }),
    shared::to_tag<4>({ 0x01, 0x04, 0x06, 0x07 }),
    shared::to_tag<4>({ 0x00, 0x01, 0x05, 0x06 }),
    shared::to_tag<4>({ 0x00, 0x01, 0x04, 0x07 }),
    shared::to_tag<4>({ 0x03, 0x04, 0x05, 0x07 })
  };

  struct rmf_file_header
  {
    endian::little_int32_t checksum;
    endian::little_int32_t offset;
  };

  std::vector<shared::archive::folder_info> get_rmf_sub_archives(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 6> header{};
    raw_data.read(header.data(), sizeof(header));

    auto volume_count = static_cast<int>(header[header.size() - 2]);

    std::array<char, 14> filename{ '\0' };

    std::vector<shared::archive::folder_info> results;
    results.reserve(volume_count);

    for (auto i = 0; i < volume_count; ++i)
    {
      raw_data.read(reinterpret_cast<std::byte*>(filename.data()), sizeof(filename) - 1);

      endian::little_uint16_t file_count{};
      raw_data.read(reinterpret_cast<std::byte*>(&file_count), sizeof(file_count));

      shared::archive::folder_info info{};
      info.name = filename.data();
      info.file_count = file_count;

      results.emplace_back(info);

      raw_data.seekg(file_count * sizeof(rmf_file_header), std::ios::cur);
    }

    return results;
  }

  std::vector<shared::archive::file_info> get_rmf_data(std::basic_istream<std::byte>& raw_data, const std::filesystem::path& archive_path)
  {
    std::vector<shared::archive::file_info> results;
    std::array<std::byte, 6> header{};
    raw_data.read(header.data(), sizeof(header));

    auto volume_count = static_cast<int>(header[header.size() - 2]);

    std::array<char, 14> filename{ '\0' };
    std::cout << volume_count << " volumes are present" << '\n';

    auto real_path = archive_path.parent_path().parent_path();

    auto map_filename = archive_path.parent_path().filename().string();

    auto volume_filename = archive_path.filename().string();

    for (auto i = 0; i < volume_count; ++i)
    {
      raw_data.read(reinterpret_cast<std::byte*>(filename.data()), sizeof(filename) - 1);

      endian::little_uint16_t file_count{};
      raw_data.read(reinterpret_cast<std::byte*>(&file_count), sizeof(file_count));

      if (volume_filename == std::string_view(filename.data()))
      {
        std::vector<rmf_file_header> headers(file_count, rmf_file_header{});

        raw_data.read(reinterpret_cast<std::byte*>(headers.data()), file_count * sizeof(rmf_file_header));

        std::sort(headers.begin(), headers.end(), [](const auto& a, const auto& b) {
               return a.offset < b.offset;
        });

        results.reserve(file_count);

        auto volume = std::basic_ifstream<std::byte>{ real_path / volume_filename, std::ios::binary };

        std::array<char, 14> child_filename{ '\0' };

        for (auto x = 0; x < file_count; ++x)
        {
          auto& file_header = headers[x];

          volume.seekg(file_header.offset, std::ios::beg);

          volume.read(reinterpret_cast<std::byte*>(child_filename.data()), child_filename.size() - 1);
          endian::little_uint32_t file_size{};
          volume.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));

          shared::archive::file_info info{};
          info.offset = file_header.offset;
          info.filename = child_filename.data();
          info.size = file_size;
          info.folder_path = real_path / map_filename / volume_filename;
          info.compression_type = shared::archive::compression_type::none;

          results.emplace_back(info);
        }

        break;
      }
      else
      {
        raw_data.seekg(file_count * sizeof(rmf_file_header), std::ios::cur);
      }
    }

    return results;
  }

  void get_dyn_data(std::basic_istream<std::byte>& raw_data)
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

  void get_vol_data(std::basic_istream<std::byte>& raw_data)
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

    std::array<char, 14> filename{ '\0' };
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

  struct rmf_file_archive : shared::archive::file_archive
  {
    bool stream_is_supported(std::basic_istream<std::byte>& stream) override
    {
      std::array<std::byte, 4> tag{};
      stream.read(tag.data(), sizeof(tag));

      stream.seekg(-int(sizeof(tag)), std::ios::cur);

      auto result = false;

      for (auto& rmf_tag : rmf_tags)
      {
        result = tag == rmf_tag;

        if (result)
        {
          break;
        }
      }

      return result;
    }

    std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> get_content_info(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) override
    {
      std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> results;

      if (std::filesystem::exists(archive_or_folder_path))
      {
        auto raw_results = get_rmf_sub_archives(stream);

        results.reserve(raw_results.size());

        std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
          value.full_path = archive_or_folder_path / value.name;

          return value;
        });
      }
      else
      {
        auto raw_results = get_rmf_data(stream, archive_or_folder_path);

        results.reserve(raw_results.size());

        std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
          return value;
        });
      }

      return results;
    }

    void set_stream_position(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info) override
    {
      // TODO this actually needs to skip passed the header before the file contents
      if (int(stream.tellg()) != info.offset)
      {
        stream.seekg(info.offset, std::ios::beg);
      }
    }

    void extract_file_contents(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info, std::basic_ostream<std::byte>& output) override
    {
      set_stream_position(stream, info);

      std::copy_n(std::istreambuf_iterator<std::byte>(stream),
        info.size,
        std::ostreambuf_iterator<std::byte>(output));
    }
  };

}// namespace three_space::vol

#endif//DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
