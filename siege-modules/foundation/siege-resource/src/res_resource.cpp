#include <array>
#include <bitset>
#include <map>
#include <filesystem>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/platform/resource.hpp>
#include <siege/resource/res_resource.hpp>

namespace siege::resource::res
{
  namespace endian = siege::platform;
  namespace fs = std::filesystem;

  constexpr static auto riff_tag = platform::to_tag<4>("RIFF");
  constexpr static auto cdxa_tag = platform::to_tag<4>("CDXA");
  constexpr static auto fmt_tag = platform::to_tag<4>({ 'f', 'm', 't', 0x20 });
  constexpr static auto data_tag = platform::to_tag<4>("data");
  constexpr static auto total_sector_size = 2352u;
  constexpr static auto form_1_data_size = 2048u;
  constexpr static auto form_2_data_size = 2324u;

  struct res_header
  {
    std::array<std::byte, 4> riff_header;
    endian::little_uint32_t riff_size;
    std::array<std::byte, 4> type;
    std::array<std::byte, 4> format_header;
    endian::little_uint32_t format_size;
    std::array<std::byte, 16> format_data;
    std::array<std::byte, 4> data_header;
    endian::little_uint32_t data_size;
  };

  struct xa_sector_header
  {
    std::array<std::byte, 12> sync_pattern;
    std::array<std::uint8_t, 3> address;
    std::uint8_t mode;

    struct sub_header
    {
      std::uint8_t file_number;
      std::uint8_t channel;
      std::uint8_t sub_mode;
      std::uint8_t audio_info;
    };

    std::array<sub_header, 2> sub_headers;
  };

  struct file_index
  {
    endian::little_uint32_t sector_number;
    endian::little_uint32_t size;
  };


  bool res_resource_reader::is_supported(std::istream& stream)
  {
    auto path = siege::platform::get_stream_path(stream);

    if (path)
    {
      platform::istream_pos_resetter resetter(stream);

      if (path->extension() == ".res" || path->extension() == ".RES")
      {
        res_header header{};
        stream.read((char*)&header, sizeof(res_header));

        if (header.riff_header == riff_tag && header.type == cdxa_tag && header.format_header == fmt_tag && header.data_header == data_tag)
        {
          return true;
        }
      }
    }

    return false;
  }

  bool res_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<res_resource_reader::content_info> res_resource_reader::get_content_listing(std::any&, std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<res_resource_reader::content_info> results;


    res_header header{};
    stream.read((char*)&header, sizeof(res_header));

    if (header.riff_header == riff_tag && header.type == cdxa_tag && header.format_header == fmt_tag && header.data_header == data_tag)
    {
      auto main_index = stream.tellg();

      std::vector<file_index> files;

      std::vector<std::byte> file_index_storage;
      file_index_storage.reserve(2324 * 2);

      for (auto i = 0; i < 2; ++i)
      {
        xa_sector_header sector;

        stream.read((char*)&sector, sizeof(sector));

        auto sector_size = std::bitset<8>(sector.sub_headers[0].sub_mode)[8 - 5] ? form_1_data_size : form_2_data_size;

        auto index = file_index_storage.size();
        file_index_storage.resize(file_index_storage.size() + sector_size);
        stream.read((char*)file_index_storage.data() + index, sector_size);
        stream.seekg(form_2_data_size - sector_size + 4, std::ios::cur);
      }

      files.resize(file_index_storage.size() / sizeof(file_index));

      std::memcpy(files.data(), file_index_storage.data(), file_index_storage.size());

      auto exe_path = query.archive_path.parent_path() / "SLUS_009.24";

      std::vector<std::string> file_names;
      file_names.reserve(files.size());

      std::error_code code;

      if (std::filesystem::exists(exe_path, code))
      {
        std::ifstream exe_data(exe_path, std::ios::binary);

        exe_data.seekg(25096, std::ios::beg);
        std::string temp_str;
        temp_str.reserve(32);

        for (auto i = 0; i < 10000; i++)
        {
          auto temp = exe_data.get();

          if (temp > 0 && temp <= 127)
          {
            temp_str.push_back((char)temp);
          }
          else if (!temp_str.empty())
          {
            file_names.emplace_back(std::move(temp_str));
            temp_str = std::string();
          }

          if (!file_names.empty() && file_names.back() == "TRK\\KJ_FA.TRK")
          {
            break;
          }
        }

        // Start 25096 FILMS\CREDITS.STR
        //  FILMS\OUTRO3.STR
        //  End 34292 TRK\KJ_FA.TRK
      }
      else
      {
        file_names.resize(files.size());
      }

      std::map<fs::path, std::vector<std::string>> folders;

      auto get_parent_path = [&](auto& entry) {
        auto parent_path = fs::path(entry.data()).make_preferred().parent_path();
        return parent_path == fs::path() ? query.archive_path : query.archive_path / parent_path;
      };

      for (auto& entry : file_names)
      {
        auto parent_path = get_parent_path(entry);

        auto iter = folders.find(parent_path);

        if (iter == folders.end())
        {
          iter = folders.emplace(parent_path, std::vector<std::string>{}).first;
        }

        iter->second.emplace_back(fs::path(entry).make_preferred().filename().string());
      }

      for (auto& folder : folders)
      {
        if (folder.first.parent_path() == query.folder_path)
        {
          results.emplace_back(res_resource_reader::folder_info{
            .name = folder.first.filename().string(),
            .file_count = folder.second.size(),
            .full_path = folder.first,
            .archive_path = query.archive_path });
        }
      }

      for (auto& file : files)
      {
        if (file.size == 0)
        {
          break;
        }

        if (file_names.empty())
        {
          break;
        }

        auto last_string = std::move(file_names.back());

        auto sector_size = (file.size / total_sector_size) * total_sector_size;

        if ((file.size % total_sector_size) != 0)
        {
          sector_size += total_sector_size;
        }

        file_names.pop_back();

        auto parent_path = get_parent_path(last_string);

        if (parent_path == query.folder_path)
        {
          results.emplace_back(res_resource_reader::file_info{
            .filename = fs::path(last_string).make_preferred().filename().string(),
            .offset = (std::size_t)main_index + (file.sector_number * total_sector_size),
            .size = file.size,
            .compressed_size = sector_size,
            .folder_path = query.folder_path,
            .archive_path = query.archive_path,
          });
        }
      }
    }

    return results;
  }

  void res_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void res_resource_reader::extract_file_contents(std::any&, std::istream& stream, const siege::platform::file_info& info, std::ostream& output) const
  {
    if (!info.compressed_size)
    {
      return;
    }

    set_stream_position(stream, info);

    std::vector<char> sector_data;
    sector_data.reserve(*info.compressed_size);

    for (auto i = 0u; i < *info.compressed_size; i += total_sector_size)
    {
      xa_sector_header sector;

      stream.read((char*)&sector, sizeof(sector));
      auto sector_size = std::bitset<8>(sector.sub_headers[0].sub_mode)[8 - 5] ? form_1_data_size : form_2_data_size;

      sector_data.clear();
      sector_data.resize(sector_size);
      stream.read(sector_data.data(), sector_size);
      output.write(sector_data.data(), sector_size);

      stream.seekg(form_2_data_size - sector_size + 4, std::ios::cur);
    }
  }
}// namespace siege::resource::res