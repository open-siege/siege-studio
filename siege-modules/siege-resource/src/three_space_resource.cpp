#include <array>
#include <vector>
#include <fstream>
#include <utility>
#include <string>
#include <cstring>

#include <siege/resource/three_space_resource.hpp>
#include <siege/platform/stream.hpp>

namespace siege::resource::vol::three_space
{
  namespace endian = siege::platform;

  constexpr auto rmf_tags = std::array<std::array<std::byte, 4>, 12>{
    platform::to_tag<4>({ 0x00, 0x01, 0x05, 0x07 }),
    platform::to_tag<4>({ 0x00, 0x01, 0x06, 0x07 }),
    platform::to_tag<4>({ 0x00, 0x01, 0x03, 0x06 }),
    platform::to_tag<4>({ 0x00, 0x01, 0x02, 0x06 }),
    platform::to_tag<4>({ 0x00, 0x02, 0x05, 0x07 }),
    platform::to_tag<4>({ 0x00, 0x02, 0x04, 0x07 }),
    platform::to_tag<4>({ 0x01, 0x00, 0x00, 0x00 }),
    platform::to_tag<4>({ 0x01, 0x04, 0x06, 0x07 }),
    platform::to_tag<4>({ 0x01, 0x02, 0x04, 0x05 }),
    platform::to_tag<4>({ 0x00, 0x01, 0x05, 0x06 }),
    platform::to_tag<4>({ 0x00, 0x01, 0x04, 0x07 }),
    platform::to_tag<4>({ 0x03, 0x04, 0x05, 0x07 })
  };

  constexpr auto dyn_tag = platform::to_tag<20>("Dynamix Volume File");

  constexpr auto vol_tag = platform::to_tag<4>({ 'V', 'O', 'L', 'N' });

  struct rmf_file_header
  {
    endian::little_int32_t checksum;
    endian::little_int32_t offset;
  };

  std::vector<siege::platform::folder_info> get_rmf_sub_archives(std::istream& raw_data)
  {
    std::array<std::byte, 6> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    auto volume_count = static_cast<int>(header[header.size() - 2]);

    std::array<char, 14> filename{ '\0' };

    std::vector<siege::platform::folder_info> results;
    results.reserve(volume_count);

    for (auto i = 0; i < volume_count; ++i)
    {
      platform::read(raw_data, reinterpret_cast<char*>(filename.data()), sizeof(filename) - 1);

      endian::little_uint16_t file_count{};
      platform::read(raw_data, reinterpret_cast<char*>(&file_count), sizeof(file_count));

      siege::platform::folder_info info{};
      info.name = filename.data();
      info.file_count = file_count;

      results.emplace_back(info);

      raw_data.seekg(file_count * sizeof(rmf_file_header), std::ios::cur);
    }

    return results;
  }

  std::vector<siege::platform::file_info> get_rmf_data(std::istream& raw_data, const std::filesystem::path& folder_path)
  {
    std::vector<siege::platform::file_info> results;
    std::array<std::byte, 6> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    auto volume_count = static_cast<int>(header[header.size() - 2]);

    std::array<char, 14> filename{ '\0' };

    auto real_path = folder_path.parent_path().parent_path();
    auto map_filename = folder_path.parent_path().filename().string();
    auto volume_filename = folder_path.filename().string();

    for (auto i = 0; i < volume_count; ++i)
    {
      platform::read(raw_data, reinterpret_cast<char*>(filename.data()), sizeof(filename) - 1);

      endian::little_uint16_t file_count{};
      platform::read(raw_data, reinterpret_cast<char*>(&file_count), sizeof(file_count));

      if (volume_filename == std::string_view(filename.data()))
      {
        std::vector<rmf_file_header> headers(file_count, rmf_file_header{});

        platform::read(raw_data, reinterpret_cast<char*>(headers.data()), file_count * sizeof(rmf_file_header));

        std::sort(headers.begin(), headers.end(), [](const auto& a, const auto& b) {
          return a.offset < b.offset;
        });

        results.reserve(file_count);

        auto volume = std::ifstream{ real_path / volume_filename, std::ios::binary };

        std::array<char, 14> child_filename{ '\0' };

        for (auto x = 0; x < file_count; ++x)
        {
          auto& file_header = headers[x];

          volume.seekg(file_header.offset, std::ios::beg);

          volume.read(reinterpret_cast<char*>(child_filename.data()), child_filename.size() - 1);
          endian::little_uint32_t file_size{};
          volume.read(reinterpret_cast<char*>(&file_size), sizeof(file_size));

          siege::platform::file_info info{};
          info.offset = file_header.offset;
          info.filename = child_filename.data();
          info.size = file_size;
          info.folder_path = real_path / map_filename / volume_filename;
          info.compression_type = siege::platform::compression_type::none;

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

  std::vector<siege::platform::file_info> get_dyn_data(std::istream& raw_data)
  {
    std::array<std::byte, 20> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    if (header != dyn_tag)
    {
      throw std::invalid_argument("File was is not a DYN volume, as expected.");
    }

    raw_data.seekg(12, std::ios::cur);

    endian::little_uint32_t file_count{};

    platform::read(raw_data, reinterpret_cast<char*>(&file_count), sizeof(file_count));

    // The section being skipped appears to be an array of checksums.
    // TODO verify if the checksums are crc32.
    raw_data.seekg(file_count * sizeof(std::array<std::byte, 4>), std::ios::cur);

    std::array<char, 14> child_filename{ '\0' };

    std::vector<siege::platform::file_info> results;
    results.reserve(file_count);

    for (auto x = 0u; x < file_count; ++x)
    {
      siege::platform::file_info info{};

      info.compression_type = siege::platform::compression_type::none;
      info.offset = std::size_t(raw_data.tellg());

      platform::read(raw_data, reinterpret_cast<char*>(child_filename.data()), child_filename.size() - 1);

      endian::little_uint32_t file_size{};

      platform::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));

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

  std::vector<std::string> get_vol_folders(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    platform::read(raw_data, reinterpret_cast<char*>(header.data()), sizeof(header));

    if (header != vol_tag)
    {
      throw std::invalid_argument("File was is not a VOL file, as expected.");
    }

    raw_data.seekg(6, std::ios::cur);

    endian::little_uint16_t num_chars{};

    platform::read(raw_data, reinterpret_cast<char*>(&num_chars), sizeof(num_chars));

    std::vector<char> raw_folders(num_chars, '\0');
    platform::read(raw_data, reinterpret_cast<char*>(raw_folders.data()), raw_folders.size());

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

  std::vector<siege::platform::file_info> get_vol_data(std::istream& raw_data, const std::filesystem::path& folder_path)
  {
    const auto folders = get_vol_folders(raw_data);
    auto folder_name = folder_path.filename().string();

    endian::little_uint16_t num_files;
    platform::read(raw_data, reinterpret_cast<char*>(&num_files), sizeof(num_files));

    endian::little_uint32_t header_size;
    platform::read(raw_data, reinterpret_cast<char*>(&header_size), sizeof(header_size));

    std::vector<siege::platform::file_info> files;
    files.reserve(num_files);

    std::array<char, 14> filename{ '\0' };

    for (auto i = 0u; i < num_files; ++i)
    {
      siege::platform::file_info info{};

      platform::read(raw_data, reinterpret_cast<char*>(filename.data()), filename.size() - 1);
      std::uint8_t folder_index;
      platform::read(raw_data, reinterpret_cast<char*>(&folder_index), sizeof(folder_index));

      endian::little_uint32_t offset;
      platform::read(raw_data, reinterpret_cast<char*>(&offset), sizeof(offset));

      if (folder_index > folders.size() || folders.empty())
      {
        info.folder_path = folder_name;
      }
      else
      {
        info.folder_path = folders[folder_index];
      }

      if (folder_name == info.folder_path.string())
      {
        info.compression_type = siege::platform::compression_type::none;
        info.offset = offset;
        info.filename = filename.data();

        files.emplace_back(info);
      }
    }

    for (auto& file : files)
    {
      raw_data.seekg(file.offset, std::ios::beg);

      std::byte entry{};
      platform::read(raw_data, &entry, sizeof(entry));

      if (!(entry == std::byte{ 0x02 } || entry == std::byte{ 0x09 }))
      {
        throw std::invalid_argument("VOL file has corrupted data.");
      }

      file.compression_type = entry == std::byte{ 0x02 } ? platform::compression_type::none : platform::compression_type::lz;

      std::array<endian::little_uint32_t, 2> file_info{};
      platform::read(raw_data, reinterpret_cast<char*>(&file_info), sizeof(file_info));

      file.size = file_info[0];
    }

    return files;
  }

  bool rmf_resource_reader::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

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

  bool rmf_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<rmf_resource_reader::content_info> rmf_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> results;

    if (query.archive_path == query.folder_path)
    {
      auto raw_results = get_rmf_sub_archives(stream);

      results.reserve(raw_results.size());

      std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
        value.full_path = query.archive_path / value.name;

        return value;
      });
    }
    else
    {
      auto raw_results = get_rmf_data(stream, query.folder_path);

      results.reserve(raw_results.size());

      std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
        return value;
      });
    }

    return results;
  }

  void rmf_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    constexpr auto header_size = sizeof(std::array<std::byte, 13>) + sizeof(endian::little_int32_t);

    if (std::size_t(stream.tellg()) == info.offset)
    {
      stream.seekg(header_size, std::ios::cur);
    }
    else if (std::size_t(stream.tellg()) != info.offset + header_size)
    {
      stream.seekg(info.offset + header_size, std::ios::beg);
    }
  }

  void rmf_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
    auto main_path = info.folder_path.parent_path().parent_path();
    auto resource_file = info.folder_path.filename();

    auto real_stream = std::ifstream(main_path / resource_file, std::ios::binary);

    set_stream_position(real_stream, info);

    std::copy_n(std::istreambuf_iterator(real_stream),
                info.size,
                std::ostreambuf_iterator(output));
  }

  bool dyn_resource_reader::is_supported(std::istream& stream)
  {
    std::array<std::byte, 20> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == dyn_tag;
  }


  bool dyn_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<dyn_resource_reader::content_info> dyn_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<dyn_resource_reader::content_info> results;

    auto raw_results = get_dyn_data(stream);

    results.reserve(raw_results.size());

    std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
      value.folder_path = query.folder_path;

      return value;
    });

    return results;
  }

  void dyn_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    constexpr auto header_size = sizeof(std::array<std::byte, 13>) + sizeof(endian::little_int32_t);

    if (std::size_t(stream.tellg()) == info.offset)
    {
      stream.seekg(header_size, std::ios::cur);
    }
    else if (std::size_t(stream.tellg()) != info.offset + header_size)
    {
      stream.seekg(info.offset + header_size, std::ios::beg);
    }
  }

  void dyn_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));
  }

  bool vol_resource_reader::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == vol_tag;
  }

  bool vol_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<dyn_resource_reader::content_info> vol_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<dyn_resource_reader::content_info> results;

    if (query.archive_path == query.folder_path)
    {
      auto position = stream.tellg();
      auto raw_results = get_vol_folders(stream);

      results.reserve(raw_results.size());

      std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
        siege::platform::folder_info info{};
        info.full_path = query.archive_path / value;
        info.name = value;

        return info;
      });

      if (!raw_results.empty())
      {
        return results;
      }
      stream.seekg(position, std::ios::beg);
    }

    auto raw_results = get_vol_data(stream, query.folder_path);

    results.reserve(raw_results.size());

    std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](auto& value) {
           value.folder_path = query.folder_path;
           return value;
    });

    return results;
  }

  constexpr auto header_size = sizeof(std::byte) + sizeof(std::array<endian::little_uint32_t, 2>);

  void vol_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) == info.offset)
    {
      stream.seekg(header_size, std::ios::cur);
    }
    else if (std::size_t(stream.tellg()) != info.offset + header_size)
    {
      stream.seekg(info.offset + header_size, std::ios::beg);
    }
  }

  void vol_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
    auto current_position = stream.tellg();
    stream.seekg(0, std::ios::end);
    auto last_byte = static_cast<std::size_t>(stream.tellg());

    stream.seekg(current_position, std::ios::beg);

    set_stream_position(stream, info);

    const auto remaining_bytes = last_byte - info.offset - header_size;

    std::copy_n(std::istreambuf_iterator(stream),
      info.size > remaining_bytes ? remaining_bytes : info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::vol::three_space
