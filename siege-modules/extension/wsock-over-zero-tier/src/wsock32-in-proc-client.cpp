// TODO Separate the zero tier wrapper part into its own backend dll.
// The API used will be the same as future wrappers.
// The main properties of a backend DLL:
// * Implements the minimal required API set and not everything possible - though still in terms of ws2_32.
// * Sockets are created non-blocking and broadcast capable by default.
//      * The client layer should deal with blocking as it is a special case.
// * WSAStartup handles all needed start-up with the help of environment variables.
// * No passthrough to system ws2_32 - this should be handled by the client layer fully.

#include <ZeroTierSockets.h>

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

HMODULE get_ztlib();
std::optional<std::uint64_t> get_zero_tier_network_id();

bool use_zero_tier();

struct socket_handle_info
{
  void insert(SOCKET socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    handles.insert(socket);

    // we always block on the client layer by default
    virtual_blocking_handles.emplace(socket);
  }

  void erase(SOCKET socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    handles.erase(socket);
  }

  void set_virtual_blocking(SOCKET socket, bool should_block)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    if (!handles.contains(socket))
    {
      return;
    }
    if (should_block)
    {
      virtual_blocking_handles.emplace(socket);
    }
    else
    {
      virtual_blocking_handles.erase(socket);
    }
  }

  bool is_virtual_blocking(SOCKET socket) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return virtual_blocking_handles.contains(socket);
  }

  bool contains(SOCKET socket) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return handles.contains(socket);
  }

private:
  std::set<SOCKET> handles;
  std::set<SOCKET> virtual_blocking_handles;

  mutable std::shared_mutex mutex;
};

socket_handle_info& get_socket_handles()
{
  static socket_handle_info info{};
  return info;
}

extern "C" {
int __stdcall siege_WSAStartup(WORD version, LPWSADATA data)
{
  ensure_imports();
  get_log() << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version);


  if (auto network_id = get_zero_tier_network_id(); network_id && get_ztlib())
  {
    auto module = get_ztlib();
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

  get_log().flush();
  return imports->WSAStartup(version, data);
}

int __stdcall siege_WSACleanup()
{
  ensure_imports();
  get_log() << "siege_WSACleanup";

  if (use_zero_tier())
  {
    return backend->WSACleanup();
  }

  return imports->WSACleanup();
}

static_assert(SOCK_STREAM == ZTS_SOCK_STREAM);
static_assert(SOCK_DGRAM == ZTS_SOCK_DGRAM);
static_assert(SOCK_RAW == ZTS_SOCK_RAW);
static_assert(AF_UNSPEC == ZTS_AF_UNSPEC);
static_assert(AF_INET == ZTS_AF_INET);
static_assert(IPPROTO_IP == ZTS_IPPROTO_IP);
static_assert(IPPROTO_TCP == ZTS_IPPROTO_TCP);
static_assert(IPPROTO_UDP == ZTS_IPPROTO_UDP);
static_assert(IPPROTO_ICMP == ZTS_IPPROTO_ICMP);
static_assert(IPPROTO_RAW == ZTS_IPPROTO_RAW);
static_assert(AF_INET == ZTS_AF_INET);
SOCKET __stdcall siege_socket(int af, int type, int protocol)
{
  ensure_imports();
  get_log() << "siege_socket af: " << af_to_string(af) << ", type: " << type_to_string(type) << ", protocol: " << protocol_to_string(protocol) << ", thread: " << GetCurrentThreadId();

  if (use_zero_tier())
  {
    auto socket = backend->socket(af, type, protocol);

    if (socket != SOCKET_ERROR)
    {
      get_socket_handles().insert(socket);
    }

    return socket;
  }
  return imports->socket(af, type, protocol);
}

static_assert(SO_DEBUG == ZTS_SO_DEBUG);
static_assert(SO_ACCEPTCONN == ZTS_SO_ACCEPTCONN);
static_assert(SO_REUSEADDR == ZTS_SO_REUSEADDR);
static_assert(SO_KEEPALIVE == ZTS_SO_KEEPALIVE);
static_assert(SO_DONTROUTE == ZTS_SO_DONTROUTE);
static_assert(SO_BROADCAST == ZTS_SO_BROADCAST);
static_assert(SO_USELOOPBACK == ZTS_SO_USELOOPBACK);
static_assert(SO_SNDTIMEO == ZTS_SO_SNDTIMEO);
static_assert(SO_RCVTIMEO == ZTS_SO_RCVTIMEO);
static_assert(SO_RCVBUF == ZTS_SO_RCVBUF);
static_assert(SO_SNDBUF == ZTS_SO_SNDBUF);
static_assert(SO_ERROR == ZTS_SO_ERROR);
static_assert(SO_LINGER == ZTS_SO_LINGER);
static_assert(SO_ACCEPTCONN == ZTS_SO_ACCEPTCONN);
static_assert(SOL_SOCKET != ZTS_SOL_SOCKET);
int __stdcall siege_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen)
{
  get_log() << "siege_setsockopt: " << ws << " " << optname;

  if (use_zero_tier())
  {
    return backend->setsockopt(ws, level, optname, optval, optlen);
  }

  return imports->setsockopt(ws, level, optname, optval, optlen);
}

int __stdcall siege_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen)
{
  get_log() << "siege_getsockopt" << ws << " " << optname;
  if (use_zero_tier())
  {
    return backend->getsockopt(ws, level, optname, optval, optlen);
  }

  auto result = imports->getsockopt(ws, level, optname, optval, optlen);

  if (result != 0)
  {
    get_log() << "getsockopt WSAGetLastError " << imports->WSAGetLastError();
  }

  return result;
}

int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept
{
  if (!use_zero_tier())
  {
    auto result = imports->recvfrom(ws, buf, len, flags, from, fromLen);

    if (result < 0)
    {
      get_log() << "recvfrom WSAGetLastError " << imports->WSAGetLastError();
    }

    return result;
  }

try_again:
  auto result = backend->recvfrom(ws, buf, len, flags, from, fromLen);
  auto last_error = imports->WSAGetLastError();
  if (get_socket_handles().is_virtual_blocking(ws) && result == SOCKET_ERROR && last_error == WSAEWOULDBLOCK)
  {
    DWORD timeout = 0;
    int param_size = sizeof(timeout);
    result = backend->getsockopt(ws, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), &param_size);

    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(ws, &read_set);

    timeval wait_time{
        .tv_usec = static_cast<long>(timeout * 1000u)
    };

    result = backend->select(1, &read_set, nullptr, nullptr, timeout ? &wait_time : nullptr);

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
  if (!use_zero_tier())
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

    result = backend->select(1, nullptr, &write_set, nullptr, nullptr);

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
  if (use_zero_tier())
  {
    return backend->getsockname(ws, name, length);
  }
  return imports->getsockname(ws, name, length);
}

int __stdcall siege_getpeername(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getpeername\n";
  if (use_zero_tier())
  {
    return backend->getpeername(ws, name, length);
  }

  return imports->getpeername(ws, name, length);
}

static_assert(FIONREAD == ZTS_FIONREAD);
static_assert(FIONBIO == ZTS_FIONBIO);
static_assert(IOCPARM_MASK == ZTS_IOCPARM_MASK);
static_assert(IOC_VOID == ZTS_IOC_VOID);
static_assert(IOC_OUT == ZTS_IOC_OUT);
static_assert(IOC_IN == ZTS_IOC_IN);
static_assert(IOC_INOUT == ZTS_IOC_INOUT);
int __stdcall siege_ioctlsocket(SOCKET ws, long cmd, u_long* argp)
{
  get_log() << "siege_ioctlsocket, cmd: " << ioctl_cmd_to_string(cmd);
  if (use_zero_tier())
  {
    return backend->ioctlsocket(ws, cmd, argp);
  }
  auto result = imports->ioctlsocket(ws, cmd, argp);

  if (result == 0 && cmd == FIONBIO && argp)
  {
    get_socket_handles().set_virtual_blocking(ws, *argp == 0);
  }

  get_log() << "siege_ioctlsocket finished";

  return result;
}

int __stdcall siege_listen(SOCKET ws, int backlog)
{
  get_log() << "siege_listen\n";
  if (use_zero_tier())
  {
    return backend->listen(ws, backlog);
  }
  return imports->listen(ws, backlog);
}

SOCKET __stdcall siege_accept(SOCKET ws, sockaddr* name, int* namelen)
{
  get_log() << "siege_accept\n";
  if (use_zero_tier())
  {
    return backend->accept(ws, name, namelen);
  }

  return imports->accept(ws, name, namelen);
}

int __stdcall siege_connect(SOCKET ws, const sockaddr* name, int namelen)
{
  get_log() << "siege_connect " << ws;

  if (use_zero_tier())
  {
    return backend->connect(ws, name, namelen);
  }

  return imports->connect(ws, name, namelen);
}

int __stdcall siege_bind(SOCKET ws, const sockaddr* addr, int namelen)
{
  get_log() << "siege_bind " << ws << std::endl;

  if (use_zero_tier())
  {
    return backend->bind(ws, addr, namelen);
  }

  auto result = imports->bind(ws, addr, namelen);

  get_log() << "Bind call has error " << imports->WSAGetLastError();

  return result;
}

#ifdef SD_RECEIVE
static_assert(SD_RECEIVE == ZTS_SHUT_RD);
static_assert(SD_SEND == ZTS_SHUT_WR);
static_assert(SD_BOTH == ZTS_SHUT_RDWR);
#endif
int __stdcall siege_shutdown(SOCKET ws, int how)
{
  get_log() << "siege_shutdown\n";
  if (use_zero_tier())
  {
    return backend->shutdown(ws, how);
  }
  return imports->shutdown(ws, how);
}

int __stdcall siege_closesocket(SOCKET ws)
{
  get_log() << "siege_closesocket\n";
  if (use_zero_tier())
  {
    auto result = backend->closesocket(ws);

    if (result != SOCKET_ERROR)
    {
      get_socket_handles().erase(ws);
    }

    return result;
  }
  return imports->closesocket(ws);
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout)
{
  if (use_zero_tier())
  {
    return backend->select(value, read, write, except, timeout);
  }

  return imports->select(value, read, write, except, timeout);
}

int __stdcall siege___WSAFDIsSet(SOCKET ws, fd_set* set)
{
  if (use_zero_tier())
  {
    return backend->__WSAFDIsSet(ws, set);
  }

  return imports->__WSAFDIsSet(ws, set);
}

hostent* __stdcall siege_gethostbyname(const char* name)
{
  ensure_imports();
  if (name)
  {
    get_log() << "siege_gethostbyname: " << name;
  }
  else
  {
    get_log() << "siege_gethostbyname with no name \n";
  }

  if (use_zero_tier())
  {
    return imports->gethostbyname(name);
  }

  return imports->gethostbyname(name);
}
}
HMODULE get_ztlib()
{
  static HMODULE ztlib = [] {
    auto module_path = win32::module_ref::current_module().GetModuleFileName();

    auto zt_path = fs::path(module_path).parent_path() / "ws2_32-on-zero-tier.dll";

    get_log() << "Loading zero tier library: " << zt_path;

    auto result = ::LoadLibraryExW(zt_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

    return result;
  }();

  return ztlib;
}

std::optional<std::uint64_t> get_zero_tier_network_id()
{
  auto load_network_id = []() -> std::optional<std::uint64_t> {
    try
    {
      get_log() << "get_zero_tier_network_id\n";


      if (auto env_size = ::GetEnvironmentVariableA("ZERO_TIER_NETWORK_ID", nullptr, 0); env_size >= 1)
      {
        std::string network_id(env_size - 1, '\0');
        ::GetEnvironmentVariableA("ZERO_TIER_NETWORK_ID", network_id.data(), network_id.size() + 1);

        get_log() << "Zero Tier Network ID is " << network_id;
        return std::strtoull(network_id.data(), 0, 16);
      }

      get_log() << "No zero tier network ID\n";
      return std::nullopt;
    }
    catch (...)
    {
      return std::nullopt;
    }
  };

#ifdef _DEBUG
  std::optional<std::uint64_t> result = load_network_id();
#else
  static std::optional<std::uint64_t> result = load_network_id();
#endif

  return result;
}

bool use_zero_tier()
{
  return get_zero_tier_network_id() && backend;
}