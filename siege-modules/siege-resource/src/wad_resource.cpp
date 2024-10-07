#include <fstream>
#include <filesystem>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstdlib>
#include <optional>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <map>

#include <siege/resource/wad_resource.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/wave.hpp>
#include <siege/platform/palette.hpp>
#include <siege/platform/bitmap.hpp>

namespace fs = std::filesystem;

namespace siege::resource::wad
{
  namespace endian = siege::platform;
  using folder_info = siege::platform::folder_info;

  constexpr auto pod_tag = platform::to_tag<8>("PODFILE");

  struct pod_file_entry
  {
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
    endian::little_uint32_t string_offset;
    endian::little_uint32_t type;
    endian::little_uint16_t id1;
    endian::little_uint16_t id2;
    std::uint32_t padding;
    std::uint32_t string_start;
    std::uint32_t string_end;
  };

  struct legacy_wad_header
  {
    endian::little_uint16_t file_count;
    endian::little_uint32_t offset;
    endian::little_uint16_t file_buffer_size;
  };

  struct legacy_file_entry
  {
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
    endian::little_uint32_t string_offset;
  };

  bool wad_resource_reader::is_supported(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 8> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    if (tag != pod_tag)
    {
      legacy_wad_header header;
      std::memcpy(&header, tag.data(), sizeof(header));

      if (header.file_count <= 3000 && header.file_buffer_size <= (3000 * 32))
      {
        auto path = siege::platform::get_stream_path(stream);

        if (!path)
        {
          return false;
        }

        return path->extension() == ".dat" || path->extension() == ".DAT" || path->extension() == ".cd" || path->extension() == ".CD" || path->extension() == ".hd" || path->extension() == ".HD" || path->extension() == ".blo" || path->extension() == ".BLO";
      }
    }

    return tag == pod_tag;
  }

  bool wad_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }


  void generate_bitmap_metadata(std::istream& stream, std::int32_t width, std::int32_t height, std::size_t offset, const std::vector<siege::platform::palette::colour_rgb>& current_palette, std::string& extension, std::any& metadata)
  {
    extension = "bmp";
    std::vector<platform::palette::colour> colours;
    colours.reserve(current_palette.size());

    std::transform(current_palette.begin(), current_palette.end(), std::back_inserter(colours), [](auto value) {
      return platform::palette::colour{ value.red, value.green, value.blue, std::byte(0xff) };
    });

    siege::platform::bitmap::bitmap_offset_settings temp{};
    temp.colours = std::move(colours);
    temp.width = width;
    temp.height = height;
    temp.bit_depth = 8;
    temp.auto_flip = false;
    temp.offset = offset;

    metadata = std::move(temp);
  }

  void generate_bitmap_metadata(std::istream& stream, std::size_t offset, std::size_t size, const std::vector<siege::platform::palette::colour_rgb>& current_palette, std::string& extension, std::any& metadata)
  {
    if (!current_palette.empty())
    {
      endian::little_uint16_t width;
      endian::little_uint16_t height;
      stream.seekg(offset, std::ios::beg);
      stream.read(reinterpret_cast<char*>(&width), sizeof(width));
      stream.read(reinterpret_cast<char*>(&height), sizeof(height));
      std::uint32_t pixel_size = (std::uint32_t)width * height;

      if (size == (pixel_size + 12))
      {
        generate_bitmap_metadata(stream, width, height, 12, current_palette, extension, metadata);
      }

      if (size == (pixel_size + 10))
      {
        generate_bitmap_metadata(stream, width, height, 10, current_palette, extension, metadata);
      }

      if (size == (pixel_size + 8))
      {
        generate_bitmap_metadata(stream, width, height, 8, current_palette, extension, metadata);
      }
    }
  }

  std::vector<wad_resource_reader::content_info> wad_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 8> tag{};

    auto current_offset = stream.tellg();

    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    if (tag != pod_tag)
    {
      legacy_wad_header header;
      std::memcpy(&header, tag.data(), sizeof(header));

      if (header.file_count == 0)
      {
        return std::vector<wad_resource_reader::content_info>{};
      }
      std::vector<legacy_file_entry> legacy_entries;
      legacy_entries.assign(header.file_count, {});

      auto entries_size = header.file_count * sizeof(legacy_file_entry);
      stream.seekg(current_offset + header.offset, std::ios::beg);
      stream.read(reinterpret_cast<char*>(legacy_entries.data()), entries_size);

      auto left_over_space = (std::uint32_t)header.file_buffer_size - entries_size;
      std::string string_table(left_over_space, '\0');
      stream.read(string_table.data(), left_over_space);

      std::vector<wad_resource_reader::content_info> results{};
      results.reserve(header.file_count);

      std::optional<std::string_view> current_group = std::nullopt;
      std::vector<siege::platform::palette::colour_rgb> default_palette;

      for (auto& entry : legacy_entries)
      {
        if (entry.string_offset == 0)
        {
          continue;
        }

        auto filename = std::string_view(string_table.data() + entry.string_offset - entries_size);

        if (filename == "soundeffects" && entry.size == 0 && entry.offset == 0)
        {
          constexpr static auto riff_header = std::array<char, 4>{ { 'R', 'I', 'F', 'F' } };
          auto wav_offset = (std::size_t)current_offset + sizeof(legacy_wad_header) + 2u;
          auto index = 0u;
          std::array<char, 4> header;

          do
          {
            endian::little_uint32_t wav_size;

            stream.seekg(wav_offset, std::ios::beg);
            stream.read(header.data(), header.size());
            stream.read((char*)&wav_size, sizeof(wav_size));

            
            if (header != riff_header)
            {
              break;
            }

            results.emplace_back(wad_resource_reader::file_info{
              .filename = std::string(filename) + std::to_string(index) + "." + "wav",
              .offset = wav_offset,
              .size = wav_size + 8u,
              .compression_type = siege::platform::compression_type::none,
              .folder_path = query.folder_path,
              .archive_path = query.archive_path });
            wav_offset += wav_size + 8u;
            index++;
          } while (header == riff_header);
        }

        if (entry.size == 0 && (filename.starts_with("start") || filename.starts_with("START")))
        {
          current_group = filename.substr(5);
          continue;
        }

        if (entry.size == 0 && (filename.ends_with("start") || filename.ends_with("START")))
        {
          current_group = filename.substr(0, filename.size() - 5);
          continue;
        }

        if (entry.size == 0 && (filename.starts_with("end") || filename.starts_with("END") || filename.ends_with("end") || filename.ends_with("END")))
        {
          current_group = std::nullopt;
          continue;
        }

        std::string extension = current_group ? std::string(*current_group) : std::string();
        std::any metadata{};

        if (default_palette.empty() && (filename == "palette" || current_group == "pal"))
        {
          stream.seekg(current_offset + entry.offset, std::ios::beg);
          default_palette.assign(entry.size / sizeof(siege::platform::palette::colour_rgb), {});
          stream.read(reinterpret_cast<char*>(default_palette.data()), entry.size);
        }

        if (current_group == "flats" && entry.size == (64 * 64) && !default_palette.empty())
        {
          generate_bitmap_metadata(stream, 64, 64, 0, default_palette, extension, metadata);
        }
        else if (current_group == std::nullopt && !default_palette.empty())
        {
          generate_bitmap_metadata(stream, current_offset + entry.offset, entry.size, default_palette, extension, metadata);
        }

        results.emplace_back(wad_resource_reader::file_info{
          .filename = !extension.empty() ? std::string(filename) + "." + extension : std::string(filename),
          .offset = entry.offset,
          .size = entry.size,
          .compression_type = siege::platform::compression_type::none,
          .folder_path = query.folder_path,
          .archive_path = query.archive_path,
          .metadata = std::move(metadata) });
      }

      return results;
    }

    endian::little_uint32_t file_count;
    endian::little_uint32_t offset;
    endian::little_uint32_t file_buffer_size;


    stream.seekg(4, std::ios::cur);
    stream.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));
    stream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    stream.read(reinterpret_cast<char*>(&file_buffer_size), sizeof(file_buffer_size));

    if (file_count == 0)
    {
      return std::vector<wad_resource_reader::content_info>{};
    }

    std::vector<pod_file_entry> entries;
    entries.assign(file_count, {});


    auto entries_size = file_count * sizeof(pod_file_entry);
    stream.seekg(current_offset + offset, std::ios::beg);
    stream.read(reinterpret_cast<char*>(entries.data()), entries_size);

    auto left_over_space = (std::uint32_t)file_buffer_size - entries_size;
    std::string string_table(left_over_space, '\0');
    stream.read(string_table.data(), left_over_space);

    auto index = 0;

    std::vector<wad_resource_reader::content_info> results{};
    results.reserve(file_count);

    std::optional<std::string_view> current_group = std::nullopt;

    std::vector<siege::platform::palette::colour_rgb> default_palette;
    std::vector<siege::platform::palette::colour_rgb> current_palette;

    for (auto& entry : entries)
    {
      auto filename = std::string_view(string_table.data() + entry.string_offset - entries_size);

      if (entry.type == 0x32 && entry.size == 0 && filename.starts_with("start"))
      {
        current_group = filename.substr(5);
        continue;
      }

      if (entry.type == 0x32 && entry.size == 0 && filename.starts_with("end"))
      {
        current_group = std::nullopt;
        current_palette = default_palette;
        continue;
      }

      std::string extension = current_group ? std::string(*current_group) : std::string();

      std::any metadata{};

      if (entry.type == 0x2)
      {
        extension = "pal";
        if (default_palette.empty())
        {
          stream.seekg(entry.offset, std::ios::beg);
          default_palette.assign(entry.size / sizeof(siege::platform::palette::colour_rgb), {});
          stream.read(reinterpret_cast<char*>(default_palette.data()), entry.size);
          current_palette = default_palette;
        }

        if (current_group)
        {
          stream.seekg(entry.offset, std::ios::beg);
          current_palette.assign(entry.size / sizeof(siege::platform::palette::colour_rgb), {});
          stream.read(reinterpret_cast<char*>(current_palette.data()), entry.size);
        }
      }
      else if (entry.type == 0x1 && current_group == "music")
      {
        extension = "mds";
      }
      else if (entry.type == 0x1 && current_group == "sound")
      {
        extension = "wav";
        metadata = siege::platform::wave::header_settings{
          .num_channels = 1,
          .sample_rate = 11025,
          .bits_per_sample = 8
        };
      }
      else if (entry.type == 0x1 && (current_group == "leveltext" || current_group == "othertext"))
      {
        extension = "txt";
      }
      else if (entry.type == 0x1 && entry.size == 640 * 480)
      {
        if (!current_palette.empty())
        {
          generate_bitmap_metadata(stream, 640, 480, 0, default_palette, extension, metadata);
        }
      }
      else if (entry.type == 0x1 && entry.size == 320 * 200)
      {
        if (!current_palette.empty())
        {
          generate_bitmap_metadata(stream, 320, 200, 0, default_palette, extension, metadata);
        }
      }
      else if (entry.type != 0x1)
      {
        generate_bitmap_metadata(stream, current_offset + entry.offset, entry.size, current_palette, extension, metadata);
      }

      results.emplace_back(wad_resource_reader::file_info{
        .filename = !extension.empty() ? std::string(filename) + "." + extension : std::string(filename),
        .offset = entry.offset,
        .size = entry.size,
        .compression_type = siege::platform::compression_type::none,
        .folder_path = query.folder_path,
        .archive_path = query.archive_path,
        .metadata = std::move(metadata) });
    }

    return results;
  }

  void wad_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void wad_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage) const
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::wad
