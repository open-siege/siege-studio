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

  constexpr auto file_record_tag = shared::to_tag<4>({ '7', 'z', 0xbc, 0xaf });

  std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }


  // TODO add proper checks for if a file is supported
  bool iso_file_archive::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag != file_record_tag;
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
