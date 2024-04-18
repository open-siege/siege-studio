#include <vector>
#include <SFML/Graphics.hpp>
#include "image.hpp"

namespace siege::content::bmp
{
  using file_tag = std::array<std::byte, 4>;
  constexpr file_tag png_tag = platform::to_tag<4>({ 0x89, 'P', 'N', 'G' });
  constexpr std::array<std::byte, 3> gif_tag = platform::to_tag<3>({ 'G', 'I', 'F' });
  constexpr std::array<std::byte, 2> jpg_tag = platform::to_tag<2>({ 0xff, 0x28 });

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

  windows_bmp_data get_supported_data(const siege::platform::file_info& info, std::istream& raw_data)
  {
    sf::Image image;

    std::vector<std::byte> data(info.size);
    raw_data.read(reinterpret_cast<char *>(data.data()), info.size);

    if (image.loadFromMemory(data.data(), info.size))
    {
      windows_bmp_header header{};
      windows_bmp_info bmp_info{};
      std::vector<pal::colour> colours;
      std::vector<std::int32_t> indexes;

      const auto [width, height] = image.getSize();

      bmp_info.bit_depth = 32;
      bmp_info.width = width;
      bmp_info.height = height;

      const auto num_pixels = width * height;
      auto* pixels = reinterpret_cast<const std::array<std::byte, 4>*>(image.getPixelsPtr());
      std::transform(pixels, pixels + num_pixels, std::back_inserter(colours), [](auto& colour) {
        return pal::colour{ colour[0], colour[1], colour[2], colour[3] };
      });

      return {
        header,
        bmp_info,
        colours,
        indexes
      };
    }

    return {};
  }

  windows_bmp_data get_png_data(const siege::platform::file_info& info, std::istream& raw_data)
  {
      return get_supported_data(info, raw_data);
  }

  windows_bmp_data get_jpg_data(const siege::platform::file_info& info, std::istream& raw_data)
  {
    return get_supported_data(info, raw_data);
  }

  windows_bmp_data get_gif_data(const siege::platform::file_info& info, std::istream& raw_data)
  {
    return get_supported_data(info, raw_data);
  }

  windows_bmp_data get_tga_data(const siege::platform::file_info& info, std::istream& raw_data)
  {
    return get_supported_data(info, raw_data);
  }
}