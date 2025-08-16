#include <array>
#include <vector>
#include <sstream>
#include <spanstream>
#include <map>
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

  prj_resource_reader::prj_resource_reader() : resource_reader{ stream_is_supported, get_content_listing, set_stream_position, extract_file_contents }
  {
  }

  bool prj_resource_reader::stream_is_supported(std::istream& stream)
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

  struct prj_cache
  {
    std::map<std::filesystem::path, prj_resource_reader::file_info> files;
  };

  std::vector<prj_resource_reader::content_info> prj_resource_reader::get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query)
  {
    prj_cache storage;
    prj_cache& file_cache = storage;

    if (cache.has_value() && cache.type() == typeid(prj_cache))
    {
      file_cache = *std::any_cast<prj_cache>(&cache);
    }

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
          file_info.metadata = (std::uint16_t)name_entry.entry_index;

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


          if (file_info.filename.extension() == ".BWD" || file_info.filename.extension() == ".bwd")
          {
            file_info.compression_type = siege::platform::compression_type::custom;
            file_info.compressed_size = file_info.size;

            file_info.size = file_info.size * 64;
          }

          auto iter = file_cache.files.find(file_info.folder_path / file_info.filename);

          if (iter == file_cache.files.end())
          {
            file_cache.files.emplace(file_info.folder_path / file_info.filename, file_info);
          }
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

    if (&file_cache == &storage)
    {
      cache = std::move(file_cache);
    }

    return results;
  }

  void prj_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info)
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  struct iff_tag
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t size;
  };

  struct iff_data
  {
    iff_tag tag;
    std::vector<char> data;
  };

  void prj_resource_reader::extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output)
  {
    set_stream_position(stream, info);

    if (info.filename.extension() == ".BWD" || info.filename.extension() == ".bwd" && cache.has_value() && cache.type() == typeid(prj_cache))
    {
      constexpr auto bwd_entry_tag = platform::to_tag<4>({ 'B', 'W', 'D', '\0' });
      constexpr auto obj_entry_tag = platform::to_tag<4>({ 'O', 'B', 'J', '\0' });
      constexpr auto anim_entry_tag = platform::to_tag<4>({ 'A', 'N', 'I', 'M' });
      constexpr auto vptf_entry_tag = platform::to_tag<4>({ 'V', 'P', 'T', 'F' });
      constexpr auto pitf_entry_tag = platform::to_tag<4>({ 'P', 'I', 'T', 'F' });
      constexpr auto cptf_entry_tag = platform::to_tag<4>({ 'C', 'P', 'T', 'F' });
      constexpr auto mgdf_entry_tag = platform::to_tag<4>({ 'M', 'G', 'D', 'F' });
      constexpr auto asnd_entry_tag = platform::to_tag<4>({ 'A', 'S', 'N', 'D' });

      std::map<std::uint16_t, prj_resource_reader::file_info*> wtb_files_by_index;// obj
      std::map<std::uint16_t, prj_resource_reader::file_info*> tdi_files_by_index;// anim
      std::map<std::uint16_t, prj_resource_reader::file_info*> cpi_files_by_index;// cptf
      std::map<std::uint16_t, prj_resource_reader::file_info*> hdi_files_by_index;// hudf
      std::map<std::uint16_t, prj_resource_reader::file_info*> bwd_files_by_index;// pitf
      std::map<std::uint16_t, prj_resource_reader::file_info*> sfl_files_by_index;// asnd

      auto& file_cache = *std::any_cast<prj_cache>(&cache);

      for (auto& file : file_cache.files)
      {
        if (file.second.metadata.has_value() && file.second.metadata.type() == typeid(std::uint16_t))
        {
          if (file.second.filename.extension() == ".WTB" || file.second.filename.extension() == ".wtb")
          {
            auto index = std::any_cast<std::uint16_t>(file.second.metadata);
            wtb_files_by_index[index] = &file.second;
          }
          else if (file.second.filename.extension() == ".3DI" || file.second.filename.extension() == ".3di")
          {
            auto index = std::any_cast<std::uint16_t>(file.second.metadata);
            tdi_files_by_index[index] = &file.second;
          }
          else if (file.second.filename.extension() == ".CPI" || file.second.filename.extension() == ".cpi")
          {
            auto index = std::any_cast<std::uint16_t>(file.second.metadata);
            cpi_files_by_index[index] = &file.second;
          }
          else if (file.second.filename.extension() == ".HDI" || file.second.filename.extension() == ".hdi")
          {
            auto index = std::any_cast<std::uint16_t>(file.second.metadata);
            hdi_files_by_index[index] = &file.second;
          }
          else if (file.second.filename.extension() == ".BWD" || file.second.filename.extension() == ".bwd")
          {
            auto index = std::any_cast<std::uint16_t>(file.second.metadata);
            bwd_files_by_index[index] = &file.second;
          }
          else if (file.second.filename.extension() == ".SFL" || file.second.filename.extension() == ".sfl")
          {
            auto index = std::any_cast<std::uint16_t>(file.second.metadata);
            sfl_files_by_index[index] = &file.second;
          }
        }
      }

      iff_data bwd_root;

      stream.read((char*)&bwd_root.tag, sizeof(bwd_root.tag));

      if (bwd_root.tag.tag != bwd_entry_tag)
      {
        stream.seekg(-sizeof(bwd_root.tag), std::ios::cur);
        std::copy_n(std::istreambuf_iterator(stream),
          info.size,
          std::ostreambuf_iterator(output));
        return;
      }

      auto start_pos = output.tellp();
      auto bwd_size = bwd_root.tag.size;
      output.write((char*)&bwd_root.tag, sizeof(bwd_root.tag));
      std::array<char, 4> temp;

      stream.read(temp.data(), temp.size());
      output.write(temp.data(), temp.size());

      bwd_root.data.resize(bwd_root.tag.size - sizeof(temp) - sizeof(bwd_root.tag));
      stream.read(bwd_root.data.data(), bwd_root.data.size());

      std::ispanstream bwd_stream{ std::span<char>(bwd_root.data) };

      std::vector<iff_data> results;
      results.reserve(16);

      while (!(bwd_stream.eof() || bwd_stream.fail()))
      {
        iff_data& temp = results.emplace_back();

        bwd_stream.read((char*)&temp.tag, sizeof(temp.tag));

        if (bwd_stream.eof() || bwd_stream.fail())
        {
          results.pop_back();
          break;
        }

        temp.data.resize(temp.tag.size - sizeof(temp.tag));
        bwd_stream.read(temp.data.data(), temp.data.size());

        if (temp.tag.tag == obj_entry_tag)
        {
          endian::little_uint16_t file_index;
          auto real_size = temp.tag.size - sizeof(temp.tag);
          std::memcpy(&file_index, temp.data.data() + real_size - 4, sizeof(file_index));

          auto file_iter = wtb_files_by_index.find(file_index);

          if (file_iter != wtb_files_by_index.end())
          {
            set_stream_position(stream, *file_iter->second);
            temp.tag.size = temp.tag.size + file_iter->second->size;
            bwd_size = bwd_size + file_iter->second->size;
            output.write((char*)&temp.tag, sizeof(temp.tag));
            output.write(temp.data.data(), temp.data.size());
            std::copy_n(std::istreambuf_iterator(stream),
              file_iter->second->size,
              std::ostreambuf_iterator(output));
          }
        }
        else if (temp.tag.tag == anim_entry_tag)
        {
          endian::little_uint16_t file_index;
          std::memcpy(&file_index, temp.data.data(), sizeof(file_index));

          auto file_iter = tdi_files_by_index.find(file_index);

          if (file_iter != tdi_files_by_index.end())
          {
            set_stream_position(stream, *file_iter->second);
            temp.tag.size = temp.tag.size + file_iter->second->size;
            bwd_size = bwd_size + file_iter->second->size;
            output.write((char*)&temp.tag, sizeof(temp.tag));
            output.write(temp.data.data(), temp.data.size());
            std::copy_n(std::istreambuf_iterator(stream),
              file_iter->second->size,
              std::ostreambuf_iterator(output));
          }
          else
          {
            goto default_output;
          }
        }
        else if (temp.tag.tag == vptf_entry_tag)
        {
          endian::little_uint16_t file_index;
          std::memcpy(&file_index, temp.data.data(), sizeof(file_index));
        }
        else if (temp.tag.tag == cptf_entry_tag)
        {
          endian::little_uint16_t file_index;
          std::memcpy(&file_index, temp.data.data(), sizeof(file_index));

          auto file_iter = cpi_files_by_index.find(file_index);

          if (file_iter != cpi_files_by_index.end())
          {
            set_stream_position(stream, *file_iter->second);
            temp.tag.size = temp.tag.size + file_iter->second->size;
            bwd_size = bwd_size + file_iter->second->size;
            output.write((char*)&temp.tag, sizeof(temp.tag));
            output.write(temp.data.data(), temp.data.size());
            std::copy_n(std::istreambuf_iterator(stream),
              file_iter->second->size,
              std::ostreambuf_iterator(output));
          }
          else
          {
            goto default_output;
          }
        }
        else if (temp.tag.tag == pitf_entry_tag)
        {
          endian::little_uint16_t file_index;
          std::memcpy(&file_index, temp.data.data(), sizeof(file_index));

          auto file_iter = bwd_files_by_index.find(file_index);

          if (file_iter != bwd_files_by_index.end())
          {
            set_stream_position(stream, *file_iter->second);
            temp.tag.size = temp.tag.size + file_iter->second->size;
            bwd_size = bwd_size + file_iter->second->size;
            output.write((char*)&temp.tag, sizeof(temp.tag));
            output.write(temp.data.data(), temp.data.size());
            std::copy_n(std::istreambuf_iterator(stream),
              file_iter->second->size,
              std::ostreambuf_iterator(output));
          }
          else
          {
            goto default_output;
          }
        }
        else if (temp.tag.tag == mgdf_entry_tag)
        {
          endian::little_uint16_t file_index;
          std::memcpy(&file_index, temp.data.data(), sizeof(file_index));
        }
        else if (temp.tag.tag == asnd_entry_tag)
        {
          endian::little_uint16_t file_index;
          std::memcpy(&file_index, temp.data.data(), sizeof(file_index));

          auto file_iter = sfl_files_by_index.find(file_index);

          if (file_iter != sfl_files_by_index.end())
          {
            set_stream_position(stream, *file_iter->second);
            temp.tag.size = temp.tag.size + file_iter->second->size;
            bwd_size = bwd_size + file_iter->second->size;
            output.write((char*)&temp.tag, sizeof(temp.tag));
            output.write(temp.data.data(), temp.data.size());
            std::copy_n(std::istreambuf_iterator(stream),
              file_iter->second->size,
              std::ostreambuf_iterator(output));
          }
          else
          {
            goto default_output;
          }
        }
        else
        {
        default_output:
          output.write((char*)&temp.tag, sizeof(temp.tag));
          output.write(temp.data.data(), temp.data.size());
        }
      }

      output.seekp((std::size_t)start_pos + sizeof(bwd_root.tag.tag), std::ios::beg);
      output.write((char*)&bwd_size, sizeof(bwd_size));
    }
    else
    {
      std::copy_n(std::istreambuf_iterator(stream),
        info.size,
        std::ostreambuf_iterator(output));
    }
  }
}// namespace siege::resource::prj