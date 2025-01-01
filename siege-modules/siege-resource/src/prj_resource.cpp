#include <array>
#include <vector>
#include <sstream>
#include <siege/platform/resource.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/resource/prj_resource.hpp>

namespace siege::resource::prj
{
  namespace endian = siege::platform;

  constexpr auto header_tag = platform::to_tag<4>({ 'P', 'R', 'O', 'J' });
  constexpr auto folder_index_tag = platform::to_tag<4>({ 'D', 'D', 'I', 'T' });
  constexpr auto folder_entry_tag = platform::to_tag<4>({ 'I', 'N', 'D', 'X' });
  constexpr auto file_name_data_tag = platform::to_tag<4>({ 'S', 'Y', 'M', 'B' });
  constexpr auto file_entry_data_tag = platform::to_tag<4>({ 'D', 'A', 'T', 'A' });

  struct dir_entry
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t index_offset;
    endian::little_uint32_t index_size;
    endian::little_uint32_t symbol_offset;
    endian::little_uint32_t symbol_size;
    endian::little_uint32_t unknown;
  };

  struct dir_index_entry
  {
    std::array<std::byte, 4> tag;
    endian::little_uint16_t total_file_count;
    endian::little_uint16_t real_file_count;
    endian::little_uint16_t real_file_count2;
  };

  struct prf_index
  {
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
  };

  struct file_symbol_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint16_t total_file_count;
    endian::little_uint16_t real_file_count;
    endian::little_uint16_t padding;
  };

  struct file_symbol_entry
  {
    std::array<char, 16> filename;
    endian::little_uint16_t entry_index;
  };

  struct alignas(std::byte) file_data_entry
  {
    std::array<std::byte, 4> folder_tag;
    struct date_time
    {
      std::byte hour;
      std::byte minute;
      std::byte day;
      std::byte month;
      endian::little_uint16_t year;
      endian::little_uint16_t padding;
    } time_stamp;
    endian::little_uint16_t entry_index;
    endian::little_uint16_t unknown1;
    endian::little_uint16_t unknown2;
    std::array<char, 16> symbol_name;
    std::array<char, 16> filename;
  };

  bool prj_resource_reader::is_supported(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);

    std::array<std::byte, 4> first_tag;
    stream.read((char*)&first_tag, sizeof(first_tag));
    stream.seekg(sizeof(first_tag), std::ios::cur);
    stream.seekg(sizeof(first_tag), std::ios::cur);

    std::array<std::byte, 4> second_tag;
    stream.read((char*)&second_tag, sizeof(second_tag));

    return first_tag == header_tag && second_tag == folder_index_tag;
  }

  bool prj_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<prj_resource_reader::content_info> prj_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);

    std::array<std::byte, 4> first_tag;
    stream.read((char*)&first_tag, sizeof(first_tag));
    stream.seekg(sizeof(first_tag), std::ios::cur);
    stream.seekg(sizeof(first_tag), std::ios::cur);

    std::array<std::byte, 4> second_tag;
    stream.read((char*)&second_tag, sizeof(second_tag));

    if (first_tag != header_tag || second_tag != folder_index_tag)
    {
      return {};
    }

    endian::little_uint32_t index_size{};

    stream.read((char*)&index_size, sizeof(index_size));

    std::stringstream temp;
    std::copy_n(std::istreambuf_iterator(stream),
      index_size,
      std::ostreambuf_iterator(temp));

    temp.seekg(sizeof(index_size), std::ios::cur);

    endian::little_uint16_t folder_count{};
    temp.read((char*)&folder_count, sizeof(folder_count));

    std::vector<dir_entry> dir_entries;
    dir_entries.resize(folder_count);
    temp.read((char*)dir_entries.data(), sizeof(dir_entry) * dir_entries.size());

    std::vector<prj_resource_reader::content_info> results;
    results.reserve(folder_count);

    if (query.folder_path != query.archive_path)
    {
      auto folder_name = query.folder_path.stem().string();

      for (auto& entry : dir_entries)
      {
        if (folder_name == std::string_view((char*)entry.tag.data(), entry.tag[3] == std::byte{} ? 3 : 4))
        {
          continue;
        }
        entry.symbol_offset = 0;
        entry.index_offset = 0;
      }
    }
    else
    {
      for (auto& entry : dir_entries)
      {
        if (entry.index_offset == 0 || entry.symbol_offset == 0)
        {
          continue;
        }

        auto name = std::string((char*)entry.tag.data(), entry.tag[3] == std::byte{} ? 3 : 4);

        results.emplace_back(prj_resource_reader::folder_info{
          .name = name,
          .full_path = query.archive_path / name,
          .archive_path = query.archive_path });
      }

      return results;
    }

    for (auto& entry : dir_entries)
    {
      if (entry.index_offset == 0 || entry.symbol_offset == 0)
      {
        continue;
      }

      stream.seekg(entry.index_offset + (std::uint32_t)resetter.position, std::ios::beg);

      std::array<std::byte, 4> second_tag;
      stream.read((char*)&second_tag, sizeof(second_tag));

      if (second_tag != folder_entry_tag)
      {
        return results;
      }

      stream.read((char*)&index_size, sizeof(index_size));
      temp.str("");
      temp.seekg(0, std::ios::beg);

      std::copy_n(std::istreambuf_iterator(stream),
        index_size,
        std::ostreambuf_iterator(temp));

      temp.seekg(sizeof(index_size), std::ios::cur);


      dir_index_entry index_entry;
      temp.read((char*)&index_entry, sizeof(index_entry));

      if (index_entry.tag != entry.tag)
      {
        break;
      }

      std::vector<prf_index> file_indices;
      file_indices.resize(index_entry.real_file_count);
      temp.read((char*)file_indices.data(), sizeof(prf_index) * file_indices.size());

      stream.seekg(entry.symbol_offset + (std::uint32_t)resetter.position, std::ios::beg);

      stream.read((char*)&second_tag, sizeof(second_tag));

      if (second_tag != file_name_data_tag)
      {
        return results;
      }

      stream.read((char*)&index_size, sizeof(index_size));
      temp.str("");
      temp.seekg(0, std::ios::beg);

      std::copy_n(std::istreambuf_iterator(stream),
        index_size,
        std::ostreambuf_iterator(temp));

      temp.seekg(sizeof(index_size), std::ios::cur);

      file_symbol_header symbol_header;
      temp.read((char*)&symbol_header, sizeof(symbol_header));

      if (symbol_header.tag != entry.tag)
      {
        return results;
      }

      std::vector<file_symbol_entry> name_entries;
      name_entries.resize(symbol_header.real_file_count);
      temp.read((char*)name_entries.data(), sizeof(file_symbol_entry) * name_entries.size());

      std::vector<prj_resource_reader::file_info> file_results;
      file_results.reserve(name_entries.size());

      for (auto& name_entry : name_entries)
      {
        try
        {
          auto& index_entry = file_indices.at(name_entry.entry_index);

          auto& file_info = file_results.emplace_back();
          file_info.offset = index_entry.offset + (std::uint32_t)resetter.position;
          file_info.size = index_entry.size;
          file_info.filename = name_entry.filename.data();
          file_info.archive_path = query.archive_path;
          file_info.folder_path = query.folder_path;
          file_info.metadata = name_entry.entry_index;

          stream.seekg(file_info.offset, std::ios::beg);
          std::array<std::byte, 4> file_tag_value;
          stream.read((char*)&file_tag_value, sizeof(file_tag_value));

          if (file_tag_value != file_entry_data_tag)
          {
            continue;
          }
          stream.read((char*)&index_size, sizeof(index_size));
          stream.seekg(sizeof(index_size), std::ios::cur);

          file_data_entry final_entry;

          stream.read((char*)&final_entry, sizeof(final_entry));

          if (std::string_view(final_entry.symbol_name.data()) != file_info.filename)
          {
            break;
          }

          file_info.offset = stream.tellg();
          file_info.size = index_size;
          file_info.size -= (sizeof(final_entry) + sizeof(index_size));
          file_info.filename = final_entry.filename.data();
        }
        catch (...)
        {
          break;
        }
      }

      results.reserve(results.capacity() + file_results.size());
      for (auto& file_result : file_results)
      {
        results.emplace_back(std::move(file_result));
      }
    }

    return results;
  }

  void prj_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void prj_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::prj