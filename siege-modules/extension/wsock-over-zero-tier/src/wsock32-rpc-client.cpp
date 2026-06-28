#include <WinSock2.h>
#include <ws2tcpip.h>

#include <siege/platform/win/module.hpp>
#include <siege/platform/win/process.hpp>
#include <siege/platform/shared.hpp>

import std;
import wsock32.shared;
import wsock32.rpc;

namespace fs = std::filesystem;

void ensure_imports();
std::ostream& get_log();
std::optional<std::uint64_t> get_zero_tier_network_id();
bool use_zero_tier();
std::string ioctl_cmd_to_string(long cmd);
std::string protocol_to_string(int protocol);
std::string type_to_string(int type);
std::string af_to_string(int af);

static struct rpc_process_info : ::PROCESS_INFORMATION
{
  HWND server = nullptr;

  // This is fine if we only ever have wsock and ws2_32 from one process.
  // But fails completely when we have more than two.
  // TODO find a way to handle more than two clients.
  bool owning = true;
} server_info{};

std::optional<wsock_imports> imports{};
std::shared_ptr<void> cleanup = nullptr;

std::shared_ptr<std::pair<const ATOM, std::span<char>>> get_global_memory(std::size_t size, std::optional<ATOM> key = std::nullopt)
{
  static std::map<ATOM, std::span<char>> cache;
  static std::map<ATOM, std::span<char>> used_globals;
  static std::set<HANDLE> mapping_handles;
  static std::shared_ptr<void> deferred = { nullptr, [](...) {
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

template<typename TParam, int MessageId>
int send_message_to_server(SOCKET socket, std::function<TParam*(void*)> init, std::function<void(TParam*)> on_finish = nullptr)
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

  return (int)return_value;
}


extern "C" {
int __stdcall siege_WSAStartup(WORD version, LPWSADATA data)
{
  ensure_imports();
  get_log() << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version) << '\n';
  auto result = imports->WSAStartup(version, data);

  if (auto network_id = get_zero_tier_network_id(); network_id)
  {
    if (result == 0)
    {
      imports->WSACleanup();
    }

    // preallocate some memory
    try
    {
      get_log() << "Preallocating shared memory\n";
      auto temp1 = get_global_memory(1024);
      auto temp2 = get_global_memory(1024);
    }
    catch (...)
    {
      get_log() << "Could not preallocate memory\n";
      return WSASYSNOTREADY;
    }

    HWND server_window = nullptr;

    get_log() << "Finding existing server window\n";
    for (auto i = 0; i < 3; ++i)
    {
      server_window = ::FindWindowExW(HWND_MESSAGE, nullptr, L"wsock32-rpc-server", nullptr);
      if (server_window)
      {
        server_info.owning = false;
        server_info.server = server_window;
        server_info.dwThreadId = ::GetWindowThreadProcessId(server_info.server, &server_info.dwProcessId);
        return result;
      }
      ::Sleep(50);
    }

    get_log() << "No server found. Launching new server\n";

    auto exe_path = fs::path(win32::module_ref::current_module().GetModuleFileName()).parent_path() / L"wsock32-rpc-server.exe";
    auto process_info = win32::CreateProcessW({
      .application_name = exe_path.c_str(),
    });

    if (!process_info)
    {
      get_log() << "Could not launch server\n";
      return WSASYSNOTREADY;
    }

    std::memcpy(&server_info, &process_info, sizeof(process_info));

    cleanup = std::shared_ptr<void>{ nullptr, [](...) {
                                      ::PostThreadMessageW(server_info.dwThreadId, WM_QUIT, 0, 0);
                                      ::WaitForSingleObject(server_info.hProcess, 1000);
                                      ::CloseHandle(server_info.hProcess);
                                      ::CloseHandle(server_info.hThread);
                                    } };

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
      get_log() << "Could not find server window\n";
      cleanup.reset();
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
      cleanup.reset();
      return WSASYSNOTREADY;
    }

    server_info.server = server_window;
  }

  get_log().flush();
  return result;
}

int __stdcall siege_WSACleanup()
{
  ensure_imports();
  get_log() << "siege_WSACleanup" << '\n';

  if (use_zero_tier())
  {
    if (server_info.owning && server_info.server && cleanup)
    {
      cleanup.reset();
    }

    return 0;
  }

  return imports->WSACleanup();
}

SOCKET __stdcall siege_socket(int af, int type, int protocol)
{
  ensure_imports();
  get_log() << "siege_socket af: " << af_to_string(af) << ", type: " << type_to_string(type) << ", protocol: " << protocol_to_string(protocol) << ", thread: " << GetCurrentThreadId() << '\n';

  if (use_zero_tier())
  {
    socket_params params{ .address_family = af, .type = type, .protocol = protocol };

    auto data = get_global_memory(sizeof(params));
    std::memcpy(data->second.data(), &params, sizeof(params));
    DWORD_PTR new_socket = 0;
    auto result = ::SendMessageTimeoutW(server_info.server, socket_params::message_id, 0, (LPARAM)data->first, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &new_socket);
    data.reset();

    if (!result)
    {
      get_log() << "Did not receive a successful result\n";
      imports->WSASetLastError(WSAESOCKTNOSUPPORT);
      return INVALID_SOCKET;
    }

    if ((SOCKET)new_socket == INVALID_SOCKET)
    {
      get_log() << "Received invalid socket\n";
      int last_error = WSAESOCKTNOSUPPORT;
      if (auto server_last_error = (int)::GetPropW(server_info.server, L"LastError"); server_last_error)
      {
        last_error = server_last_error;
      }
      imports->WSASetLastError(last_error);
      return INVALID_SOCKET;
    }

    imports->WSASetLastError(0);
    get_log() << "Returning new socket " << (std::size_t)new_socket << '\n';
    return (SOCKET)new_socket;
  }


  auto result = imports->socket(af, type, protocol);
  get_log() << "Created winsock socket successfully (" << (int)result << ")" << '\n';
  return result;
}

#ifdef USE_WINSOCK2
SOCKET __stdcall siege_WSASocketW(int af, int type, int protocol, LPWSAPROTOCOL_INFOW lpProtocolInfo, GROUP g, DWORD dwFlags)
{
  get_log() << "siege_WSASocketW " << '\n';
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use siege_WSASocketW, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSASocketW(af, type, protocol, lpProtocolInfo, g, dwFlags);
}
#endif

int __stdcall siege_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen)
{
  get_log() << "siege_setsockopt " << '\n';
  if (use_zero_tier())
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

int __stdcall siege_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen)
{
  get_log() << "siege_getsockopt " << '\n';
  if (use_zero_tier())
  {
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

  auto result = imports->getsockopt(ws, level, optname, optval, optlen);

  if (result != 0)
  {
    get_log() << "getsockopt WSAGetLastError " << imports->WSAGetLastError() << '\n';
  }

  return result;
}


int __stdcall siege_bind(SOCKET ws, const sockaddr* addr, int namelen)
{
  get_log() << "siege_bind " << '\n';

  if (use_zero_tier())
  {
    return send_message_to_server<bind_params, bind_params::message_id>(ws, [=](void* raw) {
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
  auto result = imports->bind(ws, addr, namelen);

  get_log() << "Bind call has error " << imports->WSAGetLastError() << "\n";

  return result;
}

int __stdcall siege_ioctlsocket(SOCKET ws, long cmd, u_long* argp)
{
  get_log() << "siege_ioctlsocket, cmd: " << ioctl_cmd_to_string(cmd) << '\n';

  if (use_zero_tier())
  {
    return send_message_to_server<ioctl_params, ioctl_params::message_id>(ws, [=](void* raw) {
      auto* params = new (raw) ioctl_params{ .command = cmd };

      if (argp)
      {
        params->argument = *argp;
      } 
      return params; }, [=](ioctl_params* params) { if (argp)
      {
        *argp = params->argument;
        } });
  }

  auto result = imports->ioctlsocket(ws, cmd, argp);

  get_log() << "siege_ioctlsocket finished" << '\n';

  return result;
}

#ifdef USE_WINSOCK2
int __stdcall siege_WSAIoctl(SOCKET s, DWORD controlCode, LPVOID inBuffer, DWORD inBufferCount, LPVOID outBuffer, DWORD outBufferCount, LPDWORD bytesReturned, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use siege_WSAIoctl, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSAIoctl(s, controlCode, inBuffer, inBufferCount, outBuffer, outBufferCount, bytesReturned, overlapped, completionRoutine);
}
#endif


int __stdcall siege_recv(SOCKET ws, char* buf, int len, int flags)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use siege_recv, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }
  return imports->recv(ws, buf, len, flags);
}

#ifdef USE_WINSOCK2
int __stdcall siege_WSARecv(SOCKET ws, LPWSABUF buffers, DWORD bufferCount, LPDWORD numberOfBytesRecvd, LPDWORD flags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use siege_WSARecv, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSARecv(ws, buffers, bufferCount, numberOfBytesRecvd, flags, lpOverlapped, completionRoutine);
}
#endif

int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen)
{
  if (use_zero_tier())
  {
    // TODO the server is always non-blocking.
    // However, if we want to have blocking sockets we should block on the client side.
    return send_message_to_server<recvfrom_params, recvfrom_params::message_id>(ws, [=](void* raw) {
      auto* params = new (raw) recvfrom_params{ .flags = flags };

      if (buf && len)
      {
        params->buffer_length = len;

        auto temp = get_global_memory(params->buffer_length);
        params->buffer = temp->first;
      }

      if (from && fromLen)
      {
        params->from_address_size = std::clamp<int>(*fromLen, 0, (int)sizeof(params->from_address));
        std::memcpy(&params->from_address, from, params->from_address_size);
      }

      return params; }, [=](recvfrom_params* params) {
      if (buf && len)
      {
        auto data = get_global_memory(params->buffer_length, params->buffer);
        std::memcpy(buf, data->second.data(), params->buffer_length);
      }

      if (from && fromLen)
      {
        auto len = std::clamp<int>(params->from_address_size, 0, *fromLen);
        *fromLen = len;
        std::memcpy(from, &params->from_address, len);
      } });
  }

  return imports->recvfrom(ws, buf, len, flags, from, fromLen);
}

int __stdcall siege_getsockname(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getsockname" << '\n';
  if (use_zero_tier())
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

int __stdcall siege_getpeername(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getpeername" << '\n';
  if (use_zero_tier())
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

  return imports->getpeername(ws, name, length);
}

int __stdcall siege_listen(SOCKET ws, int backlog)
{
  get_log() << "siege_listen\n";
  if (use_zero_tier())
  {
  }
  return imports->listen(ws, backlog);
}

SOCKET __stdcall siege_accept(SOCKET ws, sockaddr* name, int* namelen)
{
  get_log() << "siege_accept\n";
  if (use_zero_tier())
  {
  }

  return imports->accept(ws, name, namelen);
}

int __stdcall siege_connect(SOCKET ws, const sockaddr* name, int namelen)
{
  get_log() << "siege_connect" << '\n';

  if (use_zero_tier())
  {
  }
  return imports->connect(ws, name, namelen);
}

int __stdcall siege_send(SOCKET ws, const char* buf, int len, int flags)
{
  if (use_zero_tier())
  {
  }
  return imports->send(ws, buf, len, flags);
}

int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
  if (use_zero_tier())
  {
    return send_message_to_server<sendto_params, sendto_params::message_id>(ws, [=](void* raw) {
      auto* params = new (raw) sendto_params{ .flags = flags };

      if (buf && len)
      {
        params->buffer_length = len;

        auto temp = get_global_memory(params->buffer_length);
        params->buffer = temp->first;

        std::memcpy(temp->second.data(), buf, len);
      }

      if (to && tolen)
      {
        params->to_address_size = std::clamp<int>(tolen, 0, (int)sizeof(params->to_address));
        std::memcpy(&params->to_address, to, params->to_address_size);
      }

      return params; });
  }

  return imports->sendto(ws, buf, len, flags, to, tolen);
}

int __stdcall siege_shutdown(SOCKET ws, int how)
{
  get_log() << "siege_shutdown\n";
  if (use_zero_tier())
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

int __stdcall siege_closesocket(SOCKET ws)
{
  get_log() << "siege_closesocket\n";
  if (use_zero_tier())
  {
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

    return (int)return_value;
  }
  return imports->closesocket(ws);
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout)
{
  if (use_zero_tier())
  {
    // TODO If the timeout is too long
    // then the client will ignore the response from the server.
    // It's better to update this to make the client do the waiting and
    // send small timeout increments to the server
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

      if (timeout)
      {
        params->timeout = *timeout;
      } 
          return params; }, [=](select_params* params) {
      
      if (read)
      {
        *read = params->read_set;
      }

      if (write)
      {
        *write = params->write_set;
      }

      if (except)
      {
        *except = params->except_set;
      } });
  }
  return imports->select(value, read, write, except, timeout);
}

int __stdcall siege___WSAFDIsSet(SOCKET ws, fd_set* set)
{
  if (use_zero_tier())
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


hostent* __stdcall siege_gethostbyaddr(const char* addr, int len, int type)
{
  ensure_imports();

  get_log() << "siege_gethostbyaddr\n";

  return imports->gethostbyaddr(addr, len, type);
}

hostent* __stdcall siege_gethostbyname(const char* name)
{
  ensure_imports();

  if (use_zero_tier())
  {
    get_log() << "siege_gethostbyname.\n";
    ::MessageBoxW(nullptr, L"The game tried to use siege_gethostbyname, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->gethostbyname(name);
}

auto __stdcall siege_WSAAsyncGetHostByName(HWND window, u_int message, const char* name, char* buffer, int buffer_length)
{
  if (use_zero_tier())
  {
    get_log() << "siege_WSAAsyncGetHostByName.\n";
    ::MessageBoxW(nullptr, L"The game tried to use WSAAsyncGetHostByName, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSAAsyncGetHostByName(window, message, name, buffer, buffer_length);
}

auto __stdcall siege_WSACancelAsyncRequest(HANDLE request)
{
  if (use_zero_tier())
  {
    get_log() << "siege_WSACancelAsyncRequest.\n";
    ::MessageBoxW(nullptr, L"The game tried to use WSACancelAsyncRequest, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSACancelAsyncRequest(request);
}

auto __stdcall siege_WSAAsyncSelect(SOCKET socket, HWND window, u_int message, long flags)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSAAsyncSelect, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);

    ::ExitProcess(-1);
  }

  return imports->WSAAsyncSelect(socket, window, message, flags);
}

#ifdef USE_WINSOCK2
auto __stdcall siege_WSAGetOverlappedResult(SOCKET socket, OVERLAPPED* overlapped, DWORD* transfer, BOOL wait, DWORD* flags)
{
  if (use_zero_tier())
  {
    get_log() << "siege_WSAGetOverlappedResult called. quitting.\n";
    ::ExitProcess(-1);
    // cancel get host by name task
  }
  return imports->WSAGetOverlappedResult(socket, overlapped, transfer, wait, flags);
}

auto __stdcall siege_WSARecvFrom(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD* flags, sockaddr* from, INT* from_len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSARecvFrom, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSARecvFrom(socket, buffers, buffer_count, bytes_received, flags, from, from_len, overlapped, completion_handler);
}

auto __stdcall siege_WSASend(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD flags, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSASend, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSASend(socket, buffers, buffer_count, bytes_received, flags, overlapped, completion_handler);
}

auto __stdcall siege_WSASendTo(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD flags, const sockaddr* to, int len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSASendTo, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }
  return imports->WSASendTo(socket, buffers, buffer_count, bytes_received, flags, to, len, overlapped, completion_handler);
}

auto __stdcall siege_WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSAEventSelect, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSAEventSelect(s, hEventObject, lNetworkEvents);
}

auto __stdcall siege_WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSAEnumNetworkEvents, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }
  return imports->WSAEnumNetworkEvents(s, hEventObject, lpNetworkEvents);
}

auto __stdcall siege_WSACreateEvent()
{
  ensure_imports();
  return imports->WSACreateEvent();
}

auto __stdcall siege_WSAResetEvent(HANDLE event)
{
  return imports->WSAResetEvent(event);
}

auto __stdcall siege_WSACloseEvent(HANDLE event)
{
  return imports->WSACloseEvent(event);
}

auto __stdcall siege_WSAWaitForMultipleEvents(DWORD event_count, const HANDLE* events, BOOL wait_all, DWORD timeout, BOOL alertable)
{
  return imports->WSAWaitForMultipleEvents(event_count, events, wait_all, timeout, alertable);
}
#endif

auto __stdcall siege_gethostname(char* name, int namelen)
{
  ensure_imports();
  return imports->gethostname(name, namelen);
}

auto __stdcall siege_WSAGetLastError()
{
  ensure_imports();
  return imports->WSAGetLastError();
}

auto __stdcall siege_htonl(u_long value)
{
  ensure_imports();
  return imports->htonl(value);
}

auto __stdcall siege_htons(u_short value)
{
  get_log() << "siege_htons: " << value << '\n';
  ensure_imports();
  return imports->htons(value);
}

auto __stdcall siege_ntohl(u_long value)
{
  ensure_imports();
  return imports->ntohl(value);
}

auto __stdcall siege_ntohs(u_short value)
{
  ensure_imports();
  return imports->ntohs(value);
}

unsigned long __stdcall siege_inet_addr(const char* addr)
{
  ensure_imports();
  return imports->inet_addr(addr);
}

auto __stdcall siege_inet_ntoa(in_addr in)
{
  ensure_imports();
  return imports->inet_ntoa(in);
}

#ifdef USE_WINSOCK2
auto __stdcall siege_WSAStringToAddressA(LPSTR address_str, INT family, LPWSAPROTOCOL_INFOA info, LPSOCKADDR out_address, LPINT out_len)
{
  ensure_imports();
  return imports->WSAStringToAddressA(address_str, family, info, out_address, out_len);
}
#endif

auto __stdcall siege_WSASetLastError(int error)
{
  ensure_imports();
  return imports->WSASetLastError(error);
}

auto __stdcall siege_WSASetBlockingHook(FARPROC proc)
{
  ensure_imports();
  get_log() << "siege_WSASetBlockingHook " << '\n';
  return imports->WSASetBlockingHook(proc);
}

auto __stdcall siege_WSAUnhookBlockingHook()
{
  ensure_imports();
  get_log() << "siege_WSAUnhookBlockingHook " << '\n';
  return imports->WSAUnhookBlockingHook();
}

auto __stdcall siege_WSACancelBlockingCall()
{
  ensure_imports();
  get_log() << "siege_WSACancelBlockingCall " << '\n';
  return imports->WSACancelBlockingCall();
}

#ifdef USE_WINSOCK2
// This and freeaddrinfo needed by AMD's open GL driver for the RPC case
auto __stdcall siege_getaddrinfo(const char* node_name, const char* service_name, const addrinfo* hints, addrinfo** results)
{
  ensure_imports();
  get_log() << "siege_getaddrinfo " << '\n';
  return imports->getaddrinfo(node_name, service_name, hints, results);
}

auto __stdcall siege_freeaddrinfo(addrinfo* results)
{
  ensure_imports();
  get_log() << "siege_freeaddrinfo " << '\n';
  return imports->freeaddrinfo(results);
}

auto __stdcall siege_inet_ntop(int family, const void* addr, char* buf, std::size_t buf_size)
{
  ensure_imports();
  get_log() << "siege_inet_ntop " << '\n';
  return imports->inet_ntop(family, addr, buf, buf_size);
}
#endif
}

void ensure_imports()
{
  if (imports)
  {
    return;
  }

  imports = load_system_wsock();

  if (!imports)
  {
    ::ExitProcess(-1);
  }
}

std::ostream& get_log()
{
#ifdef _DEBUG
  static std::ofstream file_log("rpc-client.log", std::ios::trunc);
#else
  static std::stringstream file_log;
  file_log.str("");
#endif
  return file_log;
}

std::optional<std::uint64_t> get_zero_tier_network_id()
{
  static std::optional<std::uint64_t> result = []() -> std::optional<std::uint64_t> {
    try
    {
      get_log() << "get_zero_tier_network_id\n";


      if (auto env_size = ::GetEnvironmentVariableA("ZERO_TIER_NETWORK_ID", nullptr, 0); env_size >= 1)
      {
        std::string network_id(env_size - 1, '\0');
        ::GetEnvironmentVariableA("ZERO_TIER_NETWORK_ID", network_id.data(), network_id.size() + 1);

        get_log() << "Zero Tier Network ID is " << network_id << '\n';
        return std::strtoull(network_id.data(), 0, 16);
      }

      get_log() << "No zero tier network ID\n";
      return std::nullopt;
    }
    catch (...)
    {
      return std::nullopt;
    }
  }();

  return result;
}

bool use_zero_tier()
{
  return get_zero_tier_network_id().has_value();
}

std::string af_to_string(int af)
{
  switch (af)
  {
  case AF_UNSPEC:
    return "AF_UNSPEC";
  case AF_INET:
    return "AF_INET";
  case AF_IPX:
    return "AF_IPX";
  default:
    return std::to_string(af);
  }
}

std::string type_to_string(int type)
{
  switch (type)
  {
  case SOCK_STREAM:
    return "SOCK_STREAM";
  case SOCK_DGRAM:
    return "SOCK_DGRAM";
  case SOCK_RAW:
    return "SOCK_RAW";
  case SOCK_SEQPACKET:
    return "SOCK_SEQPACKET";
  default:
    return std::to_string(type);
  }
}

std::string protocol_to_string(int protocol)
{
  switch (protocol)
  {
  case IPPROTO_IP:
    return "IPPROTO_IP";
  case IPPROTO_TCP:
    return "IPPROTO_TCP";
  case IPPROTO_UDP:
    return "IPPROTO_UDP";
  case IPPROTO_ICMP:
    return "IPPROTO_ICMP";
  case IPPROTO_RAW:
    return "IPPROTO_RAW";
  default: {
    if (protocol >= 1000 && protocol <= 1255)
    {
      return "NSPROTO_IPX";
    }

    if (protocol == 1256)
    {
      return "NSPROTO_SPX";
    }

    return std::to_string(protocol);
  }
  }
}

std::string ioctl_cmd_to_string(long cmd)
{
  switch (cmd)
  {
  case FIONREAD:
    return "FIONREAD";
  case FIONBIO:
    return "FIONBIO";
  case SIOCATMARK:
    return "SIOCATMARK";
  default:
    return std::to_string(cmd);
  }
}