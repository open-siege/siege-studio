#include <ZeroTierSockets.h>
#include <winsock.h>
#include <optional>
#include <string>
#include <map>
#include <set>
#include <fstream>

static_assert(SOCK_STREAM == ZTS_SOCK_STREAM);
static_assert(SOCK_DGRAM == ZTS_SOCK_DGRAM);
static_assert(SOCK_RAW == ZTS_SOCK_RAW);
static_assert(AF_UNSPEC == ZTS_AF_UNSPEC);
static_assert(AF_INET == ZTS_AF_INET);

static_assert(SO_DEBUG == ZTS_SO_DEBUG);
static_assert(SO_ACCEPTCONN == ZTS_SO_ACCEPTCONN);

std::set<int>& get_zero_tier_handles()
{
  static std::set<int> handles;
  return handles;
}

HMODULE get_ztlib()
{
  static HMODULE ztlib = nullptr;
  return ztlib;
}

std::optional<std::string> get_zero_tier_network()
{
  static std::optional<std::string> result;

  return result;
}

int zt_to_winsock_result(int error)
{
  return error;
}

std::ofstream& get_log()
{
  static std::ofstream file_log("networking.log", std::ios::trunc);

  return file_log;
}

zts_sockaddr_in to_zts(sockaddr_in addr)
{
  zts_sockaddr_in zt_addr{
    .sin_len = sizeof(zts_sockaddr_in),
    .sin_family = ZTS_AF_INET,
    .sin_port = addr.sin_port
  };

  std::memcpy(&zt_addr.sin_addr, &addr.sin_addr, sizeof(zt_addr.sin_addr));
  std::memcpy(&zt_addr.sin_zero, &addr.sin_zero, sizeof(zt_addr.sin_zero));
  return zt_addr;
}

sockaddr_in from_zts(zts_sockaddr_in zt_addr)
{
  sockaddr_in addr{
    .sin_family = ZTS_AF_INET,
    .sin_port = zt_addr.sin_port
  };

  std::memcpy(&addr.sin_addr, &zt_addr.sin_addr, sizeof(addr.sin_addr));
  std::memcpy(&addr.sin_zero, &zt_addr.sin_zero, sizeof(addr.sin_zero));
  return addr;
}

hostent from_zts(zts_hostent zt_host)
{
  hostent temp{
    .h_name = zt_host.h_name,
    .h_aliases = zt_host.h_aliases,
    .h_addrtype = (short)zt_host.h_addrtype,
    .h_length = (short)zt_host.h_length,
    .h_addr_list = zt_host.h_addr_list
  };
  return temp;
}

extern "C" {
int __stdcall siege_WSAStartup(WORD version, LPWSADATA data)
{
  get_log() << "siege_WSAStartup\n";
  auto result = WSAStartup(version, data);
  // TODO Init WSA then init zero tier
  // TODO get zero tier network from environment variable
  // if zero tier dll and network variable are both available, then init zero tier.
  return result;
}

int __stdcall siege_WSACleanup()
{
  get_log() << "siege_WSACleanup\n";
  // TODO cleanup zero tier
  return WSACleanup();
}

int get_zts_errno()
{
  return *(int*)::GetProcAddress(get_ztlib(), "zts_errno");
}

SOCKET __stdcall siege_socket(int af, int type, int protocol)
{
  get_log() << "siege_socket\n";
  if ((af == AF_UNSPEC || af == AF_INET) &&
      // TODO check protocol
      // TODO check protocol
      get_zero_tier_network() && get_ztlib())
  {
    static auto* zt_socket = (std::add_pointer_t<decltype(zts_bsd_socket)>)::GetProcAddress(get_ztlib(), "zts_bsd_socket");
    auto socket = zt_socket(af, type, protocol);

    if (!socket)
    {
      ::WSASetLastError(zt_to_winsock_result(get_zts_errno()));
      return SOCKET{};
    }

    get_zero_tier_handles().emplace(socket);

    return (SOCKET)socket;
  }

  return socket(af, type, protocol);
}

// no ordinal
int __stdcall siege_recv(SOCKET s, char* buf, int len, int flags)
{
  get_log() << "siege_recv\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_recv = (std::add_pointer_t<decltype(zts_bsd_recv)>)::GetProcAddress(get_ztlib(), "zts_bsd_recv");

    // TODO convert flags
    // MSG_PEEK
    // MSG_OOB
    // MSG_WAITALL
    auto zt_flags = flags;

    auto zt_result = (int)zt_recv((int)s, buf, (std::size_t)len, zt_flags);

    if (!zt_result)
    {
      ::WSASetLastError(zt_to_winsock_result(get_zts_errno()));
      return 0;
    }
    // TODO map result
    return zt_result;
  }
  return recv(s, buf, len, flags);
}

int __stdcall siege_setsockopt_v20()
{
  return 0;
}

// no ordinal, setsockopt_v11
int __stdcall siege_setsockopt_v11(SOCKET s, int level, int optname, const char* optval, int optlen)
{
  get_log() << "siege_setsockopt\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    // TODO map level
    // SOL_SOCKET
    // TODO map optname
    // SO_BROADCAST
    // SO_CONDITIONAL_ACCEPT
    // SO_DEBUG
    // SO_DONTLINGER
    // SO_DONTROUTE
    // SO_GROUP_PRIORITY
    // SO_LINGER
    // SO_OOBINLINE
    // SO_RCVBUF
    // SO_REUSEADDR
    // SO_EXCLUSIVEADDUSE
    // SO_RCVTIMEO
    // SO_SNDBUF
    // SO_SNDTIMEO
    // SO_UPDATE_ACCEPT_CONTEXT
    // PVD_CONFIG

    // TODO level IPPROTO_TCP
    //

    static auto* zt_setsockopt = (std::add_pointer_t<decltype(zts_bsd_setsockopt)>)::GetProcAddress(get_ztlib(), "zts_bsd_setsockopt");
    auto zt_result = zt_setsockopt((int)s, level, optname, optval, optlen);

    // TODO map result
    return zt_result;
  }

  return setsockopt(s, level, optname, optval, optlen);
}

// no ordinal
int __stdcall siege_recvfrom(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromLen)
{
  get_log() << "siege_recvfrom\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_recvfrom = (std::add_pointer_t<decltype(zts_bsd_recvfrom)>)::GetProcAddress(get_ztlib(), "zts_bsd_recvfrom");

    zts_sockaddr_in zt_addr{};
    zts_socklen_t zt_size = sizeof(zt_addr);

    // TODO map flags
    // MSG_PEEK
    // MSG_OOB
    auto zt_flags = flags;

    auto zt_result = (int)zt_recvfrom((int)s, buf, len, zt_flags, (zts_sockaddr*)&zt_addr, &zt_size);
    sockaddr_in from_in = from_zts(zt_addr);
    std::memcpy(&from, &from_in, sizeof(from_in));
    *fromLen = sizeof(from_in);
    // TODO map result
    return zt_result;
  }

  return recvfrom(s, buf, len, flags, from, fromLen);
}

int __stdcall siege_getsockname(SOCKET s, sockaddr* name, int* length)
{
  get_log() << "siege_recvfrom\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_getsockname = (std::add_pointer_t<decltype(zts_bsd_getsockname)>)::GetProcAddress(get_ztlib(), "zts_bsd_getsockname");

    zts_sockaddr_in zt_addr{};

    zts_socklen_t zt_size = sizeof(zt_addr);

    auto zt_result = zt_getsockname((int)s, (zts_sockaddr*)&zt_addr, &zt_size);

    sockaddr_in from_in = from_zts(zt_addr);
    std::memcpy(&name, &from_in, sizeof(from_in));
    *length = sizeof(from_in);

    // TODO map result
    return zt_result;
  }
  return getsockname(s, name, length);
}

int __stdcall siege_ioctlsocket(SOCKET s, long cmd, u_long* argp)
{
  get_log() << "siege_ioctlsocket\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_ioctl = (std::add_pointer_t<decltype(zts_bsd_ioctl)>)::GetProcAddress(get_ztlib(), "zts_bsd_ioctl");

    // TODO investigate whether cmd needs to be mapped and how
    auto zt_result = zt_ioctl((int)s, cmd, argp);

    // TODO map result
    return zt_result;
  }
  return ioctlsocket(s, cmd, argp);
}

int __stdcall siege_connect(SOCKET s, const sockaddr* name, int namelen)
{
  get_log() << "siege_connect\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_connect = (std::add_pointer_t<decltype(zts_bsd_connect)>)::GetProcAddress(get_ztlib(), "zts_bsd_connect");

    zts_sockaddr_in zt_addr = to_zts(*(sockaddr_in*)name);

    zts_socklen_t zt_size = sizeof(zt_addr);

    auto zt_result = zt_connect((int)s, (zts_sockaddr*)&zt_addr, zt_size);

    // TODO map result
    return zt_result;
  }
  return connect(s, name, namelen);
}

int __stdcall siege_bind(SOCKET s, const sockaddr* addr, int namelen)
{
  get_log() << "siege_bind\n";
  // if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  //{
  //     static auto* zt_bind = (std::add_pointer_t<decltype(zts_bsd_bind)>)::GetProcAddress(get_ztlib(), "zts_bsd_bind");
  //
  //     zts_sockaddr_in zt_addr = to_zts(*(sockaddr_in*)addr);
  //     zts_socklen_t zt_size = sizeof(zt_addr);

  //    auto zt_result = zt_bind((int)s, (zts_sockaddr*)&zt_addr, zt_size);

  //    // TODO map result
  //    return zt_result;
  //}
  auto result = bind(s, addr, namelen);

  get_log() << "Bind call succeeded with value " << WSAGetLastError() << "\n";

  return result;
}

int __stdcall siege_send(SOCKET s, const char* buf, int len, int flags)
{
  get_log() << "siege_send\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_send = (std::add_pointer_t<decltype(zts_bsd_send)>)::GetProcAddress(get_ztlib(), "zts_bsd_send");

    // TODO convert flags here
    // MSG_DONTROUTE
    // MSG_OOB
    auto zt_flags = flags;

    auto zt_result = zt_send((int)s, buf, (std::size_t)len, zt_flags);

    // TODO map result
    return zt_result;
  }
  return send(s, buf, len, flags);
}

int __stdcall siege_sendto(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
  get_log() << "siege_sendto\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_sendto = (std::add_pointer_t<decltype(zts_bsd_sendto)>)::GetProcAddress(get_ztlib(), "zts_bsd_sendto");

    // TODO convert flags here
    // MSG_DONTROUTE
    // MSG_OOB
    auto zt_flags = flags;

    zts_sockaddr_in zt_addr{

    };

    zts_socklen_t zt_size = sizeof(zt_addr);

    auto zt_result = zt_sendto((int)s, buf, len, zt_flags, (zts_sockaddr*)&zt_addr, zt_size);

    // TODO map result
    return zt_result;
  }
  return sendto(s, buf, len, flags, to, tolen);
}

int __stdcall siege_shutdown(SOCKET s, int how)
{
  get_log() << "siege_shutdown\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_shutdown = (std::add_pointer_t<decltype(zts_bsd_shutdown)>)::GetProcAddress(get_ztlib(), "zts_bsd_shutdown");

    // TODO map zt_how
    // SD_RECEIVE
    // SD_SEND
    // SD_BOTH
    auto zt_how = how;
    auto zt_result = zt_shutdown((int)s, zt_how);

    // TODO map result
    return zt_result;
  }
  return shutdown(s, how);
}

int __stdcall siege_closesocket(SOCKET s)
{
  get_log() << "siege_closesocket\n";
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    static auto* zt_close = (std::add_pointer_t<decltype(zts_bsd_close)>)::GetProcAddress(get_ztlib(), "zts_bsd_close");
    auto zt_result = zt_close((int)s);

    // TODO map result
    return zt_result;
  }
  return closesocket(s);
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout)
{
  get_log() << "siege_select\n";
  if (get_zero_tier_network() && get_ztlib())
  {
    static auto* zt_select = (std::add_pointer_t<decltype(zts_bsd_select)>)::GetProcAddress(get_ztlib(), "zts_bsd_select");
    zts_fd_set zt_read{};
    zts_fd_set zt_write{};
    zts_fd_set zt_except{};
  }
  return select(value, read, write, except, timeout);
}

hostent* __stdcall siege_gethostbyname(const char* name)
{
  if (name)
  {
    get_log() << "siege_gethostbyname: " << name << "\n";
  }
  else
  {
    get_log() << "siege_gethostbyname with no name \n";
  }

  if (get_zero_tier_network() && get_ztlib() && name)
  {
    static auto* zt_host = (std::add_pointer_t<decltype(zts_bsd_gethostbyname)>)::GetProcAddress(get_ztlib(), "zts_bsd_gethostbyname");
    auto result = zt_host(name);

    if (result)
    {
      static std::map<std::string, hostent> host_cache;
      host_cache[name] = from_zts(*result);
      return &host_cache[name];
    }
  }

  return gethostbyname(name);
}
}
