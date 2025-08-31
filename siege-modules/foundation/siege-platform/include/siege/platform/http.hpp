#include <any>
#include <functional>
#include <string>
#include <siege/platform/shared.hpp>

namespace siege::platform
{
  struct http_client_context : std::any
  {
      using std::any::any;
  };

  struct http_callbacks
  {
    std::function<void(std::size_t)> on_received_content_length;
    std::function<void(std::size_t)> on_transmitted;
  };

  std::size_t download_http_data(http_client_context&, fs_string domain, fs_string remote_path, std::ostream& output, http_callbacks callbacks = {});
}// namespace siege::platform