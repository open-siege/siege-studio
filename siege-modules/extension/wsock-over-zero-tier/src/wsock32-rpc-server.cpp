#include <WinSock2.h>
#include <Ws2ipdef.h>

#include <siege/platform/win/basic_window.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/file.hpp>

import std;
import wsock32.rpc;
import wsock32.shared;

namespace fs = std::filesystem;

constexpr static UINT keep_alive_timer_id = 1;
constexpr static UINT safety_timer_id = 2;
constexpr static ULONGLONG grace_period_ms = 60000;
inline static ULONGLONG last_activity = 0;
inline static bool keep_alive_timer_started = false;

std::optional<backend_imports> backend;
decltype(::WSAGetLastError)* wsock_WSAGetLastError = nullptr;

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

  std::fill(temp.begin(), temp.end(), '\0');
  if (::GlobalGetAtomNameW((ATOM)lparam, temp.data(), (int)temp.size()) == 0)
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

  HANDLE shared_memory = ::OpenFileMappingW(FILE_MAP_WRITE, FALSE, key.data());

  if (!shared_memory)
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    return std::unexpected(SOCKET_ERROR);
  }

  std::shared_ptr<void> deferred = { nullptr, [shared_memory](...) {
                                      ::CloseHandle(shared_memory);
                                    } };

  auto data = ::MapViewOfFile(shared_memory, FILE_MAP_WRITE, 0, 0, 0);

  if (!data)
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    return std::unexpected(SOCKET_ERROR);
  }

  MEMORY_BASIC_INFORMATION info{};
  auto query_result = ::VirtualQuery(data, &info, sizeof(info));

  if (query_result == 0)
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    ::UnmapViewOfFile(data);
    return std::unexpected(SOCKET_ERROR);
  }

  auto new_key = keys.emplace(key);
  auto new_data = already_mapped_data.emplace(*new_key.first, std::span<char>((char*)data, info.RegionSize));

  return new_data.first->second;
}

template<typename TParam>
std::expected<TParam*, LRESULT> get_value(HWND window, LPARAM lparam)
{
  auto result = get_value(window, lparam);

  if (!result)
  {
    ::SetPropW(window, L"LastError", (HANDLE)WSA_INVALID_PARAMETER);
    return std::unexpected(SOCKET_ERROR);
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
    if (message >= WM_APP)
    {
      last_activity = ::GetTickCount64();
      if (!keep_alive_timer_started)
      {
        ::KillTimer(*this, safety_timer_id);
        ::SetTimer(*this, keep_alive_timer_id, 1000, nullptr);
        keep_alive_timer_started = true;
      }
    }


    if (message == WM_TIMER && wparam == safety_timer_id)
    {
      ::DestroyWindow(*this);
      return 0;
    }

    if (message == WM_TIMER && wparam == keep_alive_timer_id)
    {
      if (::GetTickCount64() - last_activity >= grace_period_ms)
      {
        ::DestroyWindow(*this);
      }
      return 0;
    }

    if (message == socket_params::message_id)
    {
      auto value = get_value<socket_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      // backend always creates sockets as non-blocking by default
      auto result = backend->socket(params.address_family, params.type, params.protocol);
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

      auto result = backend->ioctlsocket((SOCKET)wparam, params.command, &params.argument);
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

      auto result = backend->getsockopt((SOCKET)wparam, params.level, params.optname, params.option_data.data(), &params.option_length);
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

      auto result = backend->setsockopt((SOCKET)wparam, params.level, params.optname, params.option_data.data(), params.option_length);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == bind_params::bind_message_id || message == bind_params::connect_message_id)
    {
      auto value = get_value<bind_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      auto* func = message == bind_params::bind_message_id ? backend->bind : backend->connect;

      sockaddr* address = params.address_size == 0 ? nullptr : (sockaddr*)&params.address;
      
      auto result = func((SOCKET)wparam, address, params.address_size);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == listen_params::message_id)
    {
      auto value = get_value<listen_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();
      auto result = backend->listen((SOCKET)wparam, params.backlog);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == accept_params::message_id)
    {
      auto value = get_value<accept_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      sockaddr* address = params.from_address_size == 0 ? nullptr : (sockaddr*)&params.from_address;
      int* address_size = params.from_address_size == 0 ? nullptr : &params.from_address_size;

      auto result = backend->accept((SOCKET)wparam, address, address_size);

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

      auto result = backend->sendto((SOCKET)wparam, data->data(), params.buffer_length, params.flags, address, params.to_address_size);
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

      auto result = backend->recvfrom((SOCKET)wparam, data->data(), params.buffer_length, params.flags, address, address_size);

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

      params.host_name.back() = '\0';

      auto result = backend->gethostbyname(params.host_name.data());
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());

      if (result && result->h_name)
      {
        params.has_result = true;
        auto len = std::min(params.result.host_name.size(), std::strlen(result->h_name));
        std::memcpy(params.result.host_name.data(), result->h_name, len);

        if (result->h_addr_list)
        {
          params.result.address_type = result->h_addrtype;
          params.result.address_size = result->h_length;
          params.result.addresses_length = 0;

          for (auto i = 0u; i < params.result.addresses.size(); ++i)
          {
            if (!result->h_addr_list[i])
            {
              break;
            }

            if (result->h_length == sizeof(in_addr))
            {
              std::memcpy(params.result.addresses[i].data(), result->h_addr_list[i], sizeof(in_addr));
              params.result.addresses_length++;
            }
            else if (result->h_length == sizeof(IN6_ADDR))
            {
              std::memcpy(params.result.addresses[i].data(), result->h_addr_list[i], sizeof(IN6_ADDR));
              params.result.addresses_length++;
            }
          }
        }

        return 1;
      }
      params.has_result = false;
      return 0;
    }


    if (message == select_params::message_id)
    {
      auto value = get_value<select_params>(*this, lparam);

      if (!value)
      {
        return value.error();
      }

      auto& params = *value.value();

      timeval zero{};
      auto result = backend->select(params.fd_set_count, &params.read_set, &params.write_set, &params.except_set, &zero);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == general_params::close_message_id)
    {
      auto result = backend->closesocket((SOCKET)wparam);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == general_params::shutdown_message_id)
    {
      auto result = backend->shutdown((SOCKET)wparam, (int)lparam);
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

      auto result = backend->__WSAFDIsSet((SOCKET)wparam, &params.set_to_check);
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
      auto* func = message == sockname_params::peer_name_message_id ? backend->getpeername : backend->getsockname;

      sockaddr* address = params.address_size == 0 ? nullptr : (sockaddr*)&params.address;
      int* address_size = params.address_size == 0 ? nullptr : &params.address_size;

      auto result = func((SOCKET)wparam, address, address_size);
      ::SetPropW(*this, L"LastError", (HANDLE)wsock_WSAGetLastError());
      return result;
    }

    if (message == WM_DESTROY)
    {
      ::RemovePropW(*this, L"LastError");
      ::RemovePropW(*this, L"IsOnline");
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
      .lpszClassName = L"wsock32-rpc-server",

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
    .lpszName = L"Siege Network Helper",
    .lpszClass = MAKEINTATOM(atom),
  });

  load_local_wsock();

  if (!backend->module)
  {
    return -1;
  }

  WSADATA data{};
  auto result = backend->WSAStartup(MAKEWORD(2, 2), &data);

  if (result != 0)
  {
    return -1;
  }

  ::SetPropW(*window, L"IsOnline", (HANDLE)1);
  ::SetTimer(*window, safety_timer_id, 60000, nullptr);

  MSG msg;

  while (BOOL status = ::GetMessageW(&msg, nullptr, 0, 0) != 0)
  {
    // TODO quit shouldn't quit. it means that we check if we still connections and then
    // gracefully close if there are none.
    if (msg.message == WM_QUIT)
    {
      ::DestroyWindow(*window);
      break;
    }

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

  backend->WSACleanup();
  return (int)msg.wParam;
}

void load_local_wsock()
{
  if (backend)
  {
    return;
  }

  auto module_path = win32::module_ref::current_module().GetModuleFileName();

  std::vector<wchar_t> temp;
  temp.resize(::GetEnvironmentVariableW(L"WSOCK_RPC_BACKEND", nullptr, 0));
  ::GetEnvironmentVariableW(L"WSOCK_RPC_BACKEND", temp.data(), (DWORD)temp.size() + 1);

  fs::path lib_path;

  if (temp.empty())
  {
    lib_path = fs::path(module_path).parent_path() / L"wsock-backend-zero-tier.dll";
  }
  else
  {
    lib_path = temp.data();
  }

  auto wsock_module = ::LoadLibraryExW(lib_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

  if (!wsock_module)
  {
    return;
  }

  HMODULE ws2_32 = nullptr;
  if (!::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, L"ws2_32.dll", &ws2_32))
  {
    return;
  }

  backend = std::make_optional(backend_imports{
    .module = wsock_module,
    .WSAStartup = (decltype(backend_imports::WSAStartup))::GetProcAddress(wsock_module, "backend_WSAStartup"),
    .WSACleanup = (decltype(backend_imports::WSACleanup))::GetProcAddress(wsock_module, "backend_WSACleanup"),
    .socket = (decltype(backend_imports::socket))::GetProcAddress(wsock_module, "backend_socket"),
    .closesocket = (decltype(backend_imports::closesocket))::GetProcAddress(wsock_module, "backend_closesocket"),
    .shutdown = (decltype(backend_imports::shutdown))::GetProcAddress(wsock_module, "backend_shutdown"),
    .setsockopt = (decltype(backend_imports::setsockopt))::GetProcAddress(wsock_module, "backend_setsockopt"),
    .getsockopt = (decltype(backend_imports::getsockopt))::GetProcAddress(wsock_module, "backend_getsockopt"),
    .getsockname = (decltype(backend_imports::getsockname))::GetProcAddress(wsock_module, "backend_getsockname"),
    .getpeername = (decltype(backend_imports::getpeername))::GetProcAddress(wsock_module, "backend_getpeername"),
    .gethostbyname = (decltype(backend_imports::gethostbyname))::GetProcAddress(wsock_module, "backend_gethostbyname"),
    .recvfrom = (decltype(backend_imports::recvfrom))::GetProcAddress(wsock_module, "backend_recvfrom"),
    .sendto = (decltype(backend_imports::sendto))::GetProcAddress(wsock_module, "backend_sendto"),
    .ioctlsocket = (decltype(backend_imports::ioctlsocket))::GetProcAddress(wsock_module, "backend_ioctlsocket"),
    .bind = (decltype(backend_imports::bind))::GetProcAddress(wsock_module, "backend_bind"),
    .connect = (decltype(backend_imports::connect))::GetProcAddress(wsock_module, "backend_connect"),
    .accept = (decltype(backend_imports::accept))::GetProcAddress(wsock_module, "backend_accept"),
    .listen = (decltype(backend_imports::listen))::GetProcAddress(wsock_module, "backend_listen"),
    .select = (decltype(backend_imports::select))::GetProcAddress(wsock_module, "backend_select"),
    .__WSAFDIsSet = (decltype(backend_imports::__WSAFDIsSet))::GetProcAddress(wsock_module, "backend___WSAFDIsSet"),
  });
  wsock_WSAGetLastError = (decltype(wsock_WSAGetLastError))::GetProcAddress(ws2_32, "WSAGetLastError");
}
