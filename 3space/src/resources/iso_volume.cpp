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

#include "resources/iso_volume.hpp"
#include "resources/external_utils.hpp"

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace studio::resources::iso
{
  using folder_info = studio::resources::folder_info;
  constexpr auto cue_file_record_tag = shared::to_tag<4>({ 'F', 'I', 'L', 'E' });
  constexpr auto ccd_file_record_tag = shared::to_tag<9>({ '[', 'C', 'l', 'o', 'n', 'e', 'C', 'D', ']' });
  constexpr auto mds_file_record_tag = shared::to_tag<16>({ 'M', 'E', 'D', 'I', 'A', ' ','D', 'E', 'S', 'C', 'R', 'I', 'P', 'T', 'O','R'});

  constexpr auto iso_offset = 32768;
  constexpr auto iso_file_record_tag = shared::to_tag<6>({ 0x01, 'C', 'D', '0', '0', '1' });

  std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
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
    stream.read(reinterpret_cast<char *>(tag.data()), std::streamsize(tag.size()));

    stream.seekg(-int(tag.size()), std::ios::cur);
    if (to_array<cue_file_record_tag.size()>(tag) == cue_file_record_tag ||
        to_array<ccd_file_record_tag.size()>(tag) == ccd_file_record_tag ||
      to_array<mds_file_record_tag.size()>(tag) == mds_file_record_tag)
    {
      return true;
    }

    stream.seekg(iso_offset, std::ios::cur);
    stream.read(reinterpret_cast<char *>(tag.data()), std::streamsize(tag.size()));

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
}// namespace darkstar::vol
