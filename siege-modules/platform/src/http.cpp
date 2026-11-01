export module siege.platform.http;

import std;

export import siege.platform.shared;

namespace siege::platform
{
  export struct http_client_context : std::any
  {
    using std::any::any;
  };

  export struct http_callbacks
  {
    std::function<void(std::size_t)> on_received_content_length;
    std::function<void(std::size_t)> on_transmitted;
  };

  // TODO implement this for other platforms as needed
  export std::size_t download_http_data(http_client_context&, fs_string, fs_string, std::ostream&, http_callbacks = {})
  {
    return 0;
  }
}// namespace siege::platform
