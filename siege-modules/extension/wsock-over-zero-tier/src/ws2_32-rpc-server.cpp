#include "ws2_32-rpc.hpp"
#include <siege/platform/win/basic_window.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/module.hpp>
#include <filesystem>
#include <algorithm>
#include <expected>

namespace fs = std::filesystem;

HMODULE wsock_module = nullptr;
decltype(::WSAStartup)* wsock_WSAStartup = nullptr;
decltype(::WSACleanup)* wsock_WSACleanup = nullptr;
decltype(::socket)* wsock_socket = nullptr;
decltype(::setsockopt)* wsock_setsockopt = nullptr;
decltype(::getsockopt)* wsock_getsockopt = nullptr;
decltype(::getsockname)* wsock_getsockname = nullptr;
decltype(::getpeername)* wsock_getpeername = nullptr;
decltype(::gethostbyaddr)* wsock_gethostbyaddr = nullptr;
decltype(::gethostname)* wsock_gethostname = nullptr;
decltype(::gethostbyname)* wsock_gethostbyname = nullptr;
decltype(::recv)* wsock_recv = nullptr;
decltype(::recvfrom)* wsock_recvfrom = nullptr;
decltype(::send)* wsock_send = nullptr;
decltype(::sendto)* wsock_sendto = nullptr;
decltype(::ioctlsocket)* wsock_ioctlsocket = nullptr;
decltype(::bind)* wsock_bind = nullptr;
decltype(::connect)* wsock_connect = nullptr;
decltype(::accept)* wsock_accept = nullptr;
decltype(::listen)* wsock_listen = nullptr;
decltype(::shutdown)* wsock_shutdown = nullptr;
decltype(::select)* wsock_select = nullptr;
decltype(::closesocket)* wsock_closesocket = nullptr;
decltype(::__WSAFDIsSet)* wsock___WSAFDIsSet = nullptr;
decltype(::htonl)* wsock_htonl = nullptr;
decltype(::htons)* wsock_htons = nullptr;
decltype(::ntohl)* wsock_ntohl = nullptr;
decltype(::ntohs)* wsock_ntohs = nullptr;
decltype(::inet_addr)* wsock_inet_addr = nullptr;
decltype(::inet_ntoa)* wsock_inet_ntoa = nullptr;
decltype(::WSASetBlockingHook)* wsock_WSASetBlockingHook = nullptr;
decltype(::WSAUnhookBlockingHook)* wsock_WSAUnhookBlockingHook = nullptr;
decltype(::WSACancelBlockingCall)* wsock_WSACancelBlockingCall = nullptr;
decltype(::WSAGetLastError)* wsock_WSAGetLastError = nullptr;
decltype(::WSASetLastError)* wsock_WSASetLastError = nullptr;
decltype(::WSAAsyncGetHostByName)* wsock_WSAAsyncGetHostByName = nullptr;
decltype(::WSACancelAsyncRequest)* wsock_WSACancelAsyncRequest = nullptr;
decltype(::WSAAsyncSelect)* wsock_WSAAsyncSelect = nullptr;
decltype(::WSAStringToAddressA)* wsock_WSAStringToAddressA = nullptr;
decltype(::WSAGetOverlappedResult)* wsock_WSAGetOverlappedResult = nullptr;
decltype(::WSACreateEvent)* wsock_WSACreateEvent = nullptr;
decltype(::WSAResetEvent)* wsock_WSAResetEvent = nullptr;
decltype(::WSACloseEvent)* wsock_WSACloseEvent = nullptr;
decltype(::WSAWaitForMultipleEvents)* wsock_WSAWaitForMultipleEvents = nullptr;
decltype(::WSASendTo)* wsock_WSASendTo = nullptr;
decltype(::WSASend)* wsock_WSASend = nullptr;
decltype(::WSARecvFrom)* wsock_WSARecvFrom = nullptr;
decltype(::WSAEventSelect)* wsock_WSAEventSelect = nullptr;
decltype(::WSAEnumNetworkEvents)* wsock_WSAEnumNetworkEvents = nullptr;

std::expected<std::span<char>, LRESULT> get_value(HWND window, LPARAM lparam)
{
  static std::array<wchar_t, 256> temp{};

  static std::set<std::wstring> keys;
  static std::map<std::wstring_view, std::span<char>> already_mapped_data;
  static std::shared_ptr<void> deferred_unmap = { nullptr, [](...) {
                                                   for (auto& item : already_mapped_data)
                                                   {
                                                     ::UnmapViewOfFile(item.second.data());
                                                   }
                                                 } };

  if (::GetAtomNameW((ATOM)lparam, temp.data(), (int)temp.size()) == 0)
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    return std::unexpected(SOCKET_ERROR);
  }

  std::wstring_view key = temp.data();

  auto iter = already_mapped_data.find(key);

  if (iter != already_mapped_data.end())
  {
    return iter->second;
  }

  HANDLE shared_memory = ::OpenFileMappingW(FILE_MAP_READ, FALSE, temp.data());

  if (!shared_memory)
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    return std::unexpected(SOCKET_ERROR);
  }

  std::shared_ptr<void> deferred = { nullptr, [shared_memory](...) {
                                      ::CloseHandle(shared_memory);
                                    } };

  auto data = ::MapViewOfFile(shared_memory, FILE_MAP_READ, 0, 0, 0);

  if (!data)
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    return std::unexpected(SOCKET_ERROR);
  }

  MEMORY_BASIC_INFORMATION info{};
  auto size = ::VirtualQuery(shared_memory, &info, sizeof(info));

  if (size == 0)
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    ::UnmapViewOfFile(data);
    return std::unexpected(SOCKET_ERROR);
  }

  auto new_key = keys.emplace(key);
  auto new_data = already_mapped_data.emplace(new_key, std::span<char>((char*)data, size));

  return new_data.first->second;
}

template<typename TParam>
std::expected<TParam*, LRESULT> get_value(HWND window, LPARAM lparam)
{
  auto result = get_value(window, lparam);

  if (!result)
  {
    return result;
  }

  if (result->size() < sizeof(TParam))
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    return std::unexpected(SOCKET_ERROR);
  }

  return (TParam*)result->data();
}

struct wsock_window : win32::basic_window<wsock_window>
{
  std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
  {
    if (message == socket_params::message_id)
    {
      auto value = get_value<socket_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto result = wsock_socket(params.address_family, params.type, params.protocol);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }


    if (message == ioctl_params::message_id)
    {
      auto value = get_value<ioctl_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto result = wsock_ioctlsocket((SOCKET)wparam, params.command, &params.argument);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == sockopt_params::get_message_id)
    {
      auto value = get_value<sockopt_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto result = wsock_getsockopt((SOCKET)wparam, params.level, params.optname, params.option_data.data(), &params.option_length);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == sockopt_params::set_message_id)
    {
      auto value = get_value<sockopt_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto result = wsock_setsockopt((SOCKET)wparam, params.level, params.optname, params.option_data.data(), params.option_length);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == bind_params::message_id)
    {
      auto value = get_value<bind_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto result = wsock_bind((SOCKET)wparam, (sockaddr*)&params.address, params.address_size);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == sendto_params::message_id)
    {
      auto value = get_value<sendto_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto data = get_value(*this, params.buffer);

      if (!data)
      {
        return data.error();
      }

      sockaddr* address = params.to_address_size == 0 ? nullptr : (sockaddr*)&params.to_address;

      auto result = wsock_sendto((SOCKET)wparam, data->data(), params.buffer_length, params.flags, address, params.to_address_size);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == recvfrom_params::message_id)
    {
      auto value = get_value<recvfrom_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto data = get_value(*this, params.buffer);

      if (!data)
      {
        return data.error();
      }
      sockaddr* address = params.from_address_size == 0 ? nullptr : (sockaddr*)&params.from_address;
      int* address_size = params.from_address_size == 0 ? nullptr : &params.from_address_size;

      auto result = wsock_recvfrom((SOCKET)wparam, data->data(), params.buffer_length, params.flags, address, address_size);

      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == select_params::message_id)
    {
      auto value = get_value<select_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto result = wsock_select(params.fd_set_count, &params.read_set, &params.write_set, &params.except_set, &params.timeout);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == isset_params::message_id)
    {
      auto value = get_value<isset_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto result = wsock___WSAFDIsSet((SOCKET)wparam, &params.set_to_check);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == sockname_params::peer_name_message_id || message == sockname_params::sock_name_message_id)
    {
      auto value = get_value<sockname_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();
      auto* func = sockname_params::peer_name_message_id ? wsock_getpeername : wsock_getsockname;

      sockaddr* address = params.address_size == 0 ? nullptr : (sockaddr*)&params.address;
      int* address_size = params.address_size == 0 ? nullptr : &params.address_size;

      auto result = func((SOCKET)wparam, address, address_size);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == hostbyname_params::message_id)
    {
      auto value = get_value<hostbyname_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto result = wsock_gethostbyname(params.host_name.data());

      if (result)
      {
        params.has_result = true;
        params.result.address_type = result->h_addrtype;
        params.result.address_length = result->h_length;
        std::string_view host_name = result->h_name;

        std::size_t size = std::clamp<std::size_t>(host_name.size(), 0, params.host_name.size() - 1);
        std::memcpy(params.host_name.data(), host_name.data(), size);

        for (auto i = 0; i < result->h_length; ++i)
        {
          std::string_view addr = result->h_addr_list[i];

          std::size_t size = std::clamp<std::size_t>(addr.size(), 0, params.result.addresses[i].size() - 1);
          std::memcpy(params.result.addresses[i].data(), addr.data(), size);
        }
      }

      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return params.has_result ? 1 : 0;
    }

    if (message == WM_DESTROY)
    {
      ::PostQuitMessage(0);
    }

    return std::nullopt;
  }

  inline static auto register_class(HINSTANCE module)
  {
    WNDCLASSEXW info{
      .cbSize = sizeof(info),
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = basic_window::window_proc,
      .cbWndExtra = sizeof(void*),
      .hInstance = module,
      .lpszClassName = L"ws2_32-rpc-server",

    };
    return ::RegisterClassExW(&info);
  }

  wsock_window(HWND handle, CREATESTRUCTW& params) : basic_window(handle, params)
  {
  }
};

void load_local_wsock();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
  auto atom = wsock_window::register_class(hInstance);

  if (!atom)
  {
    return -1;
  }

  auto window = win32::CreateWindowExW(CREATESTRUCTW{
    .hwndParent = HWND_MESSAGE,
    .lpszName = L"ws2_32_rpc_server",
    .lpszClass = MAKEINTATOM(atom),
  });

  load_local_wsock();

  if (!wsock_module)
  {
    return -1;
  }

  WSADATA data{};
  auto result = wsock_WSAStartup(MAKEWORD(2, 2), &data);

  if (result != 0)
  {
    return -1;
  }

  MSG msg;

  while (BOOL status = ::GetMessageW(&msg, *window, 0, 0) != 0)
  {
    if (status == -1)
    {
      break;
    }
    else
    {
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
    }
  }
  return (int)msg.wParam;
}

void load_local_wsock()
{
  if (wsock_module)
  {
    return;
  }

  auto module_path = win32::module_ref::current_module().GetModuleFileName();


  wsock_module = LoadLibraryExW(L"ws2_32-on-zero-tier.dll", nullptr, 0);

  if (!wsock_module)
  {
    return;
  }

  wsock_WSAStartup = (decltype(wsock_WSAStartup))::GetProcAddress(wsock_module, "WSAStartup");
  wsock_WSACleanup = (decltype(wsock_WSACleanup))::GetProcAddress(wsock_module, "WSACleanup");
  wsock_socket = (decltype(wsock_socket))::GetProcAddress(wsock_module, "socket");
  wsock_setsockopt = (decltype(wsock_setsockopt))::GetProcAddress(wsock_module, "setsockopt");
  wsock_getsockname = (decltype(wsock_getsockname))::GetProcAddress(wsock_module, "getsockname");
  wsock_getpeername = (decltype(wsock_getpeername))::GetProcAddress(wsock_module, "getpeername");
  wsock_getsockopt = (decltype(wsock_getsockopt))::GetProcAddress(wsock_module, "getsockopt");
  wsock_gethostbyaddr = (decltype(wsock_gethostbyaddr))::GetProcAddress(wsock_module, "gethostbyaddr");
  wsock_gethostname = (decltype(wsock_gethostname))::GetProcAddress(wsock_module, "gethostname");
  wsock_gethostbyname = (decltype(wsock_gethostbyname))::GetProcAddress(wsock_module, "gethostbyname");
  wsock_htons = (decltype(wsock_htons))::GetProcAddress(wsock_module, "htons");
  wsock_htonl = (decltype(wsock_htonl))::GetProcAddress(wsock_module, "htonl");
  wsock_ntohl = (decltype(wsock_ntohl))::GetProcAddress(wsock_module, "ntohl");
  wsock_ntohs = (decltype(wsock_ntohs))::GetProcAddress(wsock_module, "ntohs");
  wsock_inet_addr = (decltype(wsock_inet_addr))::GetProcAddress(wsock_module, "inet_addr");
  wsock_inet_ntoa = (decltype(wsock_inet_ntoa))::GetProcAddress(wsock_module, "inet_ntoa");
  wsock_recv = (decltype(wsock_recv))::GetProcAddress(wsock_module, "recv");
  wsock_recvfrom = (decltype(wsock_recvfrom))::GetProcAddress(wsock_module, "recvfrom");
  wsock_send = (decltype(wsock_send))::GetProcAddress(wsock_module, "send");
  wsock_sendto = (decltype(wsock_sendto))::GetProcAddress(wsock_module, "sendto");
  wsock_ioctlsocket = (decltype(wsock_ioctlsocket))::GetProcAddress(wsock_module, "ioctlsocket");
  wsock_bind = (decltype(wsock_connect))::GetProcAddress(wsock_module, "bind");
  wsock_connect = (decltype(wsock_connect))::GetProcAddress(wsock_module, "connect");
  wsock_accept = (decltype(wsock_accept))::GetProcAddress(wsock_module, "accept");
  wsock_listen = (decltype(wsock_listen))::GetProcAddress(wsock_module, "listen");
  wsock_shutdown = (decltype(wsock_shutdown))::GetProcAddress(wsock_module, "shutdown");
  wsock_select = (decltype(wsock_select))::GetProcAddress(wsock_module, "select");
  wsock_closesocket = (decltype(wsock_closesocket))::GetProcAddress(wsock_module, "closesocket");
  wsock_WSAGetLastError = (decltype(wsock_WSAGetLastError))::GetProcAddress(wsock_module, "WSAGetLastError");
  wsock_WSASetLastError = (decltype(wsock_WSASetLastError))::GetProcAddress(wsock_module, "WSASetLastError");
  wsock___WSAFDIsSet = (decltype(wsock___WSAFDIsSet))::GetProcAddress(wsock_module, "__WSAFDIsSet");
  wsock_WSAAsyncGetHostByName = (decltype(wsock_WSAAsyncGetHostByName))::GetProcAddress(wsock_module, "WSAAsyncGetHostByName");
  wsock_WSACancelAsyncRequest = (decltype(wsock_WSACancelAsyncRequest))::GetProcAddress(wsock_module, "WSACancelAsyncRequest");
  wsock_WSASetBlockingHook = (decltype(wsock_WSASetBlockingHook))::GetProcAddress(wsock_module, "WSASetBlockingHook");
  wsock_WSAUnhookBlockingHook = (decltype(wsock_WSAUnhookBlockingHook))::GetProcAddress(wsock_module, "WSAUnhookBlockingHook");
  wsock_WSACancelBlockingCall = (decltype(wsock_WSACancelBlockingCall))::GetProcAddress(wsock_module, "WSACancelBlockingCall");
  wsock_WSAAsyncSelect = (decltype(wsock_WSAAsyncSelect))::GetProcAddress(wsock_module, "WSAAsyncSelect");
  wsock_WSAStringToAddressA = (decltype(wsock_WSAStringToAddressA))::GetProcAddress(wsock_module, "WSAStringToAddressA");
  wsock_WSAGetOverlappedResult = (decltype(wsock_WSAGetOverlappedResult))::GetProcAddress(wsock_module, "WSAGetOverlappedResult");
  wsock_WSACreateEvent = (decltype(wsock_WSACreateEvent))::GetProcAddress(wsock_module, "WSACreateEvent");
  wsock_WSAResetEvent = (decltype(wsock_WSAResetEvent))::GetProcAddress(wsock_module, "WSAResetEvent");
  wsock_WSACloseEvent = (decltype(wsock_WSACloseEvent))::GetProcAddress(wsock_module, "WSACloseEvent");
  wsock_WSAWaitForMultipleEvents = (decltype(wsock_WSAWaitForMultipleEvents))::GetProcAddress(wsock_module, "WSAWaitForMultipleEvents");
  wsock_WSASendTo = (decltype(wsock_WSASendTo))::GetProcAddress(wsock_module, "WSASendTo");
  wsock_WSASend = (decltype(wsock_WSASend))::GetProcAddress(wsock_module, "WSASend");
  wsock_WSARecvFrom = (decltype(wsock_WSARecvFrom))::GetProcAddress(wsock_module, "WSARecvFrom");
  wsock_WSAEventSelect = (decltype(wsock_WSAEventSelect))::GetProcAddress(wsock_module, "WSAEventSelect");
  wsock_WSAEnumNetworkEvents = (decltype(wsock_WSAEnumNetworkEvents))::GetProcAddress(wsock_module, "WSAEnumNetworkEvents");
}
