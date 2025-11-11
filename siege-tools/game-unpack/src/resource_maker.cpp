#include <siege/resource/zip_resource.hpp>
#include <siege/resource/iso_resource.hpp>
#include <siege/resource/seven_zip_resource.hpp>
#include <siege/resource/cab_resource.hpp>

namespace siege::resource
{
  bool is_resource_readable(std::istream& stream)
  {
    return zip::stream_is_supported(stream) || cab::stream_is_supported(stream) || iso::stream_is_supported(stream);
  }

  siege::platform::resource_reader make_resource_reader(std::istream& stream)
  {
    if (zip::stream_is_supported(stream))
    {
      return zip::zip_resource_reader();
    }
    else if (cab::stream_is_supported(stream))
    {
      return cab::cab_resource_reader();
    }
    else if (iso::stream_is_supported(stream))
    {
      return iso::iso_resource_reader();
    }
    else
    {
      throw std::invalid_argument("Stream provided is not supported");
    }
  }
}// namespace siege::resource