#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>
#include <string>

#include "three_space_volume.hpp"

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

  constexpr auto dyn_tag = shared::to_tag<20>("Dynamix Volume File");

  constexpr auto vol_tag = shared::to_tag<4>({ 'V', 'O', 'L', 'N' });

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

  std::vector<shared::archive::file_info> get_dyn_data(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 20> header{};
    raw_data.read(header.data(), sizeof(header));

    if (header != dyn_tag)
    {
      throw std::invalid_argument("File was is not a DYN volume, as expected.");
    }

    raw_data.seekg(12, std::ios::cur);

    endian::little_uint32_t file_count{};

    raw_data.read(reinterpret_cast<std::byte*>(&file_count), sizeof(file_count));

    // The section being skipped appears to be an array of checksums.
    // TODO verify if the checksums are crc32.
    raw_data.seekg(file_count * sizeof(std::array<std::byte, 4>), std::ios::cur);

    std::array<char, 14> child_filename{ '\0' };

    std::vector<shared::archive::file_info> results;
    results.reserve(file_count);

    for (auto x = 0u; x < file_count; ++x)
    {
      shared::archive::file_info info{};

      info.compression_type = shared::archive::compression_type::none;
      info.offset = std::size_t(raw_data.tellg());

      raw_data.read(reinterpret_cast<std::byte*>(child_filename.data()), child_filename.size() - 1);

      endian::little_uint32_t file_size{};

      raw_data.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));

      info.filename = child_filename.data();
      info.size = file_size;

      results.emplace_back(info);
      raw_data.seekg(file_size, std::ios::cur);

      auto current_position = static_cast<int>(raw_data.tellg());

      while ((current_position % 4) != 0)
      {
        current_position++;
        raw_data.seekg(1, std::ios::cur);
      }
    }

    return results;
  }

  std::vector<std::string> get_vol_folders(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};
    raw_data.read(reinterpret_cast<std::byte*>(header.data()), sizeof(header));

    if (header != vol_tag)
    {
      throw std::invalid_argument("File was is not a VOL file, as expected.");
    }

    raw_data.seekg(6, std::ios::cur);

    endian::little_uint16_t num_chars{};

    raw_data.read(reinterpret_cast<std::byte*>(&num_chars), sizeof(num_chars));

    std::vector<char> raw_folders(num_chars, '\0');
    raw_data.read(reinterpret_cast<std::byte*>(raw_folders.data()), raw_folders.size());

    std::vector<std::string> folders;
    folders.reserve(raw_folders.size() / 5);

    size_t i = 0u;
    while (i < raw_folders.size())
    {
      folders.emplace_back(raw_folders.data() + i);

      if (folders.back().back() == '\\')
      {
        folders.back().pop_back();
      }

      i += strlen(raw_folders.data() + i) + 1;
    }

    return folders;
  }

  std::vector<shared::archive::file_info> get_vol_data(std::basic_istream<std::byte>& raw_data, const std::filesystem::path& archive_path)
  {
    const auto folders = get_vol_folders(raw_data);

    auto real_path = archive_path.parent_path().parent_path();
    auto volume_filename = archive_path.parent_path().filename().string();
    auto folder_name = archive_path.filename().string();

    endian::little_uint16_t num_files;
    raw_data.read(reinterpret_cast<std::byte*>(&num_files), sizeof(num_files));

    endian::little_uint32_t header_size;
    raw_data.read(reinterpret_cast<std::byte*>(&header_size), sizeof(header_size));

    std::vector<shared::archive::file_info> files;
    files.reserve(num_files);

    std::array<char, 14> filename{ '\0' };

    for (auto i = 0u; i < num_files; ++i)
    {
      shared::archive::file_info info{};

      raw_data.read(reinterpret_cast<std::byte*>(filename.data()), filename.size() - 1);
      std::uint8_t folder_index;
      raw_data.read(reinterpret_cast<std::byte*>(&folder_index), sizeof(folder_index));

      endian::little_uint32_t offset;
      raw_data.read(reinterpret_cast<std::byte*>(&offset), sizeof(offset));

      if (folder_index > folders.size())
      {
        continue;
      }

      info.folder_path = folders[folder_index];

      if (folder_name == info.folder_path.string())
      {
        info.compression_type = shared::archive::compression_type::none;
        info.offset = offset;
        info.filename = filename.data();

        files.emplace_back(info);
      }
    }

    for (auto& file : files)
    {
      raw_data.seekg(file.offset, std::ios::beg);

      std::byte entry{};
      raw_data.read(&entry, sizeof(entry));

      if (entry != std::byte{ 0x02 })
      {
        throw std::invalid_argument("VOL file has corrupted data.");
      }

      std::array<endian::little_uint32_t, 2> file_info{};
      raw_data.read(reinterpret_cast<std::byte*>(&file_info), sizeof(file_info));

      file.size = file_info[0];
    }

    return files;
  }

  bool rmf_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream)
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

  std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> rmf_file_archive::get_content_info(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path)
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

  void rmf_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info)
  {
    if (int(stream.tellg()) == info.offset)
    {
      stream.seekg(sizeof(std::array<std::byte, 13>) + sizeof(endian::little_int32_t), std::ios::cur);
    }
    else if (int(stream.tellg()) != info.offset + sizeof(std::array<std::byte, 13>) + sizeof(endian::little_int32_t))
    {
      stream.seekg(info.offset + sizeof(endian::little_int32_t), std::ios::beg);
    }
  }

  void rmf_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info, std::basic_ostream<std::byte>& output)
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator<std::byte>(stream),
      info.size,
      std::ostreambuf_iterator<std::byte>(output));
  }

  bool dyn_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream)
  {
    std::array<std::byte, 20> tag{};
    stream.read(tag.data(), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == dyn_tag;
  }

  std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> dyn_file_archive::get_content_info(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path)
  {
    std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> results;

    auto raw_results = get_dyn_data(stream);

    results.reserve(raw_results.size());

    std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
      value.folder_path = archive_or_folder_path;

      return value;
    });

    return results;
  }

  void dyn_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info)
  {
    if (int(stream.tellg()) == info.offset)
    {
      stream.seekg(sizeof(std::array<std::byte, 13>) + sizeof(endian::little_int32_t), std::ios::cur);
    }
    else if (int(stream.tellg()) != info.offset + sizeof(std::array<std::byte, 13>) + sizeof(endian::little_int32_t))
    {
      stream.seekg(info.offset + sizeof(endian::little_int32_t), std::ios::beg);
    }
  }

  void dyn_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info, std::basic_ostream<std::byte>& output)
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator<std::byte>(stream),
      info.size,
      std::ostreambuf_iterator<std::byte>(output));
  }

  bool vol_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(tag.data(), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == vol_tag;
  }

  std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> vol_file_archive::get_content_info(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path)
  {
    std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> results;

    if (std::filesystem::exists(archive_or_folder_path))
    {
      auto raw_results = get_vol_folders(stream);

      results.reserve(raw_results.size());

      std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
        shared::archive::folder_info info{};
        info.full_path = archive_or_folder_path / value;
        info.name = value;

        return info;
      });
    }
    else
    {
      auto raw_results = get_vol_data(stream, archive_or_folder_path);

      results.reserve(raw_results.size());

      std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
        value.folder_path = archive_or_folder_path;
        return value;
      });
    }

    return results;
  }

  void vol_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info)
  {
    // TODO this actually needs to skip passed the header before the file contents
    if (int(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void vol_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info, std::basic_ostream<std::byte>& output)
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator<std::byte>(stream),
      info.size,
      std::ostreambuf_iterator<std::byte>(output));
  }
}// namespace three_space::vol