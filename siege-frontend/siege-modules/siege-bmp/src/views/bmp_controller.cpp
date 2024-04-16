#include "bmp_controller.hpp"
#include "content/bmp/bitmap.hpp"


namespace siege::views
{
  bool bmp_controller::is_bmp(std::istream& image_stream)
  {
      return studio::content::bmp::is_earthsiege_bmp(image_stream)
           || studio::content::bmp::is_microsoft_bmp(image_stream)
          || studio::content::bmp::is_phoenix_bmp(image_stream);
  }
}