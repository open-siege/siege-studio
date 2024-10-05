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

  bool cab_resource_reader::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == is5_cab_tag ||
           tag == is2_cab_tag ||
           tag == ms_cab_tag;
  }

  bool cab_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<cab_resource_reader::content_info> cab_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    return cab_get_content_listing(query);
  }

  void cab_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {

  }

  void cab_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage) const
  {
    cab_extract_file_contents(info, output, storage);
  }
}// namespace darkstar::vol
