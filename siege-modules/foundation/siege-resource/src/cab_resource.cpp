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

#include <siege/resource/cab_resource.hpp>
#include <siege/resource/external_utils.hpp>

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace siege::resource::cab
{
  using folder_info = siege::platform::folder_info;

  constexpr auto is5_cab_tag = platform::to_tag<4>({ 'I', 'S', 'c', 0x28 });
  constexpr auto is2_cab_tag = platform::to_tag<4>({ 0x13, 0x5d, 0x65, 0x8c });
  constexpr auto ms_cab_tag = platform::to_tag<4>({ 'M', 'S', 'C', 'F' });

  bool stream_is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == is5_cab_tag ||
           tag == is2_cab_tag ||
           tag == ms_cab_tag;
  }

  std::vector<cab_resource_reader::content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query)
  {
    return cab_get_content_listing(cache, query);
  }

  void extract_file_contents(std::any& cache, std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output)
  {
    cab_extract_file_contents(cache, info, output);
  }
}// namespace darkstar::vol
