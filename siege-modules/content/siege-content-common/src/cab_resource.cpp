module;
#include <cstdlib>

export module siege.resource.cab;

import std;

import siege.platform.endian;
import siege.platform.resource;
import siege.platform.shared;
import siege.resource.ext;

namespace fs = std::filesystem;

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

namespace siege::resource::cab
{
  using folder_info = siege::platform::folder_info;

  constexpr auto is5_cab_tag = platform::to_tag<4>({ 'I', 'S', 'c', 0x28 });
  constexpr auto is2_cab_tag = platform::to_tag<4>({ 0x13, 0x5d, 0x65, 0x8c });
  constexpr auto ms_cab_tag = platform::to_tag<4>({ 'M', 'S', 'C', 'F' });

  using content_info = platform::resource_reader::content_info;
  using folder_info = platform::resource_reader::folder_info;
  using file_info = platform::resource_reader::file_info;

  export bool is_stream_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == is5_cab_tag || tag == is2_cab_tag || tag == ms_cab_tag;
  }

  std::vector<content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query)
  {
    return cab_get_content_listing(cache, query);
  }

  void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output)
  {
    cab_extract_file_contents(cache, info, output);
  }

  export siege::platform::resource_reader make_resource_reader()
  {
    return {
      is_stream_supported,
      get_content_listing,
      nullptr,
      extract_file_contents
    };
  }
}// namespace siege::resource::cab
