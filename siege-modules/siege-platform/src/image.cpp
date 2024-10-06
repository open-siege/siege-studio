#include <vector>
#include <siege/platform/image.hpp>
#include <siege/platform/tagged_data.hpp>
#include <siege/platform/stream.hpp>

namespace siege::platform::bitmap
{
  using file_tag = std::array<std::byte, 4>;
  constexpr file_tag png_tag = platform::to_tag<4>({ 0x89, 'P', 'N', 'G' });
  constexpr std::array<std::byte, 3> gif_tag = platform::to_tag<3>({ 'G', 'I', 'F' });
  constexpr std::array<std::byte, 2> jpg_tag = platform::to_tag<2>({ 0xff, 0xd8 });

  bool is_jpg(std::istream& raw_data)
  {
    std::array<std::byte, 2> header{};
    raw_data.read(reinterpret_cast<char *>(header.data()), sizeof(header));

    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == jpg_tag;
  }

  bool is_png(std::istream& raw_data)
  {
    file_tag header{};
    raw_data.read(reinterpret_cast<char *>(header.data()), sizeof(header));

    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == png_tag;
  }

  bool is_gif(std::istream& raw_data)
  {
    std::array<std::byte, 3> header{};
    raw_data.read(reinterpret_cast<char *>(header.data()), sizeof(header));

    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == gif_tag;
  }


  bool is_tga(std::istream& raw_data)
  {
    std::array<std::byte, 3> header{};
    raw_data.read(reinterpret_cast<char *>(header.data()), sizeof(header));

    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    // TODO turn this into an enum
    return header[2] == std::byte{0} || header[2] == std::byte{1} ||
           header[2] == std::byte{2} || header[2] == std::byte{3} ||
           header[2] == std::byte{9} || header[2] == std::byte{10} ||
           header[2] == std::byte{11};
  }
}