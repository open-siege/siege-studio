#include <siege/resource/common_resources.hpp>

namespace siege::resource
{
  bool is_resource_readable(std::istream& stream)
  {
    return zip::is_stream_zip(stream) || cab::is_stream_supported(stream) || iso::is_stream_supported(stream);
  }

  siege::platform::resource_reader make_resource_reader(std::istream& stream)
  {
    if (zip::is_stream_zip(stream))
    {
      return zip::make_zip_resource_reader();
    }
    else if (zip::is_stream_7zip(stream))
    {
      return zip::make_7zip_resource_reader();
    }
    else if (cab::is_stream_supported(stream))
    {
      return cab::make_resource_reader();
    }
    else if (iso::is_stream_supported(stream))
    {
      return iso::make_resource_reader();
    }
    else
    {
      throw std::invalid_argument("Stream provided is not supported");
    }
  }
}// namespace siege::resource