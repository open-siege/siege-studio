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
          || siege::content::bmp::is_png(image_stream);
  }

  std::size_t bmp_controller::load_bitmap(std::istream& image_stream) noexcept
  {
      using namespace siege::content;

      auto start = image_stream.tellg();

      if (bmp::is_microsoft_bmp(image_stream))
      {
        try
        {
            original_images.emplace_back(bmp::get_bmp_data(image_stream));
            return 1;        
        }
        catch(std::exception& ex)
        {
            OutputDebugStringA(ex.what());
        }

      }
      else if (bmp::is_phoenix_bmp(image_stream))
      {
      }
      else if (bmp::is_earthsiege_bmp(image_stream))
      {
      }
      else
      {
          try
          {
                original_images.emplace_back(image_stream);
                return 1;        
          }
          catch(std::exception& ex)
          {
            OutputDebugStringA(ex.what());
          }
      }

     return 0;
  }

  content::bmp::platform_bitmap& bmp_controller::get_bitmap(std::size_t index) noexcept
  {
    assert(original_images.size() > 0);

    return original_images[index];
  }
}