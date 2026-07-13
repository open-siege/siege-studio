// TODO Separate the zero tier wrapper part into its own backend dll.
// The API used will be the same as future wrappers.
// The main properties of a backend DLL:
// * Implements the minimal required API set and not everything possible - though still in terms of ws2_32.
// * Sockets are created non-blocking and broadcast capable by default.
//      * The client layer should deal with blocking as it is a special case.
// * WSAStartup handles all needed start-up with the help of environment variables.
// * No passthrough to system ws2_32 - this should be handled by the client layer fully.

#ifdef USE_WINSOCK2
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <WinSock.h>
#endif
#include <wsnwlink.h>
#include <siege/platform/win/module.hpp>

import std;
import wsock32.shared;
import wsock32.shared.client;

namespace fs = std::filesystem;

std::optional<backend_imports> backend;

bool use_custom_backend();

extern "C" {
int __stdcall siege_WSAStartup(WORD version, LPWSADATA data)
{
  ensure_imports();
  get_log("in-proc-client") << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version);


  auto env_size = ::GetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", nullptr, 0);

  if (env_size <= 1)
  {
    return imports->WSAStartup(version, data);
  }

  if (backend)
  {
    return 0;
  }

  auto module_path = win32::module_ref::current_module().GetModuleFileName();

  std::string backend_name;

  backend_name.resize(env_size - 1);
  ::GetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", backend_name.data(), backend_name.size() + 1);

  auto zt_path = fs::path(module_path).parent_path() / backend_name;

  get_log() << "Loading backend library: " << zt_path;

  auto module = ::LoadLibraryExW(zt_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

  if (!module)
  {
    return WSASYSNOTREADY;
  }

  backend = std::make_optional(backend_imports{
    .module = module,
    .WSAStartup = (decltype(backend_imports::WSAStartup))::GetProcAddress(module, "backend_WSAStartup"),
    .WSACleanup = (decltype(backend_imports::WSACleanup))::GetProcAddress(module, "backend_WSACleanup"),
    .socket = (decltype(backend_imports::socket))::GetProcAddress(module, "backend_socket"),
    .closesocket = (decltype(backend_imports::closesocket))::GetProcAddress(module, "backend_closesocket"),
    .shutdown = (decltype(backend_imports::shutdown))::GetProcAddress(module, "backend_shutdown"),
    .setsockopt = (decltype(backend_imports::setsockopt))::GetProcAddress(module, "backend_setsockopt"),
    .getsockopt = (decltype(backend_imports::getsockopt))::GetProcAddress(module, "backend_getsockopt"),
    .getsockname = (decltype(backend_imports::getsockname))::GetProcAddress(module, "backend_getsockname"),
    .getpeername = (decltype(backend_imports::getpeername))::GetProcAddress(module, "backend_getpeername"),
    .gethostbyname = (decltype(backend_imports::gethostbyname))::GetProcAddress(module, "backend_gethostbyname"),
    .recvfrom = (decltype(backend_imports::recvfrom))::GetProcAddress(module, "backend_recvfrom"),
    .sendto = (decltype(backend_imports::sendto))::GetProcAddress(module, "backend_sendto"),
    .ioctlsocket = (decltype(backend_imports::ioctlsocket))::GetProcAddress(module, "backend_ioctlsocket"),
    .bind = (decltype(backend_imports::bind))::GetProcAddress(module, "backend_bind"),
    .connect = (decltype(backend_imports::connect))::GetProcAddress(module, "backend_connect"),
    .accept = (decltype(backend_imports::accept))::GetProcAddress(module, "backend_accept"),
    .listen = (decltype(backend_imports::listen))::GetProcAddress(module, "backend_listen"),
    .select = (decltype(backend_imports::select))::GetProcAddress(module, "backend_select"),
    .__WSAFDIsSet = (decltype(backend_imports::__WSAFDIsSet))::GetProcAddress(module, "backend___WSAFDIsSet"),
  });

  return backend->WSAStartup(version, data);
}

int __stdcall siege_WSACleanup()
{
  ensure_imports();
  get_log() << "siege_WSACleanup";

  if (!use_custom_backend())
  {
    return imports->WSACleanup();
  }

  if (!backend)
  {
    imports->WSASetLastError(WSANOTINITIALISED);
    return SOCKET_ERROR;
  }

  // Ideally, we would want to free the backend
  // but this has issues with zero tier.
  // for now, we don't really support dynamic
  // changing of backends nor multiple backends,
  // so it is probably fine.
  // However, we may end up keeping modules loaded anyway
  // and just let the
  /*::FreeLibrary(backend->module);
  backend = std::nullopt;*/

  return backend->WSACleanup();
}

SOCKET __stdcall siege_socket(int af, int type, int protocol) noexcept
{
  ensure_imports();
  get_log() << "siege_socket af: " << af_to_string(af) << ", type: " << type_to_string(type) << ", protocol: " << protocol_to_string(protocol) << ", thread: " << GetCurrentThreadId();

  if (!use_custom_backend())
  {
    return imports->socket(af, type, protocol);
  }

  auto socket = backend->socket(af, type, protocol);

  if (socket != SOCKET_ERROR)
  {
    get_socket_handles().insert(socket);
  }

  return socket;
}

int __stdcall siege_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen)
{
  get_log() << "siege_setsockopt: " << ws << " " << optname;

  if (!use_custom_backend())
  {
    return imports->setsockopt(ws, level, optname, optval, optlen);
  }

  return backend->setsockopt(ws, level, optname, optval, optlen);
}

int __stdcall siege_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen)
{
  get_log() << "siege_getsockopt" << ws << " " << optname;

  if (!use_custom_backend())
  {
    return imports->getsockopt(ws, level, optname, optval, optlen);
  }

  return backend->getsockopt(ws, level, optname, optval, optlen);
}

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


try_again:
  auto result = backend->recvfrom(ws, buf, len, flags, from, fromLen);
  auto last_error = imports->WSAGetLastError();
  if (get_socket_handles().is_virtual_blocking(ws) && result == SOCKET_ERROR && last_error == WSAEWOULDBLOCK)
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(ws, &read_set);

    auto wait_time = [ws]() -> std::optional<timeval> {
      DWORD timeout = 0;
      int param_size = sizeof(timeout);
      auto result = backend->getsockopt(ws, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), &param_size);

      if (result == SOCKET_ERROR || timeout == 0)
      {
        return std::nullopt;
      }
      return ms_to_timeval(std::chrono::milliseconds{ timeout });
    }();

    result = backend->select(1, &read_set, nullptr, nullptr, wait_time ? &*wait_time : nullptr);

    if (result == SOCKET_ERROR)
    {
      return result;
    }
    goto try_again;
  }

  return result;
}


int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept
{
  if (to)
  {
    log_sampled_write() << "siege_sendto with to address " << af_to_string(to->sa_family);
  }
  else
  {
    log_sampled_write() << "siege_sendto with no address";
  }

  if (!use_custom_backend())
  {
    return imports->sendto(ws, buf, len, flags, to, tolen);
  }

try_again:
  auto result = backend->sendto(ws, buf, len, flags, to, tolen);
  auto last_error = imports->WSAGetLastError();

  if (get_socket_handles().is_virtual_blocking(ws) && result == SOCKET_ERROR && last_error == WSAEWOULDBLOCK)
  {
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(ws, &write_set);

    auto wait_time = [ws]() -> std::optional<timeval> {
      DWORD timeout = 0;
      int param_size = sizeof(timeout);
      auto result = backend->getsockopt(ws, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), &param_size);

      if (result == SOCKET_ERROR || timeout == 0)
      {
        return std::nullopt;
      }
      return ms_to_timeval(std::chrono::milliseconds{ timeout });
    }();

    result = backend->select(1, nullptr, &write_set, nullptr, wait_time ? &*wait_time : nullptr);

    if (result == SOCKET_ERROR)
    {
      return result;
    }
    goto try_again;
  }

  return result;
}

int __stdcall siege_getsockname(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getsockname\n";
  if (!use_custom_backend())
  {
    return imports->getsockname(ws, name, length);
  }

  return backend->getsockname(ws, name, length);
}

int __stdcall siege_getpeername(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getpeername\n";
  if (!use_custom_backend())
  {
    return imports->getpeername(ws, name, length);
  }

  return backend->getpeername(ws, name, length);
}

int __stdcall siege_ioctlsocket(SOCKET ws, long cmd, u_long* argp)
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

  auto result = backend->ioctlsocket(ws, cmd, argp);

  if (result == 0 && cmd == FIONBIO && argp)
  {
    get_socket_handles().set_virtual_blocking(ws, *argp == 0);
  }

  return result;
}

int __stdcall siege_listen(SOCKET ws, int backlog)
{
  get_log() << "siege_listen\n";
  if (!use_custom_backend())
  {
    return imports->listen(ws, backlog);
  }

  return backend->listen(ws, backlog);
}

SOCKET __stdcall siege_accept(SOCKET ws, sockaddr* name, int* namelen)
{
  get_log() << "siege_accept\n";
  if (!use_custom_backend())
  {
    return imports->accept(ws, name, namelen);
  }

try_again:
  auto result = backend->accept(ws, name, namelen);
  auto last_error = imports->WSAGetLastError();

  if (get_socket_handles().is_virtual_blocking(ws) && result == INVALID_SOCKET && last_error == WSAEWOULDBLOCK)
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(ws, &read_set);

    auto select_result = backend->select(1, &read_set, nullptr, nullptr, nullptr);

    if (select_result == SOCKET_ERROR)
    {
      return INVALID_SOCKET;
    }
    goto try_again;
  }

  if (result != INVALID_SOCKET)
  {
    get_socket_handles().insert(result);
  }

  return result;
}

int __stdcall siege_connect(SOCKET ws, const sockaddr* name, int namelen)
{
  get_log() << "siege_connect " << ws;

  if (!use_custom_backend())
  {
    return imports->connect(ws, name, namelen);
  }

  auto result = backend->connect(ws, name, namelen);
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
      auto result = backend->getsockopt(ws, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), &param_size);

      if (result == SOCKET_ERROR || timeout == 0)
      {
        return std::nullopt;
      }
      return ms_to_timeval(std::chrono::milliseconds{ timeout });
    }();

    result = backend->select(1, nullptr, &write_set, nullptr, wait_time ? &*wait_time : nullptr);

    if (result == SOCKET_ERROR)
    {
      return result;
    }

    int socket_error = 0;
    int socket_size = sizeof(socket_error);
    result = backend->getsockopt(ws, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socket_error), &socket_size);

    if (result != 0)
    {
      return result;
    }

    if (socket_error != 0)
    {
      imports->WSASetLastError(socket_error);
      return SOCKET_ERROR;
    }

    return 0;
  }
  return result;
}

int __stdcall siege_bind(SOCKET ws, const sockaddr* addr, int namelen)
{
  get_log() << "siege_bind " << ws << std::endl;

  if (!use_custom_backend())
  {
    return imports->bind(ws, addr, namelen);
  }

  return backend->bind(ws, addr, namelen);
}

int __stdcall siege_shutdown(SOCKET ws, int how)
{
  get_log() << "siege_shutdown\n";
  if (!use_custom_backend())
  {
    return imports->shutdown(ws, how);
  }

  return backend->shutdown(ws, how);
}

int __stdcall siege_closesocket(SOCKET ws)
{
  get_log() << "siege_closesocket\n";
  if (!use_custom_backend())
  {
    return imports->closesocket(ws);
  }

  auto result = backend->closesocket(ws);

  if (result != SOCKET_ERROR)
  {
    get_socket_handles().erase(ws);
  }

  return result;
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout)
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

  return backend->select(value, read, write, except, timeout);
}

int __stdcall siege___WSAFDIsSet(SOCKET ws, fd_set* set)
{
  if (!use_custom_backend())
  {
    return imports->__WSAFDIsSet(ws, set);
  }

  return backend->__WSAFDIsSet(ws, set);
}

hostent* __stdcall siege_gethostbyname(const char* name)
{
  ensure_imports();

  if (!use_custom_backend())
  {
    return imports->gethostbyname(name);
  }

  if (name)
  {
    get_log() << "siege_gethostbyname: " << name;
  }
  else
  {
    get_log() << "siege_gethostbyname with no name \n";
  }
  return backend->gethostbyname(name);
}
}
bool use_custom_backend()
{
  return backend.has_value();
}