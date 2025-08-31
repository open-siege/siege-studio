#include <SDKDDKVer.h>
#include <siege/platform/http.hpp>
#include <siege/platform/win/file.hpp>
#include <winhttp.h>

namespace siege::platform
{
  struct winhttp_context
  {
    std::shared_ptr<void> session;
    std::shared_ptr<void> connection;
  };

  winhttp_context& get(http_client_context& context)
  {
    if (!context.has_value())
    {
      context = winhttp_context{};
    }

    return std::any_cast<winhttp_context&>(context);
  }

  std::size_t download_http_data(http_client_context& context, fs_string domain, fs_string remote_path, std::ostream& output, http_callbacks callbacks)
  {
    auto& self = get(context);

    if (!self.session)
    {
      self.session = std::shared_ptr<void>(::WinHttpOpen(nullptr, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0), ::WinHttpCloseHandle);
    }

    if (!self.connection)
    {
      self.connection = std::shared_ptr<void>(::WinHttpConnect(self.session.get(), domain.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0), ::WinHttpCloseHandle);
    }

    auto request = ::WinHttpOpenRequest(self.connection.get(), L"GET", remote_path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    auto sent = ::WinHttpSendRequest(request,
      WINHTTP_NO_ADDITIONAL_HEADERS,
      0,
      WINHTTP_NO_REQUEST_DATA,
      0,
      0,
      0);

    sent = ::WinHttpReceiveResponse(request, nullptr);

    thread_local std::vector<char> buffer;
    DWORD transmitted = 0;
    DWORD total_transmitted = 0;

    if (sent)
    {
      DWORD content_length = 0;
      DWORD buffer_size = sizeof(content_length);
      DWORD header_index = 0;

      if (::WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &content_length, &buffer_size, &header_index) && 
          callbacks.on_received_content_length)
      {
        callbacks.on_received_content_length(content_length);
      }

      DWORD size;
      do {

        size = 0;
        if (!::WinHttpQueryDataAvailable(request, &size))
        {
          break;
        }

        buffer.resize(size);

        if (!::WinHttpReadData(request, buffer.data(), size, &transmitted))
        {
          break;
        }

        total_transmitted += transmitted;

        if (callbacks.on_transmitted)
        {
          callbacks.on_transmitted((std::size_t)total_transmitted);
        }

        output.write(buffer.data(), buffer.size());
      } while (size > 0);
    }

    ::WinHttpCloseHandle(request);
    return (std::size_t)total_transmitted;
  }
}// namespace siege::platform