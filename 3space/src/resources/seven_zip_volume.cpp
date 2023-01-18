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

#include "resources/seven_zip_volume.hpp"
#include "resources/external_utils.hpp"

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace studio::resources::seven_zip
{
  using folder_info = studio::resources::folder_info;

  constexpr auto seven7_file_record_tag = shared::to_tag<4>({ '7', 'z', 0xbc, 0xaf });
  constexpr auto gz_deflate_file_record_tag = shared::to_tag<4>({ 0x1f, 0x8b, 0x08, 0x00 });
  constexpr auto rar_file_record_tag = shared::to_tag<4>({ 'R', 'a', 'r', '!' });
  constexpr auto common_exe_tag = shared::to_tag<4>({ 'M', 'Z', 0x90, 0x00 });

  std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }

  bool seven_zip_file_archive::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == seven7_file_record_tag ||
           tag == gz_deflate_file_record_tag ||
           tag == rar_file_record_tag ||
           tag == common_exe_tag;
  }

  bool seven_zip_file_archive::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<seven_zip_file_archive::content_info> seven_zip_file_archive::get_content_listing(std::istream& stream, const listing_query& query) const
  {
    return zip_get_content_listing(query);
  }

  void seven_zip_file_archive::set_stream_position(std::istream& stream, const studio::resources::file_info& info) const
  {

  }

  void seven_zip_file_archive::extract_file_contents(std::istream& stream,
    const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage) const
  {
    seven_extract_file_contents(info, output, storage);
  }
}// namespace darkstar::vol
