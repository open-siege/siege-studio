#include "bmp_controller.hpp"
#include <siege/content/bmp/bitmap.hpp>


namespace siege::views
{
  bool bmp_controller::is_bmp(std::istream& image_stream)
  {
      return siege::content::bmp::is_earthsiege_bmp(image_stream)
           || siege::content::bmp::is_microsoft_bmp(image_stream)
          || siege::content::bmp::is_phoenix_bmp(image_stream);
  }

  std::size_t bmp_controller::load_bitmap(std::istream& image_stream)
  {
      using namespace siege::content;

      auto start = image_stream.tellg();

      if (bmp::is_microsoft_bmp(image_stream))
      {
        raw_image = bmp::get_bmp_data(image_stream);
      }
      else if (bmp::is_phoenix_bmp(image_stream))
      {
        raw_image = bmp::get_pbmp_data(image_stream);
      }
      else if (bmp::is_earthsiege_bmp(image_stream))
      {
        raw_image = bmp::read_earthsiege_bmp(image_stream);
      }

     return image_stream.tellg() - start;
  }
}