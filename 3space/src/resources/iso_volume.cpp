#include <fstream>
#include <filesystem>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <deque>
#include <cstring>

#include "resources/iso_volume.hpp"
#include "resources/external_utils.hpp"

namespace fs = std::filesystem;

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// TODO extract audio from iso
namespace studio::resources::iso
{
  namespace endian = studio::endian;
  using folder_info = studio::resources::folder_info;
  constexpr auto cue_file_record_tag = shared::to_tag<4>({ 'F', 'I', 'L', 'E' });
  constexpr auto ccd_file_record_tag = shared::to_tag<9>({ '[', 'C', 'l', 'o', 'n', 'e', 'C', 'D', ']' });
  constexpr auto mds_file_record_tag = shared::to_tag<16>({ 'M', 'E', 'D', 'I', 'A', ' ', 'D', 'E', 'S', 'C', 'R', 'I', 'P', 'T', 'O', 'R' });

  constexpr auto iso_offset = 32768;
  constexpr auto iso_file_record_tag = shared::to_tag<6>({ 0x01, 'C', 'D', '0', '0', '1' });

  enum struct mds_medium : std::uint16_t
  {
    cd = 0x00,
    cd_r = 0x01,
    cd_rw = 0x02,
    dvd = 0x10,
    dvd_r = 0x12
  };

  struct mds_header
  {
    std::array<std::byte, 16> file_tag;
    std::array<std::uint8_t, 2> file_version;
    endian::little_uint16_t medium_type;
    endian::little_uint16_t num_sessions;
    std::array<std::byte, 58> unused;
    endian::little_uint32_t sessions_offset;
    std::array<std::byte, 4> unused2;
  };

  static_assert(sizeof(mds_header) == 88);

  struct mds_session
  {
    std::array<std::byte, 12> unused;
    endian::little_uint16_t first_track;
    endian::little_uint16_t last_track;
    std::array<std::byte, 4> unused2;
    endian::little_uint32_t tracks_offset;
  };

  static_assert(sizeof(mds_session) == 24);

  enum struct mds_track_mode : std::uint8_t
  {
    unknown = 0x00,
    audio = 0xa9,
    mode1 = 0xaa,
    mode2 = 0xab,
    mode2_form1 = 0xac,
    mode2_form2 = 0xad
  };

  enum struct mds_sub_channel_mode : std::uint8_t
  {
    none = 0x00,
    interleaved = 0x08// 96-byte sector suffix
  };

  constexpr auto sub_channel_suffix_size = 96u;

  struct mds_track
  {
    mds_track_mode mode;
    mds_sub_channel_mode sub_channel;

    std::array<std::byte, 14> unused;
    endian::little_int16_t sector_size;
    std::array<std::byte, 22> unused2;
    endian::little_int64_t offset;
    endian::little_int32_t num_files;
    std::array<std::byte, 28> unused3;
  };

  static_assert(sizeof(mds_track) == 80);

  struct mds_extra_track_data
  {
    std::array<std::byte, 8> unused;
  };

  static_assert(sizeof(mds_extra_track_data) == 8);

  struct mds_footer
  {
    std::array<std::byte, 16> unused;
  };

  static_assert(sizeof(mds_footer) == 16);

  std::optional<std::size_t> get_frames(const std::string& time_compound)
  {
    auto last_colon = time_compound.rfind(':');
    auto middle_colon = time_compound.rfind(':', last_colon - 1);

    try
    {
      auto minutes = std::stoi(time_compound.substr(middle_colon - 2, 2)) * 60 * 75;
      auto seconds = std::stoi(time_compound.substr(middle_colon + 1, 2)) * 75;
      auto frame = std::stoi(time_compound.substr(last_colon + 1, 2));

      return minutes + seconds + frame;
    }
    catch (...)
    {
    }

    return std::nullopt;
  }

  struct track_header
  {
    std::array<std::byte, 12> sync_pattern;
    std::byte hour;
    std::byte minute;
    std::byte frame;
    std::byte mode;
  };

  static_assert(sizeof(track_header) == 16);

  constexpr auto sector_size = 2352;
  constexpr auto mode_1_data_size = 2048;
  constexpr auto mode_1_remaining_size = sector_size - sizeof(track_header) - mode_1_data_size;
  constexpr auto mode_2_data_size = 2336;
  constexpr auto common_data_pattern = shared::to_tag<12>({ 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 });

  // TODO simplify the mess, lol.
  // query example
  // initial query
  //  archive_path = c:\downloads\game.cue
  //  folder_path = c:\downloads\game.cue
  // next query
  //  archive_path = c:\downloads\game.cue
  //  folder_path = c:\downloads\game.cue\game.bin
  // next query
  //  archive_path = c:\downloads\game.cue
  //  folder_path = c:\downloads\game.cue\game.bin\some_folder


  // test.cue
  //  test.bin
  //  test_track-1.cdda
  //  test_track-2.cdda
  //  test_track-3.cdda
  std::vector<iso_file_archive::content_info> cue_get_content_listing(std::istream& input, const listing_query& query)
  {
    if (query.archive_path == query.folder_path)
    {
      std::vector<iso_file_archive::content_info> results;
      results.reserve(16);

      auto lines = read_lines(input);

      std::vector<iso_file_archive::file_info> tracks;

      while (!lines.empty())
      {
        if (lines.size() < 3)
        {
          break;
        }

        auto file = std::move(lines.front());
        lines.erase(lines.begin());

        auto file_index = file.find("FILE");
        auto binary_index = file.rfind("BINARY");

        if (file_index == std::string::npos || binary_index == std::string::npos)
        {
          continue;
        }

        auto file_name = file.substr(file_index, binary_index - file_index);

        if (file_name.empty())
        {
          continue;
        }

        auto auto_track_number = 0;
        while (!lines.empty() && lines.front().find("TRACK") != std::string::npos)
        {
          auto track = std::move(lines.front());
          lines.erase(lines.begin());
          auto_track_number++;

          std::vector<std::string> indexes;
          while (!lines.empty() && lines.front().find("INDEX") != std::string::npos)
          {
            indexes.emplace_back(std::move(lines.front()));
            lines.erase(lines.begin());
          }

          if (indexes.empty())
          {
            continue;
          }

          auto time_offset = get_frames(indexes[0]);

          if (!time_offset.has_value())
          {
            continue;
          }

          auto byte_offset = time_offset.value() * sector_size;

          if (track.find("AUDIO") != std::string::npos)
          {
            auto track_number = auto_track_number;

            try
            {
              auto index = track.find("TRACK") + std::strlen("TRACK") + 1;
              track_number = std::stoi(track.substr(index, 2));
            }
            catch (...)
            {
            }

            file_info info{};
            info.archive_path = query.archive_path;
            info.compression_type = compression_type::none;
            info.offset = byte_offset;
            info.folder_path = query.archive_path / file_name;
            info.filename = "track" + std::to_string(track_number) + ".cdda";
            info.size = fs::file_size(query.archive_path.parent_path() / file_name);

            tracks.emplace_back(info);
          }
          else
          {
            auto iso_contents = iso_get_content_listing(listing_query{
              query.archive_path.parent_path() / file_name,
              query.archive_path.parent_path() / file_name });

            // TODO make the result optional so that we know if the correct software is installed or not

            folder_info folder{};
            folder.archive_path = query.archive_path;
            folder.name = file_name;
            folder.full_path = query.archive_path / file_name;

            if (iso_contents.empty())
            {
              folder.full_path = folder.full_path.replace_extension(".iso");
            }

            results.emplace_back(folder);
          }
        }
      }

      auto has_same_file = !tracks.empty() && std::all_of(tracks.begin(), tracks.end(), [&](const auto& track) {
        return tracks[0].filename == track.filename;
      });

      if (has_same_file)
      {
        auto file_size = tracks[0].size;
        for (auto i = 0u; i < tracks.size(); ++i)
        {
          auto& track = tracks[i];

          if (i + 1 < tracks.size())
          {
            track.size = tracks[i + 1].offset - tracks[i].offset;
          }
          else
          {
            track.size = file_size - tracks[i].offset;
          }
        }
      }

      return results;
    }

    auto relative_path = fs::relative(query.folder_path, query.archive_path);
    auto bin_path = query.folder_path.parent_path() / relative_path.root_path();

    if (!fs::exists(bin_path))
    {
      return {};
    }

    auto listing = iso_get_content_listing(listing_query {
      bin_path,
      bin_path / relative_path
    });

    if (!listing.empty())
    {
      // TODO these also need mapping because bin path
      // comes from a virtual path.
      return listing;
    }

    std::ifstream data_file{ bin_path, std::ios::binary };

    std::ofstream new_iso_file{ fs::temp_directory_path() / bin_path.filename(), std::ios::binary | std::ios::trunc };

    track_header header{};
    std::vector<std::byte> data_buffer(mode_2_data_size);

    while (!data_file.eof())
    {
      data_file.read(reinterpret_cast<char*>(&header), sizeof(header));

      if (header.sync_pattern != common_data_pattern)
      {
        break;
      }

      if (header.mode != std::byte(0x01))
      {
        data_file.seekg(mode_2_data_size, std::ios::cur);
        continue;
      }

      data_file.read(reinterpret_cast<char*>(data_buffer.data()), mode_1_data_size);
      data_file.seekg(mode_1_remaining_size, std::ios::cur);
      new_iso_file.write(reinterpret_cast<char*>(data_buffer.data()), mode_1_data_size);
    }

    const auto temp_archive_path = fs::temp_directory_path() / bin_path.filename();

    auto temp_results = iso_get_content_listing(listing_query {
      temp_archive_path,
      temp_archive_path / relative_path  });

    static std::unordered_map<std::string, studio::resources::content_info> mapped_paths;

    // TODO this doesn't compile in MSVC :(
      // ICE errors are the worst
    std::transform(temp_results.begin(), temp_results.end(), temp_results.begin(), [&](auto& info) {
      return std::visit(
        overloaded {
          [&](studio::resources::folder_info& arg) -> studio::resources::content_info {
          // commented out to fix ICE
            // auto original = arg;

            // // TODO remap the directory to the archive path and not temp
            // arg.full_path = query.archive_path / fs::relative(arg.full_path, temp_archive_path);
            // arg.archive_path = query.archive_path;
            // mapped_paths.emplace(arg.full_path.string(), std::move(original));

            return std::move(arg);
          },
            [&](studio::resources::file_info& arg) -> studio::resources::content_info  {
              // commented out to fix ICE
            // auto original = arg;
            // // TODO remap the directory to the archive path and not temp
            // arg.folder_path = query.archive_path / fs::relative(arg.folder_path, temp_archive_path);
            // arg.archive_path = query.archive_path;

            // mapped_paths.emplace((arg.folder_path / arg.filename).string(), std::move(original));

            return std::move(arg);
            }
        }, info);
    });

    return temp_results;
  }

  std::vector<iso_file_archive::content_info> mds_get_content_listing(std::istream& input, const listing_query& query)
  {
    std::vector<iso_file_archive::content_info> results;

    auto pos = input.tellg();
    mds_header header{};
    input.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (mds_medium(header.medium_type.value()) == mds_medium::dvd || mds_medium(header.medium_type.value()) == mds_medium::dvd_r)
    {
      return results;
    }

    auto max_size = fs::file_size(fs::path(query.archive_path).replace_extension(".mdf"));

    if (max_size == 0)
    {
      return results;
    }

    results.reserve(16);

    std::vector<mds_session> sessions;
    sessions.reserve(header.num_sessions);

    input.seekg(std::uint32_t(pos) + header.sessions_offset, std::ios::beg);

    for (auto i = 0u; i < header.num_sessions; ++i)
    {
      auto& session = sessions.emplace_back();
      input.read(reinterpret_cast<char*>(&session), sizeof(session));
    }

    for (const auto& session : sessions)
    {
      std::vector<mds_track> tracks;
      tracks.reserve(session.last_track);
      input.seekg(std::uint32_t(pos) + session.tracks_offset, std::ios::beg);

      for (auto i = 0u; session.last_track; ++i)
      {
        auto& track = tracks.emplace_back();
        input.read(reinterpret_cast<char*>(&track), sizeof(track));
      }

      // TODO open a file for real or something like that
      //      std::istream& image = input;
      //      std::vector<std::byte> data;

      for (auto i = 0u; i < tracks.size(); ++i)
      {
        const auto& track = tracks[i];
        if (track.mode != mds_track_mode::audio)
        {
          continue;
        }

        file_info music_track{};
        music_track.offset = std::size_t(track.offset);
        music_track.compression_type = track.sub_channel == mds_sub_channel_mode::interleaved ? compression_type::rle : compression_type::none;
        music_track.archive_path = query.archive_path;
        music_track.folder_path = query.archive_path / query.archive_path.filename().replace_extension(".mdf");
        music_track.filename = "track-" + std::to_string(tracks.size() + 1) + ".cdda";

        if (i == tracks.size() - 1)
        {
          music_track.size = max_size - std::size_t(track.offset);
        }
        else
        {
          music_track.size = std::size_t(tracks[i].offset - track.offset);
        }

        results.emplace_back(music_track);

        // TODO open a file for real or something like that
        //        std::stringstream temp;
        //        std::ostream& file = temp;
        //
        //        image.seekg(track.offset, std::ios::beg);
        //        data.assign(track.sector_size, std::byte{});
        //        image.read(reinterpret_cast<char*>(data.data()), track.sector_size);

        //        if (track.sub_channel == mds_sub_channel_mode::interleaved)
        //        {
        //          file.write(reinterpret_cast<char*>(data.data()), std::streamsize(data.size() - sub_channel_suffix_size));
        //        }
        //        else
        //        {
        //          file.write(reinterpret_cast<char*>(data.data()), std::streamsize(data.size()));
        //        }
      }
    }

    return results;
  }

  std::string rtrim(std::string str)
  {
    auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    str.erase(it1.base(), str.end());
    return str;
  }

  template<std::size_t Size>
  auto to_array(const std::vector<std::byte>& bytes)
  {
    std::array<std::byte, Size> results{};
    std::copy_n(bytes.begin(), Size, results.begin());
    return results;
  }

  bool iso_file_archive::is_supported(std::istream& stream)
  {
    std::vector<std::byte> tag(16);
    stream.read(reinterpret_cast<char*>(tag.data()), std::streamsize(tag.size()));

    stream.seekg(-int(tag.size()), std::ios::cur);
    if (to_array<cue_file_record_tag.size()>(tag) == cue_file_record_tag || to_array<ccd_file_record_tag.size()>(tag) == ccd_file_record_tag || to_array<mds_file_record_tag.size()>(tag) == mds_file_record_tag)
    {
      return true;
    }

    stream.seekg(iso_offset, std::ios::cur);
    stream.read(reinterpret_cast<char*>(tag.data()), std::streamsize(tag.size()));

    stream.seekg(-int(tag.size()), std::ios::cur);
    stream.seekg(-int(iso_offset), std::ios::cur);

    return to_array<iso_file_record_tag.size()>(tag) == iso_file_record_tag;
  }

  bool iso_file_archive::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<iso_file_archive::content_info> iso_file_archive::get_content_listing(std::istream& stream, const listing_query& query) const
  {
    return iso_get_content_listing(query);
  }

  void iso_file_archive::set_stream_position(std::istream& stream, const studio::resources::file_info& info) const
  {
  }

  void iso_file_archive::extract_file_contents(std::istream& stream,
    const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage) const
  {
    iso_extract_file_contents(info, output, storage);
  }
}// namespace studio::resources::iso
