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

  export std::size_t download_http_data(http_client_context&, fs_string domain, fs_string remote_path, std::ostream& output, http_callbacks callbacks = {});
}// namespace siege::platform
