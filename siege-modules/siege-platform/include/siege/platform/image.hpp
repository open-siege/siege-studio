#ifndef OPEN_SIEGE_IMAGE_HPP
#define OPEN_SIEGE_IMAGE_HPP

#include <fstream>
#include <array>
#include <vector>
#include <any>
#include <span>
#include <siege/platform/bitmap.hpp>

namespace siege::platform::bitmap
{
  bool is_png(std::istream& raw_data);
  bool is_jpg(std::istream& raw_data);
  bool is_gif(std::istream& raw_data);
  bool is_tga(std::istream& raw_data);

  class platform_image
  {
  public:
    explicit platform_image(std::span<platform::bitmap::windows_bmp_data>);
    explicit platform_image(platform::bitmap::windows_bmp_data);
    explicit platform_image(std::filesystem::path);
    explicit platform_image(std::istream&);

    std::size_t frame_count() const;

    std::size_t convert(std::size_t frame, std::pair<int, int> size, int bits, std::span<std::byte> destination) const noexcept;

  private:
    std::vector<std::any> frames;
  };
}// namespace siege::content::bmp

#endif// OPEN_SIEGE_IMAGE_HPP
