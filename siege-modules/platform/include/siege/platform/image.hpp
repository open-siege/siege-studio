#ifndef OPEN_SIEGE_IMAGE_HPP
#define OPEN_SIEGE_IMAGE_HPP

#include <fstream>
#include <array>
#include <vector>
#include <any>
#include <span>
#include <siege/platform/bitmap.hpp>

#if WIN32
#include <siege/platform/win/wic.hpp>
#endif

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

    bool operator==(const size& b) const
    {
      return this->width == b.width && this->height == b.height;
    }
  };

  class platform_image
  {
  public:
    explicit platform_image(std::span<platform::bitmap::windows_bmp_data>);
    explicit platform_image(platform::bitmap::windows_bmp_data);
    explicit platform_image(std::filesystem::path);
    explicit platform_image(std::istream&, bool deep_copy = false);

    std::size_t frame_count() const noexcept;
    size get_size(std::size_t frame) const noexcept;

#if WIN32
    inline win32::wic::bitmap_source& at(std::size_t frame)
    {
      return frames.at(frame);
    }

  private:
    std::vector<win32::wic::bitmap_source> frames;
#endif
  };
}// namespace siege::platform::bitmap

#endif// OPEN_SIEGE_IMAGE_HPP
