#include <WinSock2.h>
#include <ws2tcpip.h>

#include <siege/platform/win/module.hpp>
#include <siege/platform/win/process.hpp>
#include <siege/platform/shared.hpp>

import std;
import wsock32.shared.client;
import wsock32.shared;
import wsock32.rpc;

namespace fs = std::filesystem;

bool use_custom_backend();

static struct rpc_process_info : ::PROCESS_INFORMATION
{
  HWND server = nullptr;
} server_info{};

std::shared_ptr<std::pair<const ATOM, std::span<char>>> get_global_memory(std::size_t size, std::optional<ATOM> key = std::nullopt)
{
  static std::mutex cache_mutex;
  static std::map<ATOM, std::span<char>> cache;
  static std::map<ATOM, std::span<char>> used_globals;
  static std::set<HANDLE> mapping_handles;
  static std::shared_ptr<void> deferred = { nullptr, [](...) {
                                             std::unique_lock lock(cache_mutex);
                                             for (auto& item : cache)
                                             {
                                               ::GlobalDeleteAtom(item.first);
                                               ::UnmapViewOfFile(item.second.data());
                                             }
                                             cache.clear();

                                             for (auto& item : used_globals)
                                             {
                                               ::GlobalDeleteAtom(item.first);
                                               ::UnmapViewOfFile(item.second.data());
                                             }
                                             used_globals.clear();

                                             for (auto handle : mapping_handles)
                                             {
                                               ::CloseHandle(handle);
                                             }
                                           } };

  std::unique_lock lock(cache_mutex);
  auto iter = cache.end();

  if (key)
  {
    iter = cache.find(*key);
  }

  if (iter == cache.end())
  {
    iter = std::find_if(cache.begin(), cache.end(), [&](auto& item) {
      return item.second.size() > size;
    });
  }

  if (iter == cache.end())
  {
    static auto dll_stem = [] {
      auto stem = fs::path(win32::module_ref::current_module().GetModuleFileName()).stem();
      return siege::platform::to_lower(stem.wstring());
    }();

    // TODO once more than one client are supported,
    // there will need to be better tracking of the
    // individual clients that are loaded per process.
    auto new_name = dll_stem + L"_rpc_data" + std::to_wstring(cache.size() + used_globals.size());

    auto out_handle = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, new_name.c_str());

    if (!out_handle)
    {
      throw std::bad_alloc();
    }

    auto view = ::MapViewOfFile(out_handle, FILE_MAP_WRITE, 0, 0, 0);

    if (!view)
    {
      ::CloseHandle(out_handle);
      throw std::bad_alloc();
    }
    mapping_handles.emplace(out_handle);
    MEMORY_BASIC_INFORMATION info{};
    auto query_result = ::VirtualQuery(view, &info, sizeof(info));
    auto atom = ::GlobalAddAtomW(new_name.c_str());
    iter = cache.emplace(atom, std::span<char>((char*)view, info.RegionSize)).first;
  }

  auto added = used_globals.emplace(*iter);
  cache.erase(iter);

  return std::shared_ptr<std::pair<const ATOM, std::span<char>>>(&*added.first, [](std::pair<const ATOM, std::span<char>>* global) {
    std::unique_lock lock(cache_mutex);
    auto existing = std::find_if(used_globals.begin(), used_globals.end(), [&](auto& item) {
      return item.second.data() == global->second.data();
    });

    if (existing != used_globals.end())
    {
      cache.emplace(*existing);
      used_globals.erase(existing);
    }
  });
}

template<typename TParam, int MessageId, typename TReturn = int>
TReturn send_message_to_server(SOCKET socket, std::function<TParam*(void*)> init, std::function<void(TParam*)> on_finish = nullptr)
{
  auto data = get_global_memory(sizeof(TParam));
  auto* params = init(data->second.data());
  auto atom = data->first;
  DWORD_PTR return_value = 0;
  auto result = ::SendMessageTimeoutW(server_info.server, MessageId, (WPARAM)socket, (LPARAM)data->first, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &return_value);
  data.reset();

  if (!result)
  {
    imports->WSASetLastError(WSAESOCKTNOSUPPORT);
    return SOCKET_ERROR;
  }

  if (return_value == SOCKET_ERROR)
  {
    int last_error = WSAESOCKTNOSUPPORT;
    if (auto server_last_error = (int)::GetPropW(server_info.server, L"LastError"); server_last_error)
    {
      last_error = server_last_error;
    }
    imports->WSASetLastError(last_error);
    return SOCKET_ERROR;
  }

  imports->WSASetLastError(0);

  if (on_finish)
  {
    auto data = get_global_memory(sizeof(TParam), atom);
    auto* params = (TParam*)data->second.data();
    on_finish(params);
  }

  return static_cast<TReturn>(return_value);
}


extern "C" {
int __stdcall siege_WSAStartup(WORD version, LPWSADATA data)
{
  ensure_imports();
  get_log("rpc-client") << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version);
  auto result = imports->WSAStartup(version, data);

  auto env_size = ::GetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", nullptr, 0);
  if (env_size <= 1)
  {
    return result;
  }

  if (result == 0)
  {
    imports->WSACleanup();
  }

  if (server_info.server)
  {
    return result;
  }

  // preallocate some memory
  try
  {
    get_log() << "Preallocating shared memory";
    auto temp1 = get_global_memory(1024);
    auto temp2 = get_global_memory(1024);
  }
  catch (...)
  {
    get_log() << "Could not preallocate memory";
    return WSASYSNOTREADY;
  }

  HWND server_window = nullptr;

  get_log() << "Finding existing server window";
  for (auto i = 0; i < 3; ++i)
  {
    server_window = ::FindWindowExW(HWND_MESSAGE, nullptr, L"wsock32-rpc-server", nullptr);
    if (server_window)
    {
      server_info.server = server_window;
      server_info.dwThreadId = ::GetWindowThreadProcessId(server_info.server, &server_info.dwProcessId);
      return result;
    }
    ::Sleep(50);
  }

  get_log() << "No server found. Launching new server";

  auto exe_path = fs::path(win32::module_ref::current_module().GetModuleFileName()).parent_path() / L"wsock32-rpc-server.exe";
  auto process_info = win32::CreateProcessW({
    .application_name = exe_path.c_str(),
  });

  if (!process_info)
  {
    get_log() << "Could not launch server";
    return WSASYSNOTREADY;
  }

  std::memcpy(&server_info, &process_info, sizeof(process_info));

  if (server_info.hProcess)
  {
    ::CloseHandle(server_info.hProcess);
    server_info.hProcess = nullptr;
  }

  if (server_info.hThread)
  {
    ::CloseHandle(server_info.hThread);
    server_info.hThread = nullptr;
  }

  for (auto i = 0; i < 3; ++i)
  {
    server_window = ::FindWindowExW(HWND_MESSAGE, nullptr, L"wsock32-rpc-server", nullptr);
    if (server_window)
    {
      break;
    }
    ::Sleep(50);
  }


  if (!server_window)
  {
    get_log() << "Could not find server window";
    return WSASYSNOTREADY;
  }

  bool is_init = false;
  for (auto i = 0; i < 500; ++i)
  {
    is_init = ::GetPropW(server_window, L"IsOnline") == (HANDLE)1;

    if (is_init)
    {
      break;
    }
    ::Sleep(100);
  }

  if (!is_init)
  {
    return WSASYSNOTREADY;
  }

  server_info.server = server_window;

  get_log().flush();
  return result;
}

int __stdcall siege_WSACleanup()
{
  ensure_imports();
  get_log() << "siege_WSACleanup";

  if (use_custom_backend())
  {
    return 0;
  }

  return imports->WSACleanup();
}

SOCKET __stdcall siege_socket(int af, int type, int protocol) noexcept
{
  ensure_imports();

  if (!use_custom_backend())
  {
    return imports->socket(af, type, protocol);
  }

  get_log() << "siege_socket af: " << af_to_string(af) << ", type: " << type_to_string(type) << ", protocol: " << protocol_to_string(protocol) << ", thread: " << GetCurrentThreadId();

  socket_params params{ .address_family = af, .type = type, .protocol = protocol };

  auto data = get_global_memory(sizeof(params));
  std::memcpy(data->second.data(), &params, sizeof(params));
  DWORD_PTR new_socket = 0;
  auto result = ::SendMessageTimeoutW(server_info.server, socket_params::message_id, 0, (LPARAM)data->first, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &new_socket);
  data.reset();

  if (!result)
  {
    get_log() << "Did not receive a successful result";
    imports->WSASetLastError(WSAESOCKTNOSUPPORT);
    return INVALID_SOCKET;
  }

  if ((SOCKET)new_socket == INVALID_SOCKET)
  {
    get_log() << "Received invalid socket";
    int last_error = WSAESOCKTNOSUPPORT;
    if (auto server_last_error = (int)::GetPropW(server_info.server, L"LastError"); server_last_error)
    {
      last_error = server_last_error;
    }
    imports->WSASetLastError(last_error);
    return INVALID_SOCKET;
  }


  get_socket_handles().insert(new_socket, type, socket_handle_info::overlapped_state::overlapped);
  imports->WSASetLastError(0);
  get_log() << "Returning new socket " << (std::size_t)new_socket;
  return (SOCKET)new_socket;
}

int __stdcall siege_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen)
{
  get_log() << "siege_setsockopt ";
  if (use_custom_backend())
  {
    return send_message_to_server<sockopt_params, sockopt_params::set_message_id>(ws, [=](void* raw) {
      auto* params = new (raw) sockopt_params{ .level = level, .optname = optname };

      if (optval && optlen)
      {
        auto len = std::clamp<int>(optlen, 0, params->option_data.size());
        std::memcpy(params->option_data.data(), optval, len);
        params->option_length = len;
      }

      return params;
    });
  }

  return imports->setsockopt(ws, level, optname, optval, optlen);
}

int __stdcall siege_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen) noexcept
{
  get_log() << "siege_getsockopt ";


  if (!use_custom_backend())
  {
    auto result = imports->getsockopt(ws, level, optname, optval, optlen);

    if (result != 0)
    {
      get_log() << "getsockopt WSAGetLastError " << imports->WSAGetLastError();
    }

    return result;
  }

  if (level == SOL_SOCKET && optname == SO_MAX_MSG_SIZE && optval && optlen && *optlen)
  {
    DWORD sock_type{};
    int sock_size = sizeof(sock_type);

    auto inner_result = siege_getsockopt(ws, level, SO_TYPE, reinterpret_cast<char*>(&sock_type), &sock_size);

    if (inner_result == SOCKET_ERROR)
    {
      return inner_result;
    }

    int value = 0;

    if (sock_type == SOCK_DGRAM)
    {
      value = 65507;
    }

    auto new_size = std::clamp(*optlen, 0, (int)sizeof(value));

    std::memcpy(optval, &value, new_size);
    *optlen = new_size;
    return 0;
  }

  if (level == SOL_SOCKET && optname == SO_PROTOCOL_INFOA && optval && optlen && *optlen)
  {
    DWORD sock_type{};
    int sock_size = sizeof(sock_type);

    auto inner_result = siege_getsockopt(ws, level, SO_TYPE, reinterpret_cast<char*>(&sock_type), &sock_size);

    if (inner_result == SOCKET_ERROR)
    {
      return inner_result;
    }

    WSAPROTOCOL_INFOA info{};

    info.iVersion = 2;
    info.iSocketType = sock_type;
    info.iAddressFamily = AF_INET;
    info.iMinSockAddr = sizeof(sockaddr_in);
    info.iMaxSockAddr = sizeof(sockaddr_in);
    info.ProtocolChain.ChainLen = 1;

    if (sock_type == SOCK_DGRAM)
    {
      info.dwServiceFlags1 = XP1_CONNECTIONLESS | XP1_MESSAGE_ORIENTED | XP1_SUPPORT_BROADCAST;
      info.iProtocol = IPPROTO_UDP;
    }
    else if (sock_type == SOCK_STREAM)
    {
      info.dwServiceFlags1 = XP1_GUARANTEED_DELIVERY | XP1_GUARANTEED_ORDER | XP1_GRACEFUL_CLOSE;
      info.iProtocol = IPPROTO_TCP;
    }

    auto new_size = std::clamp(*optlen, 0, (int)sizeof(info));

    std::memcpy(optval, &info, new_size);
    *optlen = new_size;
    return 0;
  }


  return send_message_to_server<sockopt_params, sockopt_params::get_message_id>(ws, [=](void* raw) {
      auto* params = new (raw) sockopt_params{ .level = level, .optname = optname };

      if (optval && optlen)
      {
        auto len = std::clamp<int>(*optlen, 0, params->option_data.size());
        params->option_length = len;
      }
      return params; }, [=](sockopt_params* params) {

          if (optval && optlen)
          {
            auto len = std::clamp<int>(params->option_length, 0, *optlen);
            *optlen = len;
            std::memcpy(optval, params->option_data.data(), len);
          } });
}


int __stdcall siege_bind(SOCKET ws, const sockaddr* addr, int namelen)
{
  if (!use_custom_backend())
  {
    return imports->bind(ws, addr, namelen);
  }

  get_log() << "siege_bind ";

  return send_message_to_server<bind_params, bind_params::bind_message_id>(ws, [=](void* raw) {
    auto* params = new (raw) bind_params{};

    if (addr && namelen)
    {
      auto len = std::clamp<int>(namelen, 0, sizeof(params->address));
      params->address_size = len;
      std::memcpy(&params->address, addr, len);
    }
    return params;
  });
}

int __stdcall siege_ioctlsocket(SOCKET ws, long cmd, u_long* argp) noexcept
{
  if (!use_custom_backend())
  {
    return imports->ioctlsocket(ws, cmd, argp);
  }

  if (cmd == FIONREAD)
  {
    log_sampled_check() << "siege_ioctlsocket with FIONREAD";
  }
  else
  {
    get_log() << "siege_ioctlsocket with " << ioctl_cmd_to_string(cmd);
  }

  auto result = send_message_to_server<ioctl_params, ioctl_params::message_id>(ws, [=](void* raw) {
      auto* params = new (raw) ioctl_params{ .command = cmd };

      if (argp)
      {
        params->argument = *argp;
      } 
      return params; }, [=](ioctl_params* params) { if (argp)
      {
        *argp = params->argument;
        } });

  if (result == 0 && cmd == FIONBIO && argp)
  {
    get_socket_handles().set_virtual_blocking(ws, *argp == 0);
  }

  return result;
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout) noexcept;

int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept
{
  if (!use_custom_backend())
  {
    return imports->recvfrom(ws, buf, len, flags, from, fromLen);
  }

  if (from)
  {
    if (flags & MSG_PEEK)
    {
      log_sampled_check() << "siege_recvfrom MSG_PEEK from address " << af_to_string(from->sa_family);
    }
    else
    {
      log_sampled_read() << "siege_recvfrom with from address " << af_to_string(from->sa_family);
    }
  }
  else
  {
    if (flags & MSG_PEEK)
    {
      log_sampled_check() << "siege_recvfrom MSG_PEEK without address";
    }
    else
    {
      log_sampled_read() << "siege_recvfrom with without address";
    }
  }

  // TODO add SEH here for bad buffers
  auto do_recvfrom = [&]() {
    std::shared_ptr<std::pair<const ATOM, std::span<char>>> buffer_memory;

    return send_message_to_server<recvfrom_params, recvfrom_params::message_id>(ws, [&](void* raw) {
      auto* params = new (raw) recvfrom_params{ .flags = flags };

      if (buf && len)
      {
        params->buffer_length = len;

        buffer_memory = get_global_memory(params->buffer_length);
        params->buffer = buffer_memory->first;
      }

      if (from && fromLen)
      {
        params->from_address_size = std::clamp<int>(*fromLen, 0, (int)sizeof(params->from_address));
        std::memcpy(&params->from_address, from, params->from_address_size);
      }

      return params; }, [&](recvfrom_params* params) {
      if (buf && len && buffer_memory)
      {
        std::memcpy(buf, buffer_memory->second.data(), params->buffer_length);
      }

      if (from && fromLen)
      {
        auto len = std::clamp<int>(params->from_address_size, 0, *fromLen);
        *fromLen = len;
        std::memcpy(from, &params->from_address, len);
      } });
  };

try_again:
  auto result = do_recvfrom();
  auto last_error = imports->WSAGetLastError();
  if (get_socket_handles().is_virtual_blocking(ws) && result == SOCKET_ERROR && last_error == WSAEWOULDBLOCK)
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(ws, &read_set);

    auto wait_time = [ws]() -> std::optional<timeval> {
      DWORD timeout = 0;
      int param_size = sizeof(timeout);
      auto result = siege_getsockopt(ws, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), &param_size);

      if (result == SOCKET_ERROR || timeout == 0)
      {
        return std::nullopt;
      }
      return ms_to_timeval(std::chrono::milliseconds{ timeout });
    }();

    result = siege_select(1, &read_set, nullptr, nullptr, wait_time ? &*wait_time : nullptr);

    if (result == SOCKET_ERROR)
    {
      return result;
    }
    goto try_again;
  }
  else if (!get_socket_handles().is_virtual_blocking(ws))
  {
    get_socket_handles().clear_io_flags(ws, FD_READ | FD_OOB);
  }

  return result;
}

int __stdcall siege_getsockname(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getsockname";
  if (use_custom_backend())
  {
    return send_message_to_server<sockname_params, sockname_params::sock_name_message_id>(ws, [=](void* raw) {
      auto* params = new (raw) sockname_params{};

      if (name && length)
      {
        params->address_size = std::clamp<int>(*length, 0, sizeof(params->address));
      }

      return params; }, [=](sockname_params* params) {
          
      if (name && length)
      {
        auto len = std::clamp<int>(params->address_size, 0, *length);
        *length = len;
        std::memcpy(name, &params->address, len);
      } });
  }

  return imports->getsockname(ws, name, length);
}

int __stdcall siege_getpeername(SOCKET ws, sockaddr* name, int* length) noexcept
{
  get_log() << "siege_getpeername";
  if (use_custom_backend())
  {
    return send_message_to_server<sockname_params, sockname_params::peer_name_message_id>(ws, [=](void* raw) {
      auto* params = new (raw) sockname_params{};

      if (name && length)
      {
        params->address_size = std::clamp<int>(*length, 0, sizeof(params->address));
      }

      return params; }, [=](sockname_params* params) {
          
      if (name && length)
      {
        auto len = std::clamp<int>(params->address_size, 0, *length);
        *length = len;
        std::memcpy(name, &params->address, len);
      } });
  }

  return imports->getpeername(ws, name, length);
}

int __stdcall siege_listen(SOCKET ws, int backlog)
{
  if (!use_custom_backend())
  {
    return imports->listen(ws, backlog);
  }

  get_log() << "siege_listen with backlog " << backlog;

  auto result = send_message_to_server<listen_params, listen_params::message_id>(ws, [=](void* raw) {
    return new (raw) listen_params{ .backlog = backlog };
  });

  if (result != SOCKET_ERROR)
  {
    get_socket_handles().set_listening(ws, true);
  }

  return result;
}

SOCKET __stdcall siege_accept(SOCKET ws, sockaddr* from, int* fromLen) noexcept
{
  if (!use_custom_backend())
  {
    return imports->accept(ws, from, fromLen);
  }

  get_log() << "siege_accept";

  auto do_accept = [&]() {
    return send_message_to_server<accept_params, accept_params::message_id, SOCKET>(ws, [=](void* raw) {
      auto* params = new (raw) accept_params{ };

      if (from && fromLen)
      {
        params->from_address_size = std::clamp<int>(*fromLen, 0, (int)sizeof(params->from_address));
        std::memcpy(&params->from_address, from, params->from_address_size);
      }

      return params; }, [=](accept_params* params) {
      if (from && fromLen)
      {
        auto len = std::clamp<int>(params->from_address_size, 0, *fromLen);
        *fromLen = len;
        std::memcpy(from, &params->from_address, len);
      } });
  };

try_again:
  auto result = do_accept();
  auto last_error = imports->WSAGetLastError();

  if (get_socket_handles().is_virtual_blocking(ws) && result == INVALID_SOCKET && last_error == WSAEWOULDBLOCK)
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(ws, &read_set);

    auto select_result = siege_select(1, &read_set, nullptr, nullptr, nullptr);

    if (select_result == SOCKET_ERROR)
    {
      return INVALID_SOCKET;
    }
    goto try_again;
  }
  else if (!get_socket_handles().is_virtual_blocking(ws) && last_error != WSATRY_AGAIN)
  {
    get_socket_handles().clear_io_flags(ws, FD_ACCEPT);
  }

  if (result != INVALID_SOCKET)
  {
    auto parent_overlapped_state = get_socket_handles().is_overlapped(ws);

    get_socket_handles().insert(result, SOCK_STREAM, parent_overlapped_state, socket_handle_info::client_socket_state::accepted);
  }

  return result;
}

int __stdcall siege_connect(SOCKET ws, const sockaddr* name, int namelen)
{
  if (!use_custom_backend())
  {
    return imports->connect(ws, name, namelen);
  }

  get_log() << "siege_connect";

  get_socket_handles().set_client_socket_state(ws, socket_handle_info::client_socket_state::connecting);

  auto do_connect = [&]() {
    return send_message_to_server<bind_params, bind_params::connect_message_id>(ws, [=](void* raw) {
      auto* params = new (raw) bind_params{};

      if (name && namelen)
      {
        auto len = std::clamp<int>(namelen, 0, sizeof(params->address));
        params->address_size = len;
        std::memcpy(&params->address, name, len);
      }
      return params;
    });
  };

  auto result = do_connect();
  auto last_error = imports->WSAGetLastError();
  auto is_pending = last_error == WSAEWOULDBLOCK || last_error == WSAEINPROGRESS || last_error == WSAEALREADY;

  if (get_socket_handles().is_virtual_blocking(ws) && result == SOCKET_ERROR && is_pending)
  {
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(ws, &write_set);

    auto wait_time = [ws]() -> std::optional<timeval> {
      DWORD timeout = 0;
      int param_size = sizeof(timeout);
      auto result = siege_getsockopt(ws, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), &param_size);

      if (result == SOCKET_ERROR || timeout == 0)
      {
        return std::nullopt;
      }
      return ms_to_timeval(std::chrono::milliseconds{ timeout });
    }();

    result = siege_select(1, nullptr, &write_set, nullptr, wait_time ? &*wait_time : nullptr);

    if (result == SOCKET_ERROR)
    {
      return result;
    }

    int socket_error = 0;
    int socket_size = sizeof(socket_error);
    result = siege_getsockopt(ws, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socket_error), &socket_size);

    if (result != 0)
    {
      return result;
    }

    if (socket_error != 0)
    {
      get_socket_handles().set_client_socket_state(ws, socket_handle_info::client_socket_state::unconnected);
      imports->WSASetLastError(socket_error);
      return SOCKET_ERROR;
    }

    get_socket_handles().set_client_socket_state(ws, socket_handle_info::client_socket_state::connected);
    return 0;
  }

  if (result == 0)
  {
    get_socket_handles().set_client_socket_state(ws, socket_handle_info::client_socket_state::connected);
  }
  else if (!is_pending && result == SOCKET_ERROR)
  {
    get_socket_handles().set_client_socket_state(ws, socket_handle_info::client_socket_state::unconnected);
  }

  return result;
}

int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept
{
  if (!use_custom_backend())
  {
    return imports->sendto(ws, buf, len, flags, to, tolen);
  }

  // TODO add SEH here for bad buffers
  if (to)
  {
    log_sampled_write() << "siege_sendto with to address " << af_to_string(to->sa_family);
  }
  else
  {
    log_sampled_write() << "siege_sendto with no address";
  }

  auto do_sendto = [&]() {
    std::shared_ptr<std::pair<const ATOM, std::span<char>>> buffer_memory;

    return send_message_to_server<sendto_params, sendto_params::message_id>(ws, [&](void* raw) {
      auto* params = new (raw) sendto_params{ .flags = flags };

      if (buf && len)
      {
        params->buffer_length = len;

        buffer_memory = get_global_memory(params->buffer_length);
        params->buffer = buffer_memory->first;

        std::memcpy(buffer_memory->second.data(), buf, len);
      }

      if (to && tolen)
      {
        params->to_address_size = std::clamp<int>(tolen, 0, (int)sizeof(params->to_address));
        std::memcpy(&params->to_address, to, params->to_address_size);
      }

      return params; });
  };

try_again:
  auto result = do_sendto();
  auto last_error = imports->WSAGetLastError();

  if (get_socket_handles().is_virtual_blocking(ws) && result == SOCKET_ERROR && last_error == WSAEWOULDBLOCK)
  {
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(ws, &write_set);

    auto wait_time = [ws]() -> std::optional<timeval> {
      DWORD timeout = 0;
      int param_size = sizeof(timeout);
      auto result = siege_getsockopt(ws, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), &param_size);

      if (result == SOCKET_ERROR || timeout == 0)
      {
        return std::nullopt;
      }
      return ms_to_timeval(std::chrono::milliseconds{ timeout });
    }();

    result = siege_select(1, nullptr, &write_set, nullptr, wait_time ? &*wait_time : nullptr);

    if (result == SOCKET_ERROR)
    {
      return result;
    }
    goto try_again;
  }
  else if (!get_socket_handles().is_virtual_blocking(ws) && result == SOCKET_ERROR && last_error == WSAEWOULDBLOCK)
  {
    get_socket_handles().clear_io_flags(ws, FD_WRITE);
  }

  return result;
}

int __stdcall siege_shutdown(SOCKET ws, int how)
{
  get_log() << "siege_shutdown";
  if (use_custom_backend())
  {
    DWORD_PTR return_value{};
    auto result = ::SendMessageTimeoutW(server_info.server, general_params::shutdown_message_id, (WPARAM)ws, (LPARAM)how, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &return_value);

    if (!result)
    {
      imports->WSASetLastError(WSAESOCKTNOSUPPORT);
      return SOCKET_ERROR;
    }

    if (return_value == SOCKET_ERROR)
    {
      int last_error = WSAESOCKTNOSUPPORT;
      if (auto server_last_error = (int)::GetPropW(server_info.server, L"LastError"); server_last_error)
      {
        last_error = server_last_error;
      }
      imports->WSASetLastError(last_error);
      return SOCKET_ERROR;
    }

    return (int)return_value;
  }
  return imports->shutdown(ws, how);
}

// TODO needs to cancel pending overlapped operations
int __stdcall siege_closesocket(SOCKET ws)
{
  get_log() << "siege_closesocket";

  if (!use_custom_backend())
  {
    return imports->closesocket(ws);
  }

  DWORD_PTR return_value{};
  auto result = ::SendMessageTimeoutW(server_info.server, general_params::close_message_id, (WPARAM)ws, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &return_value);

  if (!result)
  {
    imports->WSASetLastError(WSAESOCKTNOSUPPORT);
    return SOCKET_ERROR;
  }

  if (return_value == SOCKET_ERROR)
  {
    int last_error = WSAESOCKTNOSUPPORT;
    if (auto server_last_error = (int)::GetPropW(server_info.server, L"LastError"); server_last_error)
    {
      last_error = server_last_error;
    }
    imports->WSASetLastError(last_error);
    return SOCKET_ERROR;
  }

  get_socket_handles().close(ws);
  return (int)return_value;
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout) noexcept
{
  if (!use_custom_backend())
  {
    return imports->select(value, read, write, except, timeout);
  }

  if (timeout)
  {
    log_sampled_check() << "siege_select with timeout, sec: " << timeout->tv_sec << ", usec: " << timeout->tv_usec;
  }
  else
  {
    log_sampled_check() << "siege_select with no timeout";
  }

  auto max_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours{ 24 });

  auto time_to_wait = max_time;

  if (timeout)
  {
    time_to_wait = std::min(timeval_to_ms(*timeout), max_time);
  }
  auto end = std::chrono::steady_clock::now() + time_to_wait;

  fd_set staging_read{};
  fd_set staging_write{};
  fd_set staging_except{};

  auto do_select = [&] {
    return send_message_to_server<select_params, select_params::message_id>(0, [=](void* raw) {
      select_params* params = new (raw) select_params{};

      params->fd_set_count = value;

      if (read)
      {
        params->read_set = *read;
      }

      if (write)
      {
        params->write_set = *write;
      }

      if (except)
      {
        params->except_set = *except;
      }
      return params; }, [&](select_params* params) {
      staging_read = params->read_set;
      staging_write = params->write_set;
      staging_except = params->except_set; });
  };

  do
  {
    auto result = do_select();

    if (result > 0)
    {
      if (read)
      {
        *read = staging_read;
      }

      if (write)
      {
        *write = staging_write;
      }

      if (except)
      {
        *except = staging_except;
      }
      return result;
    }

    if (result == SOCKET_ERROR)
    {
      return result;
    }

    if (std::chrono::steady_clock::now() >= end)
    {
      break;
    }

    auto remaining = end - std::chrono::steady_clock::now();

    auto sleep_time = std::min(
      std::chrono::duration_cast<std::chrono::milliseconds>(remaining),
      std::chrono::milliseconds{ 5 });

    std::this_thread::sleep_for(sleep_time);

  } while (std::chrono::steady_clock::now() < end);

  return 0;
}

int __stdcall siege___WSAFDIsSet(SOCKET ws, fd_set* set)
{
  if (use_custom_backend())
  {
    return send_message_to_server<isset_params, isset_params::message_id>(ws, [=](void* raw) {
      auto* params = new (raw) isset_params{};
      if (set)
      {
        params->set_to_check = *set;
      }

      return params;
    });
  }

  return imports->__WSAFDIsSet(ws, set);
}

// TODO just needs to be exposed in the backend and hooked up here
hostent* __stdcall siege_gethostbyname(const char* name) noexcept
{
  ensure_imports();

  if (!use_custom_backend())
  {
    return imports->gethostbyname(name);
  }

  get_log() << "siege_gethostbyname.";

  thread_local hostent result{};
  thread_local hostbyname_params::hostinfo storage{};
  thread_local std::vector<char*> addresses;

  auto has_result = send_message_to_server<hostbyname_params, hostbyname_params::message_id>(INVALID_SOCKET, [=](void* raw) {
      auto* params = new (raw) hostbyname_params{};

      if (name)
      {
        auto size = std::min(params->host_name.size(), std::strlen(name));
        std::memcpy(params->host_name.data(), name, size);
      }
      return params; }, [=](hostbyname_params* params) {
      if (!params->has_result)
      {
        return;
      }

      storage = params->result;

      storage.host_name.back() = '\0';

      result.h_name = storage.host_name.data();
      result.h_aliases = nullptr;
      result.h_addrtype = storage.address_type;
      result.h_length = storage.address_size;
      
      addresses.reserve(storage.addresses_length);

      for (auto i = 0; i < storage.addresses_length; i++)
      {
        addresses.emplace_back(storage.addresses[i].data());
      }
      addresses.emplace_back(nullptr);

      result.h_addr_list = addresses.data(); });

  if (!has_result)
  {
    imports->WSASetLastError(WSAHOST_NOT_FOUND);
    return nullptr;
  }

  return &result;
}
}

bool use_custom_backend()
{
  return server_info.server != nullptr;
}