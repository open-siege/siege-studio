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

socket_handle_info& get_zero_tier_handles()
{
  static socket_handle_info info{};
  return info;
}

std::optional<std::uint64_t> get_zero_tier_network_id();
std::optional<std::string> get_zero_tier_peer_id_and_public_key();
std::shared_ptr<char> get_shared_current_ip_address_storage();
int zt_to_winsock_error(int);
int zt_to_winsock_result(int code);
std::set<std::uint32_t>& get_fallback_broadcast_addresses();

int to_zt_msg_flags(int flags);
zts_sockaddr_in to_zts(sockaddr_in addr);
int get_zts_errno();
bool& get_node_online_status();
sockaddr_in from_zts(zts_sockaddr_in zt_addr);
hostent from_zts(zts_hostent zt_host);
void copy_address(zts_sockaddr_in addr, sockaddr* name, int* length);
std::pair<zts_sockaddr_in, zts_socklen_t> copy_address(const sockaddr* name, int length);

SOCKET from_zts(int);
int to_zts(SOCKET);

extern "C" {
int __stdcall backend_WSAStartup(WORD version, LPWSADATA data)
{
  ensure_imports();
  get_log("backend.zero-tier") << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version);
  auto result = imports->WSAStartup(version, data);

  if (auto network_id = get_zero_tier_network_id())
  {
    if (result == 0)
    {
      imports->WSACleanup();
    }

    get_log() << "Zero Tier library available and network is set\n";

    if (!get_node_online_status())
    {
      if (auto node_id_and_key = get_zero_tier_peer_id_and_public_key(); node_id_and_key)
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
      bool is_connected = false;

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

      zts_net_join(*network_id);

      get_log() << "Joining network\n";
      for (auto i = 0; i < 500; ++i)
      {
        if (zts_net_transport_is_ready(*network_id))
        {
          get_log() << "Joined network\n";
          is_connected = true;
          break;
        }
        zts_util_delay(100);
      }

      get_node_online_status() = is_online && is_connected;

      if (is_online && !is_connected)
      {
        get_log() << "Node is online but could not join network. Stopping node.\n";
        zts_node_stop();
        ::ExitProcess(-1);
      }
      else if (!is_online && !is_connected)
      {
        get_log() << "Node could not be started and could not join network.\n";
        ::ExitProcess(-1);
      }

      if (auto storage = get_shared_current_ip_address_storage(); storage)
      {
        zts_addr_get_str(*get_zero_tier_network_id(), ZTS_AF_INET, storage.get(), ZTS_IP_MAX_STR_LEN);
      }
    }

    return 0;
  }

  get_log().flush();
  return result;
}

int __stdcall backend_WSACleanup()
{
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

    if (zts_bsd_ioctl(socket, ZTS_FIONBIO, &non_blocking) < -1)
    {
      zts_bsd_close(socket);
      return INVALID_SOCKET;
    }

    get_log() << "Created zero tier socket successfully (" << socket << ")";
    get_zero_tier_handles().insert(socket);


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
static_assert(SO_ACCEPTCONN == ZTS_SO_ACCEPTCONN);
static_assert(SOL_SOCKET != ZTS_SOL_SOCKET);
int __stdcall backend_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen)
{
  get_log() << "siege_setsockopt: " << ws << " " << optname;
  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_setsockopt, socket: " << to_zts(ws) << " level: " << level_to_string(level) << " optname: " << option_to_string(optname);

  if (level != SOL_SOCKET)
  {
    get_log() << "Potentially unsupported socket level " << level;
  }

  BOOL some_flag = -1;

  zts_socklen_t size = sizeof(some_flag);


  static std::set<int> optnames = { SO_RCVTIMEO, SO_SNDTIMEO, SO_SNDBUF, SO_RCVBUF };

  if (level == SOL_SOCKET)
  {
    level = ZTS_SOL_SOCKET;
  }
  else
  {
    get_log() << "Setting a regular socket setting " << optname;
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
  get_log() << "siege_getsockopt" << to_zts(ws) << " " << optname;
  if (!get_zero_tier_handles().contains(to_zts(ws)))
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


  get_log() << "zts_bsd_setsockopt, level: " << level << " optname: " << optname;

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

  if (optval && optlen && *optlen == sizeof(DWORD) && (level == SOL_SOCKET || level == ZTS_SOL_SOCKET) && (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO))
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
  else
  {
    zt_result = zts_bsd_getsockopt(to_zts(ws), level, optname, &optval, &size);
  }

  if (optlen)
  {
    *optlen = size;
  }

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept
{
  imports->WSASetLastError(0);
  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

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
    get_log() << "zts_bsd_recvfrom had an error\n";

    return zt_to_winsock_result(zt_result);
  }

  if (zt_addr.sin_addr.S_addr)
  {
    get_fallback_broadcast_addresses().emplace(zt_addr.sin_addr.S_addr);
  }


  get_log() << "zts_bsd_recvfrom successful\n";

  copy_address(zt_addr, from, fromLen);

  return (int)zt_result;
}
}

int __stdcall backend_getsockname(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getsockname\n";
  if (!get_zero_tier_handles().contains(to_zts(ws)))
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

    copy_address(zt_addr, name, length);

    return zt_to_winsock_result(zt_result);
  }

  auto zt_result = zts_bsd_getsockname(to_zts(ws), nullptr, nullptr);
  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_getpeername(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getpeername\n";
  if (!get_zero_tier_handles().contains(to_zts(ws)))
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
  get_log() << "siege_ioctlsocket, cmd: " << ioctl_cmd_to_string(cmd);
  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_ioctl\n";

  auto zt_result = zts_bsd_ioctl(to_zts(ws), cmd, argp);

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_listen(SOCKET ws, int backlog)
{
  get_log() << "siege_listen\n";

  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  if (backlog == SOMAXCONN)
  {
    backlog = ZTS_FD_SETSIZE;
  }
  auto zt_result = zts_bsd_listen(to_zts(ws), backlog);
  return zt_to_winsock_result(zt_result);
}

SOCKET __stdcall backend_accept(SOCKET ws, sockaddr* name, int* namelen)
{
  get_log() << "siege_accept\n";
  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_accept\n";

  zts_sockaddr_in zt_addr{};

  zts_socklen_t zt_size = sizeof(zt_addr);

  auto zt_result = zts_bsd_accept(to_zts(ws), (zts_sockaddr*)&zt_addr, &zt_size);

  if (zt_result >= 0)
  {
    copy_address(zt_addr, name, namelen);

    if (zt_addr.sin_addr.S_addr)
    {
      get_fallback_broadcast_addresses().emplace(zt_addr.sin_addr.S_addr);
    }

    get_zero_tier_handles().insert(zt_result);
    return from_zts(zt_result);
  }

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_connect(SOCKET ws, const sockaddr* name, int namelen)
{
  get_log() << "siege_connect " << to_zts(ws);

  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_connect\n";

  if (name)
  {
    auto address_and_size = copy_address(name, namelen);
    auto zt_result = zts_bsd_connect(to_zts(ws), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

    return zt_to_winsock_result(zt_result);
  }

  auto zt_result = zts_bsd_connect(to_zts(ws), nullptr, namelen);

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_bind(SOCKET ws, const sockaddr* addr, int namelen)
{
  get_log() << "siege_bind " << ws << std::endl;

  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_bind\n";

  if (addr)
  {
    auto address_and_size = copy_address(addr, namelen);

    auto zt_result = zts_bsd_bind(to_zts(ws), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

    return zt_to_winsock_result(zt_result);
  }

  auto zt_result = zts_bsd_bind(to_zts(ws), nullptr, 0);

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept
{
  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_sendto\n";

  if (to)
  {
    auto address_and_size = copy_address(to, tolen);

    auto is_broadcast_address = [&]() {
      in_addr addr{};
      addr.S_un.S_addr = address_and_size.first.sin_addr.S_addr;

      return addr.S_un.S_un_b.s_b1 == 255 || addr.S_un.S_un_b.s_b2 == 255 || addr.S_un.S_un_b.s_b3 == 255 || addr.S_un.S_un_b.s_b4 == 255;
    };

    if (address_and_size.first.sin_addr.S_addr == ZTS_IPADDR_BROADCAST || is_broadcast_address())
    {
      get_log() << "Trying to broadcast\n";

      auto zt_result = zts_bsd_sendto(to_zts(ws), buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&address_and_size.first, address_and_size.second);
      int broadcast_result = 0;

      int sent = 0;
      if (auto& ips = get_fallback_broadcast_addresses(); !ips.empty() && zt_result < 0)
      {
        auto index = 0;

        for (auto ip : ips)
        {
          get_log() << "Could not broadcast. Trying direct IP " << index++ << ".\n";
          address_and_size.first.sin_addr.S_addr = ip;
          broadcast_result = zts_bsd_sendto(to_zts(ws), buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

          if (broadcast_result > sent)
          {
            sent = broadcast_result;
          }
        }
      }

      if (sent != len)
      {
        get_log() << "Still could not broadcast.\n";
        return zt_to_winsock_result(zt_result);
      }

      return sent;
    }

    get_log() << "Doing a regular sendto\n";
    auto zt_result = zts_bsd_sendto(to_zts(ws), buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

    if (zt_result < 0)
    {
      return zt_to_winsock_result(zt_result);
    }

    return zt_result;
  }

  get_log() << "Doing a sendto of a supposedly already bound socket\n";
  auto zt_result = zts_bsd_sendto(to_zts(ws), buf, len, to_zt_msg_flags(flags), nullptr, 0);

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
  get_log() << "siege_shutdown\n";
  if (!get_zero_tier_handles().contains(to_zts(ws)))
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
  get_log() << "siege_closesocket\n";
  if (!get_zero_tier_handles().contains(to_zts(ws)))
  {
    get_log() << "Non zero tier socket passed in" << std::endl;
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_log() << "zts_bsd_close\n";
  auto zt_result = zts_bsd_close(to_zts(ws));

  get_zero_tier_handles().erase(to_zts(ws));

  return zt_to_winsock_result(zt_result);
}

int __stdcall backend_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout)
{
  get_log() << "siege_select\n";
  zts_fd_set zt_read{};
  zts_fd_set* final_read = nullptr;

  auto transfer_set = [](auto& source, auto& dest) {
    ZTS_FD_ZERO(&dest);
    for (auto i = 0; i < source.fd_count; ++i)
    {
      auto zts = to_zts(source.fd_array[i]);
      if (get_zero_tier_handles().contains(zts))
      {
        ZTS_FD_SET(zts, &dest);
      }
      else
      {
        get_log() << "Non Zero Tier handle detected " << zts;
      }
    }
  };

  if (read && read->fd_count)
  {
    transfer_set(*read, zt_read);
    final_read = &zt_read;
  }

  zts_fd_set zt_write{};
  zts_fd_set* final_write = nullptr;

  if (write && write->fd_count)
  {
    transfer_set(*write, zt_write);
    final_write = &zt_write;
  }

  zts_fd_set zt_except{};
  zts_fd_set* final_except = nullptr;

  if (except && except->fd_count)
  {
    transfer_set(*except, zt_except);
    final_except = &zt_except;
  }

  zts_timeval timeval{};

  zts_timeval* final_timeval = nullptr;

  if (timeout)
  {
    timeval.tv_sec = timeout->tv_sec;
    timeval.tv_usec = timeout->tv_usec;

    final_timeval = &timeval;
  }

  auto count = zts_bsd_select(ZTS_FD_SETSIZE, final_read, final_write, final_except, final_timeval);

  if (count == ZTS_ERR_SOCKET || count == ZTS_ERR_SERVICE)
  {
    return zt_to_winsock_result(count);
  }
  return count;
}

int __stdcall backend___WSAFDIsSet(SOCKET ws, fd_set* set)
{
  if (!get_zero_tier_handles().contains(to_zts(ws)))
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
      if (get_zero_tier_handles().contains(zts))
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
  ensure_imports();
  if (name)
  {
    get_log() << "siege_gethostbyname: " << name;
  }
  else
  {
    get_log() << "siege_gethostbyname with no name \n";
  }

  if (name && get_zero_tier_network_id() && get_node_online_status())
  {
    get_log() << "Calling zts_bsd_gethostbyname\n";
    auto result = zts_bsd_gethostbyname(name);
    static std::map<std::string, hostent> host_cache;

    if (result)
    {
      host_cache[name] = from_zts(*result);
      get_log() << "Contains valid result\n";
      return &host_cache[name];
    }
    else if (name)
    {
      auto result = imports->gethostbyname(name);

      if (!result)
      {
        return nullptr;
      }

      std::array<char, 255> temp{};

      if (imports->gethostname(temp.data(), temp.size()) == 0 && std::string_view(name) == temp.data())
      {
        host_cache[name] = *result;

        if (host_cache[name].h_addr_list[0])
        {
          char ipstr[ZTS_IP_MAX_STR_LEN] = { 0 };
          zts_addr_get_str(*get_zero_tier_network_id(), ZTS_AF_INET, ipstr, ZTS_IP_MAX_STR_LEN);

          static std::array<char, sizeof(in_addr)> raw_ip{};
          auto ip_int = imports->inet_addr(ipstr);
          std::memcpy(raw_ip.data(), &ip_int, raw_ip.size());
          host_cache[name].h_length = 1;
          host_cache[name].h_addr_list[0] = raw_ip.data();
          host_cache[name].h_addr_list[1] = nullptr;
        }
        return &host_cache[name];
      }

      return nullptr;
    }
  }

  return imports->gethostbyname(name);
}

int zt_to_winsock_result(int code)
{
  switch (code)
  {
  case ZTS_ERR_OK:
    return 0;
  case ZTS_ERR_SOCKET: {
    get_log() << "Received ZTS_ERR_SOCKET\n";
    imports->WSASetLastError(zt_to_winsock_error(get_zts_errno()));
    return SOCKET_ERROR;
  }
  case ZTS_ERR_SERVICE: {
    get_log() << "Received ZTS_ERR_SERVICE\n";
    imports->WSASetLastError(WSANOTINITIALISED);
    return SOCKET_ERROR;
  }
  case ZTS_ERR_ARG: {
    get_log() << "Received ZTS_ERR_ARG\n";
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }
  case ZTS_ERR_NO_RESULT: {
    get_log() << "Received ZTS_ERR_NO_RESULT\n";
    return 0;
  }
  case ZTS_ERR_GENERAL: {
    get_log() << "Received ZTS_ERR_GENERAL\n";
    imports->WSASetLastError(WSASYSNOTREADY);
    return 0;
  }
  }

  return 0;
}

std::shared_ptr<char> get_shared_current_ip_address_storage()
{
  try
  {
    if (auto env_size = ::GetEnvironmentVariableW(L"ZERO_TIER_CURRENT_IP_GLOBAL_HANDLE", nullptr, 0); env_size >= 1)
    {
      std::wstring raw_handle(env_size - 1, '\0');
      ::GetEnvironmentVariableW(L"ZERO_TIER_CURRENT_IP_GLOBAL_HANDLE", raw_handle.data(), raw_handle.size() + 1);

      get_log() << "Getting ZERO_TIER_CURRENT_IP_GLOBAL_HANDLE\n";

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

std::optional<in_addr> get_zero_tier_fallback_broadcast_ip_v4()
{
  static std::optional<in_addr> result = []() -> std::optional<in_addr> {
    get_log() << "get_zero_tier_fallback_broadcast_ip_v4\n";


    if (auto env_size = ::GetEnvironmentVariableA("ZERO_TIER_FALLBACK_BROADCAST_IP_V4", nullptr, 0); env_size >= 1)
    {
      std::string network_ip(env_size - 1, '\0');
      ::GetEnvironmentVariableA("ZERO_TIER_FALLBACK_BROADCAST_IP_V4", network_ip.data(), network_ip.size() + 1);

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

std::set<std::uint32_t>& get_fallback_broadcast_addresses()
{
  static std::set<std::uint32_t> addresses = [] {
    std::set<std::uint32_t> initial;

    auto env_addr = get_zero_tier_fallback_broadcast_ip_v4();

    if (env_addr)
    {
      initial.emplace(env_addr->S_un.S_addr);
    }

    return initial;
  }();

  return addresses;
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

std::optional<std::string> get_zero_tier_peer_id_and_public_key()
{
  static std::optional<std::string> result = []() -> std::optional<std::string> {
    get_log() << "get_zero_tier_network_id\n";

    if (auto env_size = ::GetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", nullptr, 0); env_size >= 1)
    {
      std::string peer_id(env_size - 1, '\0');

      ::GetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", peer_id.data(), peer_id.size() + 1);


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

  return flags;
}

int get_zts_errno()
{
  return zts_errno;
}

int zt_to_winsock_error(int error)
{
  switch (error)
  {
  case ZTS_EPERM: {
    get_log() << "Received ZTS_EPERM\n";
    return WSAEACCES;
  }
  case ZTS_ENOENT: {
    get_log() << "Received ZTS_ENOENT\n";
#ifdef WSA_INVALID_HANDLE
    return WSA_INVALID_HANDLE;
#else
    return WSAEBADF;
#endif
  }
  case ZTS_ESRCH: {
    get_log() << "Received ZTS_ESRCH\n";
#ifdef WSA_INVALID_HANDLE
    return WSA_INVALID_HANDLE;
#else
    return WSAEBADF;
#endif
  }
  case ZTS_EINTR: {
    get_log() << "Received ZTS_EINTR\n";
    return WSAEINTR;
  }
  case ZTS_EIO: {
    get_log() << "Received ZTS_EIO\n";
#if WSA_IO_INCOMPLETE
    return WSA_IO_INCOMPLETE;
#else
    return WSAEINPROGRESS;
#endif
  }
  case ZTS_ENXIO: {
    get_log() << "Received ZTS_ENXIO\n";
    return WSAEFAULT;
  }
  case ZTS_EBADF: {
    get_log() << "Received ZTS_EBADF\n";
    return WSAEBADF;
  }
  case ZTS_EWOULDBLOCK: {
    get_log() << "Received ZTS_EWOULDBLOCK\n";
    return WSAEWOULDBLOCK;
  }
  case ZTS_ENOMEM: {
    get_log() << "Received ZTS_ENOMEM\n";
#ifdef WSA_NOT_ENOUGH_MEMORY
    return WSA_NOT_ENOUGH_MEMORY;
#else
    return WSA_QOS_TRAFFIC_CTRL_ERROR;
#endif
  }
  case ZTS_EACCES: {
    get_log() << "Received ZTS_EACCES\n";
    return WSAEACCES;
  }
  case ZTS_EFAULT: {
    get_log() << "Received ZTS_EFAULT\n";
    return WSAEFAULT;
  }
  case ZTS_EBUSY: {
    get_log() << "Received ZTS_EBUSY\n";
    return WSAEACCES;
  }
  case ZTS_EEXIST: {
    get_log() << "Received ZTS_EEXIST\n";
    return WSAEACCES;
  }
  case ZTS_ENODEV: {
    get_log() << "Received ZTS_ENODEV\n";
    return WSAEACCES;
  }
  case ZTS_EINVAL: {
    get_log() << "Received ZTS_EINVAL\n";
    return WSAEINVAL;
  }
  case ZTS_ENFILE: {
    get_log() << "Received ZTS_ENFILE\n";
    return WSAEMFILE;
  }
  case ZTS_EMFILE: {
    get_log() << "Received ZTS_EMFILE\n";
    return WSAEMFILE;
  }
  case ZTS_ENOSYS: {
    get_log() << "Received ZTS_ENOSYS\n";
    return WSAEACCES;
  }
  case ZTS_ENOTSOCK: {
    get_log() << "Received ZTS_EDESTADDRREQ\n";
    return WSAENOTSOCK;
  }
  case ZTS_EDESTADDRREQ: {
    get_log() << "Received ZTS_EDESTADDRREQ\n";
    return WSAEDESTADDRREQ;
  }
  case ZTS_EMSGSIZE: {
    get_log() << "Received ZTS_EMSGSIZE\n";
    return WSAEMSGSIZE;
  }
  case ZTS_EPROTOTYPE: {
    get_log() << "Received ZTS_EPROTOTYPE\n";
    return WSAEPROTOTYPE;
  }
  case ZTS_ENOPROTOOPT: {
    get_log() << "Received ZTS_ENOPROTOOPT\n";
    return WSAENOPROTOOPT;
  }
  case ZTS_EPROTONOSUPPORT: {
    get_log() << "Received ZTS_EPROTONOSUPPORT\n";
    return WSAEPROTONOSUPPORT;
  }
  case ZTS_ESOCKTNOSUPPORT: {
    get_log() << "Received ZTS_ESOCKTNOSUPPORT\n";
    return WSAESOCKTNOSUPPORT;
  }
  case ZTS_EOPNOTSUPP: {
    get_log() << "Received ZTS_EOPNOTSUPP\n";
    return WSAEOPNOTSUPP;
  }
  case ZTS_EPFNOSUPPORT: {
    get_log() << "Received ZTS_EPFNOSUPPORT\n";
    return WSAEPFNOSUPPORT;
  }
  case ZTS_EAFNOSUPPORT: {
    get_log() << "Received ZTS_EAFNOSUPPORT\n";
    return WSAEAFNOSUPPORT;
  }
  case ZTS_EADDRINUSE: {
    get_log() << "Received ZTS_EADDRINUSE\n";
    return WSAEADDRINUSE;
  }
  case ZTS_EADDRNOTAVAIL: {
    get_log() << "Received ZTS_EADDRNOTAVAIL\n";
    return WSAEADDRNOTAVAIL;
  }
  case ZTS_ENETDOWN: {
    get_log() << "Received ZTS_ENETDOWN\n";
    return WSAENETDOWN;
  }
  case ZTS_ENETUNREACH: {
    get_log() << "Received ZTS_ENETUNREACH\n";
    return WSAENETUNREACH;
  }
  case ZTS_ECONNABORTED: {
    get_log() << "Received ZTS_ECONNABORTED\n";
    return WSAECONNABORTED;
  }
  case ZTS_ECONNRESET: {
    get_log() << "Received ZTS_ECONNRESET\n";
    return WSAECONNRESET;
  }
  case ZTS_ENOBUFS: {
    get_log() << "Received ZTS_ENOBUFS\n";
    return WSAENOBUFS;
  }
  case ZTS_EISCONN: {
    get_log() << "Received ZTS_EISCONN\n";
    return WSAEISCONN;
  }
  case ZTS_ENOTCONN: {
    get_log() << "Received ZTS_ENOTCONN\n";
    return WSAENOTCONN;
  }
  case ZTS_ETIMEDOUT: {
    get_log() << "Received ZTS_ETIMEDOUT\n";
    return WSAETIMEDOUT;
  }
  case ZTS_ECONNREFUSED: {
    get_log() << "Received ZTS_ECONNREFUSED\n";
    return WSAECONNREFUSED;
  }
  case ZTS_EHOSTUNREACH: {

    get_log() << "Received ZTS_EHOSTUNREACH\n";
    return WSAEHOSTUNREACH;
  }
  case ZTS_EALREADY: {
    get_log() << "Received ZTS_EALREADY\n";
    return WSAEALREADY;
  }
  case ZTS_EINPROGRESS: {

    return WSAEINPROGRESS;
  }
  case 140: {
    get_log() << "Received error 140 ";
    return WSAEWOULDBLOCK;
  }
  default: {
    get_log() << "Received unknown error: " << error;
#ifdef WSA_INVALID_PARAMETER
    return WSA_INVALID_PARAMETER;
#else
    return WSAEINVAL;
#endif
  }
  }
  get_log() << "Received unknown error: " << error;
  return error;
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