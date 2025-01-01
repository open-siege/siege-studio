// constexpr auto header_tag = platform::to_tag<4>({ '#', 'V', 'B', 'D' });
// constexpr auto item_tag = platform::to_tag<4>({ 0x80, 0x10, 0x54, 0xc0 });

// uses lzw compression

#include <memory>
#include <siege/platform/resource.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/resource/mw4_resource.hpp>

namespace siege::resource::mw4
{
  namespace endian = siege::platform;

  constexpr auto file_tag = platform::to_tag<4>("#VBD");

  struct win32_file_time
  {
    endian::little_uint32_t low_part;
    endian::little_uint32_t high_part;
  };

  struct mw4_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t version;
    endian::little_uint32_t file_count;
    endian::little_uint32_t first_file_data_offset;
    endian::little_uint32_t checksum;
  };

  struct alignas(std::byte) mw4_file_entry
  {
    win32_file_time file_time;
    endian::little_uint32_t uncompressed_size;
    endian::little_uint32_t compressed_size;
    endian::little_uint32_t checksum;
    endian::little_uint16_t id;
    char string_size;
  };

  bool mw4_resource_reader::is_supported(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    mw4_header header{};
    stream.read((char*)&header, sizeof(header));

    return header.tag == file_tag && header.version == 4;
  }

  bool mw4_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<mw4_resource_reader::content_info> mw4_resource_reader::get_content_listing(std::any&, std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);

    auto current_pos = (std::size_t)stream.tellg();

    mw4_header header{};
    stream.read((char*)&header, sizeof(header));

    if (header.tag != file_tag)
    {
      return {};
    }
    std::vector<mw4_resource_reader::content_info> results;
    results.reserve(header.first_file_data_offset / (sizeof(mw4_file_entry) * 2));

    std::size_t offset = header.first_file_data_offset;

    do
    {
      mw4_file_entry entry{};

      std::string filename;
      stream.read((char*)&entry, sizeof(entry));
      filename.resize(entry.string_size);
      stream.read(filename.data(), filename.size());

      if (entry.id == 0)
      {
        continue;
      }

      results.emplace_back(mw4_resource_reader::file_info{
        .filename = std::move(filename),
        .offset = (std::size_t)offset,
        .size = entry.uncompressed_size,
        .compressed_size = entry.compressed_size,
        .compression_type = entry.compressed_size == entry.uncompressed_size ? siege::platform::compression_type::none : siege::platform::compression_type::lzw,
        .folder_path = query.folder_path,
        .archive_path = query.archive_path,
      });

      offset += entry.compressed_size;
    } while ((std::size_t)stream.tellg() < current_pos + header.first_file_data_offset);


    return results;
  }

  void mw4_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void mw4_resource_reader::extract_file_contents(std::any&, std::istream& stream, const siege::platform::file_info& info, std::ostream& output) const
  {
    if (!info.compressed_size)
    {
      return;
    }

    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      *info.compressed_size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::mw4