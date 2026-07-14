#include <ZeroTierSockets.h>

#ifdef USE_WINSOCK2
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <WinSock.h>
#endif
#include <wsnwlink.h>
#include <siege/platform/win/module.hpp>
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
        return WSASYSNOTREADY;
      }
      else if (!is_online && !is_connected)
      {
        get_log() << "Node could not be started and could not join network.\n";
        return WSASYSNOTREADY;
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

    if (zts_bsd_ioctl(socket, ZTS_FIONBIO, &non_blocking) < 0)
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
    return zt_to_winsock_result(zt_result);
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
  if (cmd != FIONREAD)
  {
    get_log() << "siege_ioctlsocket, cmd: " << ioctl_cmd_to_string(cmd);
  }

  if (!get_zero_tier_handles().contains(to_zts(ws)))
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

    int non_blocking = 1;

    if (zts_bsd_ioctl(zt_result, ZTS_FIONBIO, &non_blocking) < 0)
    {
      zts_bsd_close(zt_result);
      return INVALID_SOCKET;
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

    auto zt_result = zts_bsd_sendto(to_zts(ws), buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&address_and_size.first, address_and_size.second);

    if (zt_result < 0)
    {
      return zt_to_winsock_result(zt_result);
    }

    return zt_result;
  }

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
  zts_fd_set zt_read{};
  zts_fd_set* final_read = nullptr;

  auto transfer_set_zts = [](auto& source, auto& dest) {
    ZTS_FD_ZERO(&dest);
    for (auto i = 0; i < source.fd_count; ++i)
    {
      auto zts = to_zts(source.fd_array[i]);
      if (!get_zero_tier_handles().contains(zts))
      {
        get_log() << "Non Zero Tier handle detected " << zts;
        continue;
      }
      ZTS_FD_SET(zts, &dest);
    }
  };

  if (read && read->fd_count)
  {
    transfer_set_zts(*read, zt_read);
    final_read = &zt_read;
  }

  zts_fd_set zt_write{};
  zts_fd_set* final_write = nullptr;

  if (write && write->fd_count)
  {
    transfer_set_zts(*write, zt_write);
    final_write = &zt_write;
  }

  zts_fd_set zt_except{};
  zts_fd_set* final_except = nullptr;

  if (except && except->fd_count)
  {
    transfer_set_zts(*except, zt_except);
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

  auto transfer_set_win32 = [](zts_fd_set& source, fd_set& dest) {
    fd_set temp{};
    for (auto i = 0; i < dest.fd_count; ++i)
    {
      auto zts = to_zts(dest.fd_array[i]);
      if (!get_zero_tier_handles().contains(zts))
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

  if (final_read)
  {
    assert(read != nullptr);
    transfer_set_win32(*final_read, *read);
  }

  if (final_write)
  {
    assert(write != nullptr);
    transfer_set_win32(*final_write, *write);
  }

  if (final_except)
  {
    assert(except != nullptr);
    transfer_set_win32(*final_except, *except);
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
    return 0;
  }
  case ZTS_ERR_GENERAL: {
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

  return zt_flags;
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
    return WSAEACCES;
  }
  case ZTS_ENOENT: {
#ifdef WSA_INVALID_HANDLE
    return WSA_INVALID_HANDLE;
#else
    return WSAEBADF;
#endif
  }
  case ZTS_ESRCH: {
#ifdef WSA_INVALID_HANDLE
    return WSA_INVALID_HANDLE;
#else
    return WSAEBADF;
#endif
  }
  case ZTS_EINTR: {
    return WSAEINTR;
  }
  case ZTS_EIO: {
    return WSAEINPROGRESS;
  }
  case ZTS_ENXIO: {
    return WSAEFAULT;
  }
  case ZTS_EBADF: {
    return WSAEBADF;
  }
  // not documented but probably an implementation
  // detail leaking through
  case EWOULDBLOCK: {
    return WSAEWOULDBLOCK;
  }
  case ZTS_EWOULDBLOCK: {
    return WSAEWOULDBLOCK;
  }
  case ZTS_ENOMEM: {
#ifdef WSA_NOT_ENOUGH_MEMORY
    return WSA_NOT_ENOUGH_MEMORY;
#else
    return WSA_QOS_TRAFFIC_CTRL_ERROR;
#endif
  }
  case ZTS_EACCES: {
    return WSAEACCES;
  }
  case ZTS_EFAULT: {
    return WSAEFAULT;
  }
  case ZTS_EBUSY: {
    return WSAEACCES;
  }
  case ZTS_EEXIST: {
    return WSAEACCES;
  }
  case ZTS_ENODEV: {
    return WSAEACCES;
  }
  case ZTS_EINVAL: {
    return WSAEINVAL;
  }
  case ZTS_ENFILE: {
    return WSAEMFILE;
  }
  case ZTS_EMFILE: {
    return WSAEMFILE;
  }
  case ZTS_ENOSYS: {
    return WSAEACCES;
  }
  case ZTS_ENOTSOCK: {
    return WSAENOTSOCK;
  }
  case ZTS_EDESTADDRREQ: {
    return WSAEDESTADDRREQ;
  }
  case ZTS_EMSGSIZE: {
    return WSAEMSGSIZE;
  }
  case EPROTOTYPE: {
    return WSAEPROTOTYPE;
  }
  case ZTS_EPROTOTYPE: {
    return WSAEPROTOTYPE;
  }
  case ZTS_ENOPROTOOPT: {
    return WSAENOPROTOOPT;
  }
  case ZTS_EPROTONOSUPPORT: {
    return WSAEPROTONOSUPPORT;
  }
  case ZTS_ESOCKTNOSUPPORT: {
    return WSAESOCKTNOSUPPORT;
  }
  case EOPNOTSUPP: {
    return WSAEOPNOTSUPP;
  }
  case ZTS_EOPNOTSUPP: {
    return WSAEOPNOTSUPP;
  }
  case ZTS_EPFNOSUPPORT: {
    return WSAEPFNOSUPPORT;
  }
  case ZTS_EAFNOSUPPORT: {
    return WSAEAFNOSUPPORT;
  }
  case ZTS_EADDRINUSE: {
    return WSAEADDRINUSE;
  }
  case ZTS_EADDRNOTAVAIL: {
    return WSAEADDRNOTAVAIL;
  }
  case ENETDOWN: {
    return WSAENETDOWN;
  }
  case ZTS_ENETDOWN: {
    return WSAENETDOWN;
  }
  case ENETUNREACH: {
    return WSAENETUNREACH;
  }
  case ZTS_ENETUNREACH: {
    return WSAENETUNREACH;
  }
  case ZTS_ECONNABORTED: {
    return WSAECONNABORTED;
  }
  case ZTS_ECONNRESET: {
    return WSAECONNRESET;
  }
  case ZTS_ENOBUFS: {
    return WSAENOBUFS;
  }
  case ZTS_EISCONN: {
    return WSAEISCONN;
  }
  case ENOTCONN: {
    return WSAENOTCONN;
  }
  case ZTS_ENOTCONN: {
    return WSAENOTCONN;
  }
  case ETIMEDOUT: {
    return WSAETIMEDOUT;
  }
  case ZTS_ETIMEDOUT: {
    return WSAETIMEDOUT;
  }
  case ZTS_ECONNREFUSED: {
    return WSAECONNREFUSED;
  }
  case ZTS_EHOSTUNREACH: {
    return WSAEHOSTUNREACH;
  }
  case ZTS_EALREADY: {
    return WSAEALREADY;
  }
  case ZTS_EINPROGRESS: {

    return WSAEINPROGRESS;
  }
  default: {
#ifdef WSA_INVALID_PARAMETER
    return WSA_INVALID_PARAMETER;
#else
    return WSAEINVAL;
#endif
  }
  }
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