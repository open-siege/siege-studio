#include "bmp_controller.hpp"
#include <siege/content/bmp/bitmap.hpp>
#include <siege/content/bmp/image.hpp>

namespace siege::views
{
  bool bmp_controller::is_bmp(std::istream& image_stream) noexcept
  {
      return siege::content::bmp::is_earthsiege_bmp(image_stream)
           || siege::content::bmp::is_microsoft_bmp(image_stream)
          || siege::content::bmp::is_phoenix_bmp(image_stream)
          || siege::content::bmp::is_jpg(image_stream)
          || siege::content::bmp::is_gif(image_stream)
          || siege::content::bmp::is_png(image_stream);
  }

  std::size_t bmp_controller::load_bitmap(std::istream& image_stream) noexcept
  {
      using namespace siege::content;

      try
      {
          if (bmp::is_microsoft_bmp(image_stream))
          {
                original_image.emplace(bmp::get_bmp_data(image_stream));
          }
          else if (bmp::is_phoenix_bmp(image_stream))
          {
          }
          else if (bmp::is_earthsiege_bmp(image_stream))
          {
          }
          else 
          {
            original_image.emplace(image_stream);
          }
      }
      catch(...)
      {
      }

      if (original_image)
      {
        return original_image->frame_count();
      }

     return 0;
  }

  std::size_t bmp_controller::convert(std::size_t frame, std::pair<int, int> size, int bits, std::span<std::byte> destination) const noexcept
  {
    if (original_image)
    {
        return original_image->convert(frame, size, bits, destination);
    }

    return 0;
  }
}