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

#include <siege/resource/common_resources.hpp>
#include <siege/resource/external_utils.hpp>

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace siege::resource::zip
{
  using content_info = platform::resource_reader::content_info;
  using folder_info = platform::resource_reader::folder_info;
  using file_info = platform::resource_reader::file_info;

  constexpr auto seven7_file_record_tag = platform::to_tag<4>({ '7', 'z', 0xbc, 0xaf });
  constexpr auto gz_deflate_file_record_tag = platform::to_tag<4>({ 0x1f, 0x8b, 0x08, 0x00 });
  constexpr auto rar_file_record_tag = platform::to_tag<4>({ 'R', 'a', 'r', '!' });
  constexpr auto common_exe_tag = platform::to_tag<4>({ 'M', 'Z', 0x90, 0x00 });

  std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }

  bool is_stream_7zip(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == seven7_file_record_tag ||
           tag == gz_deflate_file_record_tag ||
           tag == rar_file_record_tag ||
           tag == common_exe_tag;
  }

  std::vector<content_info> get_zip_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query)
  {
    platform::istream_pos_resetter resetter(stream);
    return zip_get_content_listing(cache, query);
  }

  void extract_zip_file_contents(std::any& storage, std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output)
  {
    seven_extract_file_contents(storage, info, output);
  }

  siege::platform::resource_reader make_7zip_resource_reader()
  {
    return {
      is_stream_7zip,
      get_zip_content_listing,
      nullptr,
      extract_zip_file_contents
    };
  }
}// namespace darkstar::vol
