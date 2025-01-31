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

  struct size
  {
    int width;
    int height;

    size(const size&) = default;
    size(size&&) = default;
    size& operator=(const size&) = default;
    size& operator=(size&&) = default;

    size() : width(0), height(0)
    {
    }

    size(int width, int height) : width(width), height(height)
    {

    }

    size(std::pair<int, int> pair) : width(pair.first), height(pair.second)
    {
    }
  };

  class platform_image
  {
  public:
    explicit platform_image(std::span<platform::bitmap::windows_bmp_data>);
    explicit platform_image(platform::bitmap::windows_bmp_data);
    explicit platform_image(std::filesystem::path);
    explicit platform_image(std::istream&);

    std::size_t frame_count() const noexcept;
    size get_size(std::size_t frame) const noexcept;
    std::size_t convert(std::size_t frame, size size, int bits, std::span<std::byte> destination) const noexcept;
  private:
    std::vector<std::any> frames;
  };
}// namespace siege::platform::bitmap

#endif// OPEN_SIEGE_IMAGE_HPP
