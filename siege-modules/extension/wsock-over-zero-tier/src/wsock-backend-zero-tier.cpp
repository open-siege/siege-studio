#ifdef SIEGE_ZTS_USE_CRT_ERRNO
#include <cerrno>
#endif
#include <ZeroTierSockets.h>

#ifdef USE_WINSOCK2
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <WinSock.h>
#endif
#include <wsnwlink.h>
#include <siege/platform/win/module.hpp>
#include <siege/platform/shared.hpp>
#include <cassert>

import std;
import wsock32.shared;

namespace fs = std::filesystem;

struct socket_handle_info
{
  void insert(int socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    handles.emplace(socket, context{});
  }

  void erase(int socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    handles.erase(socket);
  }

  bool contains(int value) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return handles.contains(value);
  }

private:
  struct context
  {
  };

  std::map<int, context> handles;

  mutable std::shared_mutex mutex;
};

socket_handle_info& get_socket_handles()
{
  static socket_handle_info info{};
  return info;
}

auto zt_lock()
{
  static std::mutex main;
  return std::unique_lock<std::mutex>{ main };
}

std::optional<std::uint64_t> get_network_id();
std::optional<std::string> get_peer_id_and_public_key();
std::shared_ptr<char> get_shared_current_ip_address_storage();
int zt_to_winsock_error(int);
int zt_to_winsock_result(int code);
void wait_for_network_ready();

bool fallback_broadcast_sorter(std::uint32_t, std::uint32_t);
std::set<std::uint32_t, decltype(fallback_broadcast_sorter)*>& get_fallback_broadcast_addresses();
const std::map<std::uint32_t, std::uint32_t>& get_subnets();
const std::set<std::uint32_t>& get_directed_broadcasts();
void rewrite_if_foreign(zts_sockaddr_in&);

int to_zt_msg_flags(int flags);
zts_sockaddr_in to_zts(sockaddr_in addr);
int get_zts_errno();
bool& get_node_online_status();
sockaddr_in from_zts(zts_sockaddr_in zt_addr);
hostent from_zts(zts_hostent zt_host);
void copy_address(zts_sockaddr_in addr, sockaddr* name, int* length);
std::pair<zts_sockaddr_in, zts_socklen_t> copy_address(const sockaddr* name, int length);
std::string_view format_ipv4(std::array<char, 16>& out, std::uint32_t addr_nbo);
std::string_view format_ipv4_port(std::array<char, 32>& out, std::uint32_t addr_nbo, std::uint16_t port_nbo);

SOCKET from_zts(int);
int to_zts(SOCKET);

extern "C" {
int __stdcall backend_WSAStartup(WORD version, LPWSADATA data)
{
  auto _ = zt_lock();
  ensure_imports();
  get_log("backend.zero-tier") << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version);
  auto result = imports->WSAStartup(version, data);

  if (auto network_id = get_network_id())
  {
    if (result == 0)
    {
      imports->WSACleanup();
    }

    get_log() << "Zero Tier library available and network is set\n";

    if (!get_node_online_status())
    {
      if (auto node_id_and_key = get_peer_id_and_public_key(); node_id_and_key)
      {
        auto init_result = zts_init_from_memory(node_id_and_key->data(), (unsigned int)node_id_and_key->size());
        if (init_result == 0)
        {
          get_log() << "Init from memory succeeded\n";
        }
        else
        {
          get_log() << "Could not init from memory with code" << get_zts_errno();
        }
      }
      else
      {
        if (auto dll_path = win32::get_dll_directory(); dll_path)
        {
          auto dll_path_str = dll_path->string();
          zts_init_from_storage(dll_path_str.data());
        }
        else
        {
          zts_init_from_storage(".");
        }
      }

      get_log() << "Starting node\n";
      zts_node_start();
      bool is_online = false;

      for (auto i = 0; i < 500; ++i)
      {
        if (zts_node_is_online())
        {
          auto id = zts_node_get_id();

          get_log() << "Node is online with ID: " << std::to_string(id);

          is_online = true;
          break;
        }
        zts_util_delay(100);
      }

      if (!is_online)
      {
        get_log() << "Node could not be started.\n";
        return WSASYSNOTREADY;
      }

      zts_net_join(*network_id);
      get_log() << "Joining network\n";
      get_node_online_status() = true;
    }

    return 0;
  }

  get_log().flush();
  return result;
}

int __stdcall backend_WSACleanup()
{
  auto _ = zt_lock();
  ensure_imports();
  get_log() << "siege_WSACleanup";

  if (get_node_online_status())
  {
    get_log() << "Stopping Zero Tier node\n";
    auto zt_result = zts_node_stop();
    get_node_online_status() = false;
    return zt_to_winsock_result(zt_result);
  }
  return 0;
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
SOCKET __stdcall backend_socket(int af, int type, int protocol)
{
  auto _ = zt_lock();
  ensure_imports();
  get_log() << "siege_socket af: " << af_to_string(af) << ", type: " << type_to_string(type) << ", protocol: " << protocol_to_string(protocol) << ", thread: " << GetCurrentThreadId();

  if ((af == AF_UNSPEC || af == AF_INET) && (type == SOCK_STREAM || type == SOCK_DGRAM))
  {
    get_log() << "Creating zero tier socket\n";
    auto socket = zts_bsd_socket(af, type, protocol);

    if (!(protocol == ZTS_IPPROTO_IP
          || protocol == ZTS_IPPROTO_ICMP
          || protocol == ZTS_IPPROTO_TCP
          || protocol == ZTS_IPPROTO_UDP
          || protocol == ZTS_IPPROTO_RAW))
    {
      get_log() << "Unsupported protocol for zero tier: " << protocol;
    }

    if (socket == ZTS_ERR_SOCKET || socket == ZTS_ERR_SERVICE || socket == ZTS_ERR_ARG)
    {
      get_log() << "Could not create zero tier socket with error code: " << get_zts_errno();

      if (socket == ZTS_ERR_ARG)
      {
        imports->WSASetLastError(zt_to_winsock_error(get_zts_errno()));
      }
      else if (socket == ZTS_ERR_SERVICE)
      {
        imports->WSASetLastError(WSANOTINITIALISED);
      }

      return INVALID_SOCKET;
    }

    int non_blocking = 1;

    if (zts_bsd_ioctl(socket, ZTS_FIONBIO, &non_blocking) < 0)
    {
      zts_bsd_close(socket);
      return INVALID_SOCKET;
    }

    get_log() << "Created zero tier socket successfully (" << socket << ")";
    get_socket_handles().insert(socket);


    int value = 1;
    zts_socklen_t size = sizeof(value);

    zts_bsd_setsockopt(socket, ZTS_SOL_SOCKET, ZTS_SO_BROADCAST, &value, size);
    value = 65536;

    zts_bsd_setsockopt(socket, ZTS_SOL_SOCKET, ZTS_SO_RCVBUF, &value, size);
    return from_zts(socket);
  }

  imports->WSASetLastError(WSAESOCKTNOSUPPORT);
  return INVALID_SOCKET;
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
static_assert(SOL_SOCKET != ZTS_SOL_SOCKET);
int __stdcall backend_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen)
{
  auto _ = zt_lock();
  get_log() << "siege_setsockopt: " << ws << " " << optname;
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  if ((level == SOL_SOCKET || level == ZTS_SOL_SOCKET) && optname == SO_SNDBUF)
  {
    return 0;
  }

  get_log() << "zts_bsd_setsockopt, socket: " << to_zts(ws) << " level: " << level_to_string(level) << " optname: " << option_to_string(optname);

  if (level != SOL_SOCKET)
  {
    get_log() << "Potentially unsupported socket level " << level;
  }

  BOOL some_flag = -1;

  zts_socklen_t size = sizeof(some_flag);

  if (level == SOL_SOCKET)
  {
    level = ZTS_SOL_SOCKET;
  }

  int zt_result;

  if (optval && optlen == sizeof(DWORD) && (level == SOL_SOCKET || level == ZTS_SOL_SOCKET) && (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO))
  {
    get_log() << "Converting timeout values to correct format\n";
    DWORD milliseconds = 0;
    std::memcpy(&milliseconds, optval, optlen);

    zts_timeval timeout{
      .tv_usec = (long)milliseconds * 1000
    };
    zt_result = zts_bsd_setsockopt(to_zts(ws), level, optname, &timeout, sizeof(timeout));
  }
  else
  {
    zt_result = zts_bsd_setsockopt(to_zts(ws), level, optname, optval, optlen);
  }

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen)
{
  auto _ = zt_lock();
  get_log() << "siege_getsockopt" << to_zts(ws) << " " << optname;
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  if (optname == SO_BROADCAST && optval && optlen && *optlen >= sizeof(BOOL))
  {
    BOOL value = TRUE;
    std::memcpy(optval, &value, sizeof(value));
    return 0;
  }


  get_log() << "zts_bsd_getsockopt, level: " << level << " optname: " << optname;

  if (level != SOL_SOCKET)
  {
    get_log() << "Potentially unsupported socket level " << level;
  }

  BOOL some_flag = -1;
  zts_socklen_t size = 0;

  if (optlen)
  {
    size = *optlen;
  }

  if (level == SOL_SOCKET)
  {
    level = ZTS_SOL_SOCKET;
  }
  else
  {
    get_log() << "Getting a regular socket setting " << optname;
  }

  int zt_result;

  if (optval && optlen && *optlen == sizeof(DWORD) && level == ZTS_SOL_SOCKET && (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO))
  {
    get_log() << "Converting timeout values to correct format\n";

    zts_timeval timeout{};
    size = sizeof(timeout);
    zt_result = zts_bsd_getsockopt(to_zts(ws), level, optname, &timeout, &size);
    size = sizeof(DWORD);

    if (zt_result == 0)
    {
      DWORD milliseconds = static_cast<DWORD>(timeout.tv_sec) * 1000u + static_cast<DWORD>(timeout.tv_usec) / 1000u;
      std::memcpy(optval, &milliseconds, *optlen);
    }
  }
  else if (optval && optlen && *optlen == sizeof(DWORD) && level == ZTS_SOL_SOCKET && optname == SO_ERROR)
  {
    DWORD error = 0;
    zt_result = zts_bsd_getsockopt(to_zts(ws), level, optname, &error, &size);

    if (zt_result == 0)
    {
      error = static_cast<DWORD>(zt_to_winsock_error(static_cast<int>(error)));
      std::memcpy(optval, &error, sizeof(error));
    }
  }
  else
  {
    zt_result = zts_bsd_getsockopt(to_zts(ws), level, optname, optval, &size);
  }

  if (optlen)
  {
    *optlen = size;
  }

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept
{
  auto _ = zt_lock();
  imports->WSASetLastError(0);
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  wait_for_network_ready();

  zts_sockaddr_in zt_addr{
    .sin_len = sizeof(zts_sockaddr_in)
  };

  zts_socklen_t zt_size = sizeof(zt_addr);

  int zt_result = 0;

  if (from)
  {
    zt_result = (int)zts_bsd_recvfrom(to_zts(ws), buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&zt_addr, &zt_size);
  }
  else
  {
    zt_result = (int)zts_bsd_recvfrom(to_zts(ws), buf, len, to_zt_msg_flags(flags), nullptr, nullptr);
  }

  if (zt_result == ZTS_ERR_SOCKET || zt_result == ZTS_ERR_SERVICE || zt_result == ZTS_ERR_ARG)
  {
    return zt_to_winsock_result(zt_result);
  }

  if (from && zt_result >= 0)
  {
    std::array<char, 32> from_addr{};
    log_sampled_read() << "zts_bsd_recvfrom from " << format_ipv4_port(from_addr, zt_addr.sin_addr.S_addr, zt_addr.sin_port) << "\n";
  }

  if (zt_addr.sin_addr.S_addr)
  {
    get_fallback_broadcast_addresses().emplace(zt_addr.sin_addr.S_addr);
  }

  copy_address(zt_addr, from, fromLen);

  return (int)zt_result;
}

int __stdcall backend_getsockname(SOCKET ws, sockaddr* name, int* length)
{
  auto _ = zt_lock();
  get_log() << "siege_getsockname\n";
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  if (name && length)
  {
    get_log() << "zts_bsd_getsockname\n";
    zts_sockaddr_in zt_addr{
      .sin_len = sizeof(zts_sockaddr_in)
    };

    zts_socklen_t zt_size = sizeof(zt_addr);

    auto zt_result = zts_bsd_getsockname(to_zts(ws), (zts_sockaddr*)&zt_addr, &zt_size);

    if (zt_result < 0)
    {
      return zt_to_winsock_result(zt_result);
    }

    copy_address(zt_addr, name, length);

    return zt_to_winsock_result(zt_result);
  }

  auto zt_result = zts_bsd_getsockname(to_zts(ws), nullptr, nullptr);
  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_getpeername(SOCKET ws, sockaddr* name, int* length)
{
  auto _ = zt_lock();
  get_log() << "siege_getpeername\n";
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  if (name && length)
  {
    get_log() << "zts_bsd_getpeername\n";
    zts_sockaddr_in zt_addr{
      .sin_len = sizeof(zts_sockaddr_in)
    };

    zts_socklen_t zt_size = sizeof(zt_addr);

    auto zt_result = zts_bsd_getpeername(to_zts(ws), (zts_sockaddr*)&zt_addr, &zt_size);

    if (zt_result < 0)
    {
      return zt_to_winsock_result(zt_result);
    }

    copy_address(zt_addr, name, length);

    return zt_to_winsock_result(zt_result);
  }

  auto zt_result = zts_bsd_getpeername(to_zts(ws), nullptr, nullptr);
  return zt_to_winsock_result(zt_result);
}

static_assert(FIONREAD == ZTS_FIONREAD);
static_assert(FIONBIO == ZTS_FIONBIO);
static_assert(IOCPARM_MASK == ZTS_IOCPARM_MASK);
static_assert(IOC_VOID == ZTS_IOC_VOID);
static_assert(IOC_OUT == ZTS_IOC_OUT);
static_assert(IOC_IN == ZTS_IOC_IN);
static_assert(IOC_INOUT == ZTS_IOC_INOUT);
int __stdcall backend_ioctlsocket(SOCKET ws, long cmd, u_long* argp)
{
  auto _ = zt_lock();
  if (cmd != FIONREAD)
  {
    get_log() << "siege_ioctlsocket, cmd: " << ioctl_cmd_to_string(cmd);
  }

  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  auto zt_result = zts_bsd_ioctl(to_zts(ws), cmd, argp);

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_listen(SOCKET ws, int backlog)
{
  auto _ = zt_lock();
  get_log() << "siege_listen\n";

  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  wait_for_network_ready();

  if (backlog == SOMAXCONN)
  {
    backlog = ZTS_FD_SETSIZE;
  }
  auto zt_result = zts_bsd_listen(to_zts(ws), backlog);
  return zt_to_winsock_result(zt_result);
}

SOCKET __stdcall backend_accept(SOCKET ws, sockaddr* name, int* namelen)
{
  auto _ = zt_lock();
  log_sampled_check() << "siege_accept\n";
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  wait_for_network_ready();

  log_sampled_check() << "zts_bsd_accept\n";

  zts_sockaddr_in zt_addr{};

  zts_socklen_t zt_size = sizeof(zt_addr);

  auto zt_result = zts_bsd_accept(to_zts(ws), (zts_sockaddr*)&zt_addr, &zt_size);

  if (zt_result < 0)
  {
    zt_to_winsock_result(zt_result);
    return SOCKET_ERROR;
  }

  copy_address(zt_addr, name, namelen);

  if (zt_addr.sin_addr.S_addr)
  {
    get_fallback_broadcast_addresses().emplace(zt_addr.sin_addr.S_addr);
  }

  int non_blocking = 1;

  if (zts_bsd_ioctl(zt_result, ZTS_FIONBIO, &non_blocking) < 0)
  {
    zts_bsd_close(zt_result);
    return INVALID_SOCKET;
  }

  get_socket_handles().insert(zt_result);
  return from_zts(zt_result);
}

int __stdcall backend_connect(SOCKET ws, const sockaddr* name, int namelen)
{
  auto _ = zt_lock();
  get_log() << "siege_connect " << to_zts(ws);

  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  int sock_type = 0;
  zts_socklen_t sock_type_size = sizeof(sock_type);
  if (zts_bsd_getsockopt(to_zts(ws), ZTS_SOL_SOCKET, ZTS_SO_TYPE, &sock_type, &sock_type_size) != 0 || sock_type != SOCK_DGRAM)
  {
    wait_for_network_ready();
  }

  if (name)
  {
    auto address_and_size = copy_address(name, namelen);
    std::array<char, 32> requested{};
    get_log() << "zts_bsd_connect requested " << format_ipv4_port(requested, address_and_size.first.sin_addr.S_addr, address_and_size.first.sin_port) << "\n";
    rewrite_if_foreign(address_and_size.first);
    auto zt_result = zts_bsd_connect(to_zts(ws), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

    return zt_to_winsock_result(zt_result);
  }

  get_log() << "zts_bsd_connect with no address\n";

  auto zt_result = zts_bsd_connect(to_zts(ws), nullptr, namelen);

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_bind(SOCKET ws, const sockaddr* addr, int namelen)
{
  auto _ = zt_lock();
  get_log() << "siege_bind " << ws << std::endl;

  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  if (addr)
  {
    auto address_and_size = copy_address(addr, namelen);
    std::array<char, 32> requested{};
    auto requested_view = format_ipv4_port(requested, address_and_size.first.sin_addr.S_addr, address_and_size.first.sin_port);
    get_log() << "zts_bsd_bind requested " << requested_view << "\n";

    // computing a fallback in the rare case the IP comes from a real adapter
    if (auto& subnets = get_subnets(); address_and_size.first.sin_family == ZTS_AF_INET && address_and_size.first.sin_addr.S_addr != ZTS_INADDR_ANY && !subnets.empty())
    {
      auto ip = address_and_size.first.sin_addr.S_addr;
      bool on_network = false;
      for (auto [network, mask] : subnets)
      {
        if ((ip & mask) == network)
        {
          on_network = true;
          break;
        }
      }
      if (!on_network)
      {
        auto zt_id = get_network_id();

        if (zt_id)
        {
          char ipstr[ZTS_IP_MAX_STR_LEN] = { 0 };
          zts_addr_get_str(*zt_id, ZTS_AF_INET, ipstr, ZTS_IP_MAX_STR_LEN);

          auto zt_addr = imports->inet_addr(ipstr);
          address_and_size.first.sin_addr.S_addr = zt_addr == INADDR_NONE ? ZTS_INADDR_ANY : zt_addr;
        }
        else
        {
          address_and_size.first.sin_addr.S_addr = ZTS_INADDR_ANY;
        }

        std::array<char, 32> rewritten{};
        get_log() << "bind rewrite " << requested_view << " -> " << format_ipv4_port(rewritten, address_and_size.first.sin_addr.S_addr, address_and_size.first.sin_port) << "\n";
      }
    }

    auto zt_result = zts_bsd_bind(to_zts(ws), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

    return zt_to_winsock_result(zt_result);
  }

  get_log() << "zts_bsd_bind with no address\n";

  auto zt_result = zts_bsd_bind(to_zts(ws), nullptr, 0);

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept
{
  auto _ = zt_lock();
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  wait_for_network_ready();

  if (to)
  {
    auto address_and_size = copy_address(to, tolen);
    rewrite_if_foreign(address_and_size.first);

    auto dest = address_and_size.first.sin_addr.S_addr;
    const auto dest_port = address_and_size.first.sin_port;

    auto is_broadcast_address = [&]() {
      // limited broadcast, or a ZT subnet directed broadcast (e.g. 10.147.17.255)
      return dest == ZTS_IPADDR_BROADCAST || get_directed_broadcasts().contains(dest);
    };

    if (is_broadcast_address())
    {
      std::array<char, 32> broadcast_addr{};
      get_log() << "Trying to broadcast " << format_ipv4_port(broadcast_addr, address_and_size.first.sin_addr.S_addr, address_and_size.first.sin_port) << "\n";

      int sent = 0;
      int broadcast_result = 0;

      if (auto ips = get_fallback_broadcast_addresses(); !ips.empty())
      {
        for (auto ip : ips)
        {
          std::array<char, 32> fallback_addr{};
          get_log() << "Trying broadcast fallback to direct IP " << format_ipv4_port(fallback_addr, ip, dest_port) << ".\n";
          address_and_size.first.sin_addr.S_addr = ip;

          zts_fd_set set{};
          zts_timeval zero{ .tv_usec = 1000 };

          ZTS_FD_SET(to_zts(ws), &set);
          auto is_ready = zts_bsd_select(to_zts(ws) + 1, nullptr, &set, nullptr, &zero);

          if (is_ready && ZTS_FD_ISSET(to_zts(ws), &set))
          {
            get_log() << "Socket ready, doing broadcast " << format_ipv4_port(fallback_addr, ip, dest_port) << ".\n";
            broadcast_result = zts_bsd_sendto(to_zts(ws), buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

            if (broadcast_result > sent)
            {
              sent = broadcast_result;
            }
          }
          else
          {
            get_log() << "Socket not ready, trying next IP.\n";
          }
        }
      }

      if (zts_net_get_broadcast(*get_network_id()))
      {
        zts_fd_set set{};
        zts_timeval zero{ .tv_usec = 1000 };

        ZTS_FD_SET(to_zts(ws), &set);
        auto is_ready = zts_bsd_select(to_zts(ws) + 1, nullptr, &set, nullptr, &zero);

        if (is_ready && ZTS_FD_ISSET(to_zts(ws), &set))
        {
          address_and_size = copy_address(to, tolen);
          auto zt_result = zts_bsd_sendto(to_zts(ws), buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&address_and_size.first, address_and_size.second);
          if (zt_result > sent)
          {
            sent = zt_result;
          }
        }
      }

      if (sent > 0)
      {
        return sent;
      }
      // we always say we would block
      else
      {
        imports->WSASetLastError(WSAEWOULDBLOCK);
        return SOCKET_ERROR;
      }
    }

    auto zt_result = zts_bsd_sendto(to_zts(ws), buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

    if (zt_result < 0)
    {
      return zt_to_winsock_result(zt_result);
    }

    return zt_result;
  }

  auto zt_result = zts_bsd_send(to_zts(ws), buf, len, to_zt_msg_flags(flags));

  if (zt_result < 0)
  {
    return zt_to_winsock_result(zt_result);
  }

  return zt_result;
}

#ifdef SD_RECEIVE
static_assert(SD_RECEIVE == ZTS_SHUT_RD);
static_assert(SD_SEND == ZTS_SHUT_WR);
static_assert(SD_BOTH == ZTS_SHUT_RDWR);
#endif
int __stdcall backend_shutdown(SOCKET ws, int how)
{
  auto _ = zt_lock();
  get_log() << "siege_shutdown\n";
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_shutdown\n";

  auto zt_result = zts_bsd_shutdown(to_zts(ws), how);

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_closesocket(SOCKET ws)
{
  auto _ = zt_lock();
  get_log() << "siege_closesocket\n";
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_close\n";
  auto zt_result = zts_bsd_close(to_zts(ws));

  get_socket_handles().erase(to_zts(ws));

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_select([[maybe_unused]] int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout)
{
  fd_set original_read{};
  fd_set original_write{};
  fd_set original_except{};

  if (read)
  {
    original_read = *read;
  }

  if (write)
  {
    original_write = *write;
  }

  if (except)
  {
    original_except = *except;
  }

  auto transfer_set_zts = [](auto& source, auto& dest) {
    ZTS_FD_ZERO(&dest);
    for (auto i = 0u; i < source.fd_count; ++i)
    {
      auto zts = to_zts(source.fd_array[i]);
      if (!get_socket_handles().contains(zts))
      {
        get_log() << "Non Zero Tier handle detected " << zts;
        continue;
      }
      ZTS_FD_SET(zts, &dest);
    }
  };

  auto transfer_set_win32 = [](zts_fd_set& source, fd_set& dest) {
    fd_set temp{};
    for (auto i = 0u; i < dest.fd_count; ++i)
    {
      auto zts = to_zts(dest.fd_array[i]);
      if (!get_socket_handles().contains(zts))
      {
        get_log() << "Non Zero Tier handle detected " << zts;
        continue;
      }

      if (ZTS_FD_ISSET(zts, &source))
      {
        FD_SET(dest.fd_array[i], &temp);
      }
    }

    std::memcpy(&dest, &temp, sizeof(dest));
  };

  auto finish_ready = [&](zts_fd_set* final_read, zts_fd_set* final_write, zts_fd_set* final_except) {
    if (read)
    {
      *read = original_read;
      if (final_read)
      {
        transfer_set_win32(*final_read, *read);
      }
    }

    if (write)
    {
      *write = original_write;
      if (final_write)
      {
        transfer_set_win32(*final_write, *write);
      }
    }

    if (except)
    {
      *except = original_except;
      if (final_except)
      {
        transfer_set_win32(*final_except, *except);
      }
    }

    // Winsock: connect failure is except-only. lwIP reports failure as write+except.
    if (write && except && write->fd_count && except->fd_count)
    {
      fd_set kept_write{};
      for (u_int i = 0; i < write->fd_count; ++i)
      {
        if (!FD_ISSET(write->fd_array[i], except))
        {
          FD_SET(write->fd_array[i], &kept_write);
        }
      }
      *write = kept_write;
    }

    int count = 0;
    if (read)
    {
      count += static_cast<int>(read->fd_count);
    }
    if (write)
    {
      count += static_cast<int>(write->fd_count);
    }
    if (except)
    {
      count += static_cast<int>(except->fd_count);
    }
    return count;
  };

  constexpr auto slice = std::chrono::milliseconds{ 1 };
  std::optional<std::chrono::steady_clock::time_point> end;
  if (timeout)
  {
    end = std::chrono::steady_clock::now() + timeval_to_ms(*timeout);
  }

  auto lock = zt_lock();

  do
  {
    zts_fd_set zt_read{};
    zts_fd_set* final_read = nullptr;

    if (read && original_read.fd_count)
    {
      transfer_set_zts(original_read, zt_read);
      final_read = &zt_read;
    }

    zts_fd_set zt_write{};
    zts_fd_set* final_write = nullptr;

    if (write && original_write.fd_count)
    {
      transfer_set_zts(original_write, zt_write);
      final_write = &zt_write;
    }

    zts_fd_set zt_except{};
    zts_fd_set* final_except = nullptr;

    if (except && original_except.fd_count)
    {
      transfer_set_zts(original_except, zt_except);
      final_except = &zt_except;
    }

    auto wait = std::chrono::duration_cast<std::chrono::microseconds>(slice);
    if (end)
    {
      auto remaining = *end - std::chrono::steady_clock::now();
      if (remaining <= std::chrono::steady_clock::duration::zero())
      {
        wait = {};
      }
      else
      {
        wait = std::min(wait, std::chrono::duration_cast<std::chrono::microseconds>(remaining));
      }
    }

    zts_timeval slice_time{
      .tv_sec = static_cast<long>(std::chrono::duration_cast<std::chrono::seconds>(wait).count()),
      .tv_usec = static_cast<long>((wait % std::chrono::seconds{ 1 }).count())
    };

    auto count = zts_bsd_select(ZTS_FD_SETSIZE, final_read, final_write, final_except, &slice_time);

    if (count == ZTS_ERR_SOCKET || count == ZTS_ERR_SERVICE)
    {
      return zt_to_winsock_result(count);
    }

    if (count > 0)
    {
      return finish_ready(final_read, final_write, final_except);
    }

    if (end && std::chrono::steady_clock::now() >= *end)
    {
      break;
    }

    // Drop the backend lock between slices so other threads can use ZeroTier.
    lock.unlock();
    std::this_thread::yield();
    lock.lock();
  } while (!end || std::chrono::steady_clock::now() < *end);

  if (read)
  {
    FD_ZERO(read);
  }

  if (write)
  {
    FD_ZERO(write);
  }

  if (except)
  {
    FD_ZERO(except);
  }

  return 0;
}

int __stdcall backend___WSAFDIsSet(SOCKET ws, fd_set* set)
{
  auto _ = zt_lock();
  if (!get_socket_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  if (set)
  {
    zts_fd_set zt_set{};
    ZTS_FD_ZERO(&zt_set);
    for (auto i = 0; i < set->fd_count; ++i)
    {
      auto zts = to_zts(set->fd_array[i]);
      if (get_socket_handles().contains(zts))
      {
        ZTS_FD_SET(zts, &zt_set);
      }
    }
    return ZTS_FD_ISSET(to_zts(ws), &zt_set);
  }
  return 0;
}

hostent* __stdcall backend_gethostbyname(const char* name)
{
  auto _ = zt_lock();
  ensure_imports();
  if (name)
  {
    get_log() << "siege_gethostbyname: " << name;
  }
  else
  {
    get_log() << "siege_gethostbyname with no name \n";
  }

  if (!name)
  {
    return imports->gethostbyname(nullptr);
  }

  auto zt_id = get_network_id();

  if (!zt_id)
  {
    return nullptr;
  }

  wait_for_network_ready();

  auto get_internal_names = []() {
    std::set<std::string> internal_names;

    std::string temp_name;
    temp_name.resize(256);

    if (imports->gethostname(temp_name.data(), temp_name.size()) == 0)
    {
      if (auto end = temp_name.find('\0'); end != std::string::npos)
      {
        temp_name.resize(end);
      }

      internal_names.emplace(siege::platform::to_lower(temp_name));
    }

    temp_name.resize(256);
    DWORD temp_size = static_cast<DWORD>(temp_name.size());

    if (::GetComputerNameExA(ComputerNamePhysicalDnsHostname, temp_name.data(), &temp_size))
    {
      temp_name.resize(temp_size);
      internal_names.emplace(siege::platform::to_lower(temp_name));
    }

    return internal_names;
  };


  thread_local std::array<char, sizeof(in_addr)> raw_ip{};
  thread_local std::map<std::string, packed_hostent> host_cache;

  get_log() << "Calling zts_bsd_gethostbyname\n";
  auto result = zts_bsd_gethostbyname(name);

  if (result)
  {
    host_cache.emplace(name, from_zts(*result));
    get_log() << "Contains valid result\n";
    return &host_cache.at(name).host;
  }
  else
  {
    auto name_str = std::string{ name };

    auto internal_names = get_internal_names();
    hostent temp_host{
      .h_name = name_str.data(),
      .h_addrtype = AF_INET,
      .h_length = sizeof(in_addr),
    };

    if (internal_names.contains(siege::platform::to_lower(name_str)))
    {
      host_cache.emplace(name_str, temp_host);

      char ipstr[ZTS_IP_MAX_STR_LEN] = { 0 };
      zts_addr_get_str(*zt_id, ZTS_AF_INET, ipstr, ZTS_IP_MAX_STR_LEN);

      auto ip_int = imports->inet_addr(ipstr);
      std::memcpy(raw_ip.data(), &ip_int, raw_ip.size());
      auto& host = host_cache.at(name_str).host;
      host.h_addr_list[0] = raw_ip.data();
      host.h_addr_list[1] = nullptr;
      return &host_cache.at(name_str).host;
    }
  }
  return nullptr;
}
}

int zt_to_winsock_result(int code)
{
  switch (code)
  {
  case ZTS_ERR_OK:
    return 0;
  case ZTS_ERR_SOCKET: {
    imports->WSASetLastError(zt_to_winsock_error(get_zts_errno()));
    return SOCKET_ERROR;
  }
  case ZTS_ERR_SERVICE: {
    imports->WSASetLastError(WSANOTINITIALISED);
    return SOCKET_ERROR;
  }
  case ZTS_ERR_ARG: {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }
  case ZTS_ERR_NO_RESULT: {
    return SOCKET_ERROR;
  }
  case ZTS_ERR_GENERAL: {
    imports->WSASetLastError(WSASYSNOTREADY);
    return SOCKET_ERROR;
  }
  }

  return code >= 0 ? 0 : SOCKET_ERROR;
}

std::shared_ptr<char> get_shared_current_ip_address_storage()
{
  try
  {
    if (auto env_size = ::GetEnvironmentVariableW(L"SIEGE_WSOCK_CURRENT_IP_GLOBAL_HANDLE", nullptr, 0); env_size >= 1)
    {
      std::wstring raw_handle(env_size - 1, '\0');
      ::GetEnvironmentVariableW(L"SIEGE_WSOCK_CURRENT_IP_GLOBAL_HANDLE", raw_handle.data(), raw_handle.size() + 1);

      get_log() << "Getting SIEGE_WSOCK_CURRENT_IP_GLOBAL_HANDLE\n";

      if (raw_handle.empty())
      {
        return nullptr;
      }

      HANDLE global = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, raw_handle.data());

      if (!global)
      {
        return nullptr;
      }
      get_log() << "HANDLE is " << (std::size_t)global;


      auto result = ::MapViewOfFile(global, FILE_MAP_ALL_ACCESS, 0, 0, ZTS_IP_MAX_STR_LEN);

      if (!result)
      {
        get_log() << "Could not map file handle";
        return nullptr;
      }

      return std::shared_ptr<char>((char*)result, [global](char* data) {
        ::UnmapViewOfFile(data);
        ::CloseHandle(global);
      });
    }
    return nullptr;
  }
  catch (...)
  {
    return nullptr;
  }
}

std::optional<in_addr> get_fallback_broadcast_ip_v4()
{
  static std::optional<in_addr> result = []() -> std::optional<in_addr> {
    get_log() << "get_fallback_broadcast_ip_v4\n";


    if (auto env_size = ::GetEnvironmentVariableA("SIEGE_WSOCK_FALLBACK_BROADCAST_IP_V4", nullptr, 0); env_size >= 1)
    {
      std::string network_ip(env_size - 1, '\0');
      ::GetEnvironmentVariableA("SIEGE_WSOCK_FALLBACK_BROADCAST_IP_V4", network_ip.data(), network_ip.size() + 1);

      get_log() << "Zero Tier fallback broadcast IP is " << network_ip;
      in_addr result{};
      result.S_un.S_addr = imports->inet_addr(network_ip.c_str());

      if (!result.S_un.S_addr)
      {
        return std::nullopt;
      }

      return result;
    }

    get_log() << "No zero tier fallback broadcast IP\n";
    return std::nullopt;
  }();

  return result;
}

bool fallback_broadcast_sorter(std::uint32_t a, std::uint32_t b)
{
  if (a == b)
  {
    return false;
  }

  if (auto primary = get_fallback_broadcast_ip_v4())
  {
    if (a == primary->S_un.S_addr)
    {
      return true;
    }

    if (b == primary->S_un.S_addr)
    {
      return false;
    }
  }

  return a < b;
}

std::set<std::uint32_t, decltype(fallback_broadcast_sorter)*>& get_fallback_broadcast_addresses()
{
  // static would be nice, but most games have a dedicated thread or the main thread for
  // networking, and we will almost always have our default fallback ip anyway.
  thread_local std::set<std::uint32_t, decltype(fallback_broadcast_sorter)*> addresses = [] {
    std::set<std::uint32_t, decltype(fallback_broadcast_sorter)*> initial{ fallback_broadcast_sorter };

    auto env_addr = get_fallback_broadcast_ip_v4();

    if (env_addr)
    {
      initial.emplace(env_addr->S_un.S_addr);
    }

    auto zt_id = get_network_id();

    if (!zt_id)
    {
      return initial;
    }

    char ipstr[ZTS_IP_MAX_STR_LEN] = { 0 };

    wait_for_network_ready();
    if (zts_addr_get_str(*zt_id, ZTS_AF_INET, ipstr, ZTS_IP_MAX_STR_LEN) < 0)
    {
      return initial;
    }

    auto ip_int = imports->inet_addr(ipstr);

    if (ip_int)
    {
      initial.emplace(ip_int);
    }

    return initial;
  }();

  return addresses;
}

// ZT stashes the subnet prefix length in the assigned address' port field
// (network byte order). network → mask, both in network byte order.
const std::map<std::uint32_t, std::uint32_t>& get_subnets()
{
  static std::map<std::uint32_t, std::uint32_t> result = []() -> std::map<std::uint32_t, std::uint32_t> {
    std::map<std::uint32_t, std::uint32_t> subnets;

    auto net_id = get_network_id();
    if (!net_id)
    {
      return subnets;
    }

    std::array<zts_sockaddr_storage, ZTS_MAX_ASSIGNED_ADDRESSES> addresses{};
    unsigned int count = ZTS_MAX_ASSIGNED_ADDRESSES;

    wait_for_network_ready();
    if (zts_addr_get_all(*net_id, addresses.data(), &count) != ZTS_ERR_OK)
    {
      return subnets;
    }

    for (auto i = 0u; i < count; ++i)
    {
      auto* in4 = reinterpret_cast<zts_sockaddr_in*>(&addresses[i]);

      if (in4->sin_family != ZTS_AF_INET)
      {
        continue;
      }

      auto prefix = imports->ntohs(in4->sin_port);

      // port empty when only the address is available; /24 matches the usual ZT assignment
      if (prefix == 0 || prefix > 32)
      {
        prefix = 24;
      }

      std::uint32_t host_mask = prefix == 32 ? 0u : ((1u << (32 - prefix)) - 1u);
      auto mask = imports->htonl(~host_mask);
      subnets.emplace(in4->sin_addr.S_addr & mask, mask);
    }

    get_log() << "Computed " << subnets.size() << " subnet(s)\n";
    for (auto [network, mask] : subnets)
    {
      std::array<char, 16> network_str{};
      std::array<char, 16> mask_str{};
      get_log() << "  subnet " << format_ipv4(network_str, network) << " mask " << format_ipv4(mask_str, mask) << "\n";
    }
    return subnets;
  }();

  return result;
}

const std::set<std::uint32_t>& get_directed_broadcasts()
{
  static std::set<std::uint32_t> result = []() -> std::set<std::uint32_t> {
    std::set<std::uint32_t> broadcasts;

    for (auto [network, mask] : get_subnets())
    {
      broadcasts.emplace(network | ~mask);
    }

    get_log() << "Computed " << broadcasts.size() << " directed broadcast address(es)\n";
    for (auto broadcast : broadcasts)
    {
      std::array<char, 16> broadcast_str{};
      get_log() << "  directed broadcast " << format_ipv4(broadcast_str, broadcast) << "\n";
    }
    return broadcasts;
  }();

  return result;
}

// Games that probe a real NIC IP (bypassing our stack) get remapped to the
// configured ZT fallback host. Port is left alone.
void rewrite_if_foreign(zts_sockaddr_in& addr)
{
  if (addr.sin_family != ZTS_AF_INET)
  {
    return;
  }

  auto ip = addr.sin_addr.S_addr;

  if (ip == ZTS_IPADDR_BROADCAST || get_directed_broadcasts().contains(ip))
  {
    return;
  }

  auto& subnets = get_subnets();

  if (subnets.empty())
  {
    return;
  }

  for (auto [network, mask] : subnets)
  {
    if ((ip & mask) == network)
    {
      return;
    }
  }

  auto fallback = get_fallback_broadcast_ip_v4();

  if (!fallback)
  {
    return;
  }

  std::array<char, 32> from{};
  auto from_view = format_ipv4_port(from, addr.sin_addr.S_addr, addr.sin_port);
  addr.sin_addr.S_addr = fallback->S_un.S_addr;
  std::array<char, 32> to{};
  get_log() << "rewrote foreign address " << from_view << " -> " << format_ipv4_port(to, addr.sin_addr.S_addr, addr.sin_port) << "\n";
}

std::optional<std::uint64_t> get_network_id()
{
  static std::optional<std::uint64_t> result = []() -> std::optional<std::uint64_t> {
    try
    {
      get_log() << "get_network_id\n";


      if (auto env_size = ::GetEnvironmentVariableA("SIEGE_WSOCK_NETWORK_ID", nullptr, 0); env_size >= 1)
      {
        std::string network_id(env_size - 1, '\0');
        ::GetEnvironmentVariableA("SIEGE_WSOCK_NETWORK_ID", network_id.data(), network_id.size() + 1);

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
  }();

  return result;
}

std::optional<std::string> get_peer_id_and_public_key()
{
  static std::optional<std::string> result = []() -> std::optional<std::string> {
    get_log() << "get_peer_id_and_public_key\n";

    if (auto env_size = ::GetEnvironmentVariableA("SIEGE_WSOCK_NODE_AUTH_KEY", nullptr, 0); env_size >= 1)
    {
      std::string peer_id(env_size - 1, '\0');

      ::GetEnvironmentVariableA("SIEGE_WSOCK_NODE_AUTH_KEY", peer_id.data(), peer_id.size() + 1);


      peer_id.resize(ZTS_ID_STR_BUF_LEN);

      if (zts_id_pair_is_valid(peer_id.data(), (unsigned int)peer_id.size()))
      {
        get_log() << "Zero Tier peer id and public key retrieved\n";
        return peer_id;
      }
      else
      {
        get_log() << "Zero Tier peer id and public key retrieved, but invalid\n";
        return std::nullopt;
      }
    }

    get_log() << "No zero tier peer ID and public key\n";
    return std::nullopt;
  }();

  return result;
}

int to_zt_msg_flags(int flags)
{
  auto zt_flags = 0;

  if (flags & MSG_PEEK)
  {
    zt_flags |= ZTS_MSG_PEEK;
  }

  if (flags & MSG_OOB)
  {
    zt_flags |= ZTS_MSG_OOB;
  }

  return zt_flags;
}

int get_zts_errno()
{
  return zts_errno;
}

#ifdef WSA_INVALID_HANDLE
constexpr int wsa_invalid_handle = WSA_INVALID_HANDLE;
#else
constexpr int wsa_invalid_handle = WSAEBADF;
#endif

#ifdef WSA_NOT_ENOUGH_MEMORY
constexpr int wsa_not_enough_memory = WSA_NOT_ENOUGH_MEMORY;
#else
constexpr int wsa_not_enough_memory = WSA_QOS_TRAFFIC_CTRL_ERROR;
#endif

#ifdef WSA_INVALID_PARAMETER
constexpr int wsa_invalid_parameter = WSA_INVALID_PARAMETER;
#else
constexpr int wsa_invalid_parameter = WSAEINVAL;
#endif

constexpr bool zts_errno_uses_crt =
#ifdef SIEGE_ZTS_USE_CRT_ERRNO
  true
#else
  false
#endif
  ;

int zt_to_winsock_error(int error)
{
  if (error == 0)
  {
    return 0;
  }

  if constexpr (zts_errno_uses_crt)
  {
    constexpr std::pair<int, int> crt_to_wsa[] = {
      { EPERM, WSAEACCES },
      { ENOENT, wsa_invalid_handle },
      { ESRCH, wsa_invalid_handle },
      { EINTR, WSAEINTR },
      { EIO, WSAEINPROGRESS },
      { ENXIO, WSAEFAULT },
      { EBADF, WSAEBADF },
      { EWOULDBLOCK, WSAEWOULDBLOCK },
      { EAGAIN, WSAEWOULDBLOCK },
      { ENOMEM, wsa_not_enough_memory },
      { EACCES, WSAEACCES },
      { EFAULT, WSAEFAULT },
      { EBUSY, WSAEACCES },
      { EEXIST, WSAEACCES },
      { ENODEV, WSAEACCES },
      { EINVAL, WSAEINVAL },
      { ENFILE, WSAEMFILE },
      { EMFILE, WSAEMFILE },
      { ENOSYS, WSAEACCES },
      { ENOTSOCK, WSAENOTSOCK },
      { EDESTADDRREQ, WSAEDESTADDRREQ },
      { EMSGSIZE, WSAEMSGSIZE },
      { EPROTOTYPE, WSAEPROTOTYPE },
      { ENOPROTOOPT, WSAENOPROTOOPT },
      { EPROTONOSUPPORT, WSAEPROTONOSUPPORT },
      { EOPNOTSUPP, WSAEOPNOTSUPP },
      { EAFNOSUPPORT, WSAEAFNOSUPPORT },
      { EADDRINUSE, WSAEADDRINUSE },
      { EADDRNOTAVAIL, WSAEADDRNOTAVAIL },
      { ENETDOWN, WSAENETDOWN },
      { ENETUNREACH, WSAENETUNREACH },
      { ECONNABORTED, WSAECONNABORTED },
      { ECONNRESET, WSAECONNRESET },
      { ENOBUFS, WSAENOBUFS },
      { EISCONN, WSAEISCONN },
      { ENOTCONN, WSAENOTCONN },
      { ETIMEDOUT, WSAETIMEDOUT },
      { ECONNREFUSED, WSAECONNREFUSED },
      { EHOSTUNREACH, WSAEHOSTUNREACH },
      { EALREADY, WSAEALREADY },
      // on windows, WSAEINPROGRESS means something else
      // (it's about blocking hooks and service provider callbacks).
      // WOULDBLOCK is dual-purpose in wsock.
      { EINPROGRESS, WSAEWOULDBLOCK },
    };

    for (auto [from, to] : crt_to_wsa)
    {
      if (error == from)
      {
        return to;
      }
    }

    return wsa_invalid_parameter;
  }
  else
  {
    constexpr std::pair<int, int> zts_to_wsa[] = {
      { ZTS_EPERM, WSAEACCES },
      { ZTS_ENOENT, wsa_invalid_handle },
      { ZTS_ESRCH, wsa_invalid_handle },
      { ZTS_EINTR, WSAEINTR },
      { ZTS_EIO, WSAEINPROGRESS },
      { ZTS_ENXIO, WSAEFAULT },
      { ZTS_EBADF, WSAEBADF },
      { ZTS_EWOULDBLOCK, WSAEWOULDBLOCK },
      { ZTS_ENOMEM, wsa_not_enough_memory },
      { ZTS_EACCES, WSAEACCES },
      { ZTS_EFAULT, WSAEFAULT },
      { ZTS_EBUSY, WSAEACCES },
      { ZTS_EEXIST, WSAEACCES },
      { ZTS_ENODEV, WSAEACCES },
      { ZTS_EINVAL, WSAEINVAL },
      { ZTS_ENFILE, WSAEMFILE },
      { ZTS_EMFILE, WSAEMFILE },
      { ZTS_ENOSYS, WSAEACCES },
      { ZTS_ENOTSOCK, WSAENOTSOCK },
      { ZTS_EDESTADDRREQ, WSAEDESTADDRREQ },
      { ZTS_EMSGSIZE, WSAEMSGSIZE },
      { ZTS_EPROTOTYPE, WSAEPROTOTYPE },
      { ZTS_ENOPROTOOPT, WSAENOPROTOOPT },
      { ZTS_EPROTONOSUPPORT, WSAEPROTONOSUPPORT },
      { ZTS_ESOCKTNOSUPPORT, WSAESOCKTNOSUPPORT },
      { ZTS_EOPNOTSUPP, WSAEOPNOTSUPP },
      { ZTS_EPFNOSUPPORT, WSAEPFNOSUPPORT },
      { ZTS_EAFNOSUPPORT, WSAEAFNOSUPPORT },
      { ZTS_EADDRINUSE, WSAEADDRINUSE },
      { ZTS_EADDRNOTAVAIL, WSAEADDRNOTAVAIL },
      { ZTS_ENETDOWN, WSAENETDOWN },
      { ZTS_ENETUNREACH, WSAENETUNREACH },
      { ZTS_ECONNABORTED, WSAECONNABORTED },
      { ZTS_ECONNRESET, WSAECONNRESET },
      { ZTS_ENOBUFS, WSAENOBUFS },
      { ZTS_EISCONN, WSAEISCONN },
      { ZTS_ENOTCONN, WSAENOTCONN },
      { ZTS_ETIMEDOUT, WSAETIMEDOUT },
      { ZTS_ECONNREFUSED, WSAECONNREFUSED },
      { ZTS_EHOSTUNREACH, WSAEHOSTUNREACH },
      { ZTS_EALREADY, WSAEALREADY },
      // on windows, WSAEINPROGRESS means something else
      // (it's about blocking hooks and service provider callbacks).
      // WOULDBLOCK is dual-purpose in wsock.
      { ZTS_EINPROGRESS, WSAEWOULDBLOCK },
    };

    for (auto [from, to] : zts_to_wsa)
    {
      if (error == from)
      {
        return to;
      }
    }

    return wsa_invalid_parameter;
  }
}

SOCKET from_zts(int socket)
{
  return socket + 1000;
}

int to_zts(SOCKET socket)
{
  return (int)socket - 1000;
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

std::string_view format_ipv4(std::array<char, 16>& out, std::uint32_t addr_nbo)
{
  in_addr address{ .S_un = { .S_addr = addr_nbo } };
  if (!imports->inet_ntop(AF_INET, &address, out.data(), out.size()))
  {
    out[0] = '?';
    out[1] = '\0';
  }
  return out.data();
}

std::string_view format_ipv4_port(std::array<char, 32>& out, std::uint32_t addr_nbo, std::uint16_t port_nbo)
{
  std::array<char, 16> ip{};
  auto ip_view = format_ipv4(ip, addr_nbo);
  auto written = std::snprintf(out.data(), out.size(), "%.*s:%u", static_cast<int>(ip_view.size()), ip_view.data(), static_cast<unsigned>(imports->ntohs(port_nbo)));
  if (written < 0)
  {
    out[0] = '?';
    out[1] = '\0';
    return out.data();
  }
  return { out.data(), static_cast<std::size_t>(written) };
}

static_assert(sizeof(sockaddr) >= sizeof(sockaddr_in));
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

void copy_address(zts_sockaddr_in addr, sockaddr* name, int* length)
{
  if (!name)
  {
    return;
  }

  if (!length || (length && *length <= 0))
  {
    return;
  }

  auto temp = from_zts(addr);

  auto size = sizeof(temp);

  if (length)
  {
    size = *length > sizeof(temp) ? sizeof(temp) : *length;
    *length = size;
  }

  std::memcpy(name, &temp, size);
}

std::pair<zts_sockaddr_in, zts_socklen_t> copy_address(const sockaddr* name, int length)
{
  if (!name)
  {
    return {};
  }

  if (length <= 0)
  {
    return {};
  }

  sockaddr_in temp{};
  zts_socklen_t size = length > sizeof(temp) ? sizeof(temp) : length;

  std::memcpy(&temp, name, size);
  return std::make_pair(to_zts(temp), sizeof(zts_sockaddr_in));
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

bool& get_node_online_status()
{
  static bool node_is_online = false;
  return node_is_online;
}

void wait_for_network_ready()
{
  static bool waited = false;

  if (waited)
  {
    return;
  }

  waited = true;

  if (!get_node_online_status())
  {
    return;
  }

  auto network_id = get_network_id();

  if (!network_id)
  {
    return;
  }

  for (auto i = 0; i < 500; ++i)
  {
    if (zts_net_transport_is_ready(*network_id))
    {
      get_log() << "Joined network\n";

      if (auto storage = get_shared_current_ip_address_storage(); storage)
      {
        zts_addr_get_str(*network_id, ZTS_AF_INET, storage.get(), ZTS_IP_MAX_STR_LEN);
      }

      return;
    }
    zts_util_delay(100);
  }

  get_log() << "Network transport not ready before timeout\n";
}