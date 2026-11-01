export module siege.platform.image;

import std;

export import siege.platform.bitmap;
import siege.platform.stream;

// TODO implement for other platforms as needed
namespace siege::platform::bitmap
{
  export class platform_image
  {
  public:
    explicit platform_image(std::span<windows_bmp_data>)
    {
    }

    explicit platform_image(windows_bmp_data)
    {
    }

    explicit platform_image(std::filesystem::path)
    {
    }

    explicit platform_image(std::istream&, bool = false)
    {
    }

    std::size_t frame_count() const noexcept
    {
      return 0;
    }

    size get_size(std::size_t) const noexcept
    {
      return {};
    }
  };
}// namespace siege::platform::bitmap
