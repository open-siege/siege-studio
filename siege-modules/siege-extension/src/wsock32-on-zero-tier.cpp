#include <ZeroTierSockets.h>

#if USE_WINSOCK2
#include <WinSock2.h>
#else
#include <WinSock.h>
#endif
#include <optional>
#include <string>
#include <map>
#include <iomanip>
#include <set>
#include <array>
#include <vector>
#include <filesystem>

#ifdef _DEBUG
#include <fstream>
#else
#include <sstream>
#endif
#include <siege/platform/win/module.hpp>

namespace fs = std::filesystem;

std::set<int>& get_zero_tier_handles()
{
  static std::set<int> handles;
  return handles;
}


void load_system_wsock();


std::ostream& get_log()
{
#ifdef _DEBUG
  static std::ofstream file_log("networking.log", std::ios::trunc);
#else
  static std::stringstream file_log;
  file_log.str("");
#endif
  return file_log;
}


HMODULE get_ztlib()
{
  static HMODULE ztlib = [] {
    auto module_path = win32::module_ref::current_module().GetModuleFileName();

    auto zt_path = fs::path(module_path).parent_path() / "zt-shared.dll";


    get_log() << "Loading zero tier library: " << zt_path << std::endl;

    auto result = LoadLibraryExW(zt_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

    return result;
  }();

  return ztlib;
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

std::optional<std::string> get_zero_tier_peer_id_and_public_key()
{
  static std::optional<std::string> result = []() -> std::optional<std::string> {
    get_log() << "get_zero_tier_network_id\n";

    if (auto env_size = ::GetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", nullptr, 0); env_size >= 1)
    {
      std::string peer_id(env_size - 1, '\0');

      ::GetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", peer_id.data(), peer_id.size() + 1);

      static auto* id_pair_is_valid = (std::add_pointer_t<decltype(zts_id_pair_is_valid)>)::GetProcAddress(get_ztlib(), "zts_id_pair_is_valid");

      peer_id.resize(ZTS_ID_STR_BUF_LEN);

      if (id_pair_is_valid(peer_id.data(), (unsigned int)peer_id.size()))
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

std::optional<in_addr> get_zero_tier_fallback_broadcast_ip_v4();

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
  return *(int*)::GetProcAddress(get_ztlib(), "zts_errno");
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
    return WSA_NOT_ENOUGH_MEMORY
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
    get_log() << "Received error 140 " << "\n";
    return WSAEWOULDBLOCK;
  }
  default: {
    get_log() << "Received unknown error: " << error << "\n";
#ifdef WSA_INVALID_PARAMETER
    return WSA_INVALID_PARAMETER;
#else
    return WSAEINVAL;
#endif
  }
  }
  get_log() << "Received unknown error: " << error << "\n";
  return error;
}

int zt_to_winsock_result(int code);

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

bool use_zero_tier()
{
  return get_zero_tier_network_id() && get_ztlib();
}

extern "C" {
HMODULE wsock_module = nullptr;
decltype(::WSAStartup)* wsock_WSAStartup = nullptr;
decltype(::WSACleanup)* wsock_WSACleanup = nullptr;
decltype(::socket)* wsock_socket = nullptr;
decltype(::setsockopt)* wsock_setsockopt = nullptr;
decltype(::getsockopt)* wsock_getsockopt = nullptr;
decltype(::getsockname)* wsock_getsockname = nullptr;
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

int __stdcall siege_WSAStartup(WORD version, LPWSADATA data)
{
  constexpr auto value = FIOASYNC;
  load_system_wsock();
  get_log() << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version) << std::endl;
  auto result = wsock_WSAStartup(version, data);

  if (auto network_id = get_zero_tier_network_id(); network_id && get_ztlib())
  {
    if (result == 0)
    {
      wsock_WSACleanup();
    }

    get_log() << "Zero Tier library available and network is set\n";
    static auto* init_from_memory = (std::add_pointer_t<decltype(zts_init_from_memory)>)::GetProcAddress(get_ztlib(), "zts_init_from_memory");
    static auto* init_from_storage = (std::add_pointer_t<decltype(zts_init_from_storage)>)::GetProcAddress(get_ztlib(), "zts_init_from_storage");
    static auto* node_get_id_pair = (std::add_pointer_t<decltype(zts_node_get_id_pair)>)::GetProcAddress(get_ztlib(), "zts_node_get_id_pair");
    static auto* node_start = (std::add_pointer_t<decltype(zts_node_start)>)::GetProcAddress(get_ztlib(), "zts_node_start");
    static auto* node_get_id = (std::add_pointer_t<decltype(zts_node_get_id)>)::GetProcAddress(get_ztlib(), "zts_node_get_id");
    static auto* node_is_online = (std::add_pointer_t<decltype(zts_node_is_online)>)::GetProcAddress(get_ztlib(), "zts_node_is_online");
    static auto* util_delay = (std::add_pointer_t<decltype(zts_util_delay)>)::GetProcAddress(get_ztlib(), "zts_util_delay");
    static auto* net_join = (std::add_pointer_t<decltype(zts_net_join)>)::GetProcAddress(get_ztlib(), "zts_net_join");
    static auto* net_transport_is_ready = (std::add_pointer_t<decltype(zts_net_transport_is_ready)>)::GetProcAddress(get_ztlib(), "zts_net_transport_is_ready");

    if (!get_node_online_status())
    {
      if (auto node_id_and_key = get_zero_tier_peer_id_and_public_key(); node_id_and_key)
      {
        auto init_result = init_from_memory(node_id_and_key->data(), (unsigned int)node_id_and_key->size());
        if (init_result == 0)
        {
          get_log() << "Init from memory succeeded\n";
        }
        else
        {
          get_log() << "Could not init from memory with code" << get_zts_errno() << '\n';
        }
      }
      else
      {
        init_from_storage(".");
      }

      get_log() << "Starting node\n";
      node_start();
      bool is_online = false;
      bool is_connected = false;

      for (auto i = 0; i < 500; ++i)
      {
        if (node_is_online())
        {
          auto id = node_get_id();

          get_log() << "Node is online with ID: " << std::to_string(id) << '\n';

          is_online = true;
          break;
        }
        util_delay(100);
      }

      net_join(*network_id);

      get_log() << "Joining network\n";
      for (auto i = 0; i < 500; ++i)
      {
        if (net_transport_is_ready(*network_id))
        {
          get_log() << "Joined network\n";
          is_connected = true;
          break;
        }
        util_delay(100);
      }

      get_node_online_status() = is_online && is_connected;

      if (is_online && !is_connected)
      {
        get_log() << "Node is online but could not join network. Stopping node.\n";
        static auto* node_stop = (std::add_pointer_t<decltype(zts_node_stop)>)::GetProcAddress(get_ztlib(), "zts_node_stop");
        node_stop();
        ::ExitProcess(-1);
      }
      else if (!is_online && !is_connected)
      {
        get_log() << "Node could not be started and could not join network.\n";
        ::ExitProcess(-1);
      }
    }

    return 0;
  }

  get_log().flush();
  return result;
}

int __stdcall siege_WSACleanup()
{
  load_system_wsock();
  get_log() << "siege_WSACleanup" << std::endl;

  if (use_zero_tier())
  {
    if (get_node_online_status())
    {
      get_log() << "Stopping Zero Tier node\n";
      static auto* node_stop = (std::add_pointer_t<decltype(zts_node_stop)>)::GetProcAddress(get_ztlib(), "zts_node_stop");
      auto zt_result = node_stop();
      get_node_online_status() = false;
      return zt_to_winsock_result(zt_result);
    }
    return 0;
  }

  return wsock_WSACleanup();
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
  load_system_wsock();
  get_log() << "siege_socket af: " << af << ", type: " << type << ", protocol: " << protocol << ", thread: " << GetCurrentThreadId() << std::endl;

  if (use_zero_tier())
  {
    if ((af == AF_UNSPEC || af == AF_INET) && (type == SOCK_STREAM || type == SOCK_DGRAM))
    {
      get_log() << "Creating zero tier socket\n";
      static auto* zt_socket = (std::add_pointer_t<decltype(zts_bsd_socket)>)::GetProcAddress(get_ztlib(), "zts_bsd_socket");
      static auto* zt_setsockopt = (std::add_pointer_t<decltype(zts_bsd_setsockopt)>)::GetProcAddress(get_ztlib(), "zts_bsd_setsockopt");
      auto socket = zt_socket(af, type, protocol);

      if (!(protocol == ZTS_IPPROTO_IP
            || protocol == ZTS_IPPROTO_ICMP
            || protocol == ZTS_IPPROTO_TCP
            || protocol == ZTS_IPPROTO_UDP
            || protocol == ZTS_IPPROTO_RAW))
      {
        get_log() << "Unsupported protocol for zero tier: " << protocol << std::endl;
      }

      if (socket == ZTS_ERR_SOCKET || socket == ZTS_ERR_SERVICE || socket == ZTS_ERR_ARG)
      {
        get_log() << "Could not create zero tier socket with error code: " << get_zts_errno() << '\0';

        if (socket == ZTS_ERR_ARG)
        {
          wsock_WSASetLastError(zt_to_winsock_error(get_zts_errno()));
        }
        else if (socket == ZTS_ERR_SERVICE)
        {
          wsock_WSASetLastError(WSANOTINITIALISED);
        }

        return INVALID_SOCKET;
      }

      get_log() << "Created zero tier socket successfully (" << socket << ")" << std::endl;
      get_zero_tier_handles().emplace(socket);


      int value = 65536;
      zts_socklen_t size = sizeof(value);

      zt_setsockopt(socket, ZTS_SOL_SOCKET, ZTS_SO_RCVBUF, &value, size);


      return (SOCKET)socket;
    }

    wsock_WSASetLastError(WSAESOCKTNOSUPPORT);
    return INVALID_SOCKET;
  }


  auto result = wsock_socket(af, type, protocol);
  get_log() << "Created winsock socket successfully (" << (int)result << ")" << std::endl;
  return result;
}

int __stdcall siege_recv(SOCKET s, char* buf, int len, int flags)
{
  get_log() << "siege_recv " << (int)s << std::endl;
  if (get_zero_tier_handles().contains((int)s) && get_ztlib())
  {
    get_log() << "zts_bsd_recv\n";

    static auto* zt_recv = (std::add_pointer_t<decltype(zts_bsd_recv)>)::GetProcAddress(get_ztlib(), "zts_bsd_recv");

    auto zt_result = (int)zt_recv((int)s, buf, (std::size_t)len, to_zt_msg_flags(flags));

    return zt_to_winsock_result(zt_result);
  }
  return wsock_recv(s, buf, len, flags);
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
int __stdcall siege_setsockopt(SOCKET s, int level, int optname, const char* optval, int optlen)
{
  get_log() << "siege_setsockopt" << (int)s << " " << optname << std::endl;
  if (use_zero_tier())
  {
    get_log() << "zts_bsd_setsockopt, level: " << level << " optname: " << optname << '\n';

    if (level != SOL_SOCKET)
    {
      get_log() << "Potentially unsupported socket level " << level << "\n";
    }

    BOOL some_flag = -1;
    static auto* zt_getsockopt = (std::add_pointer_t<decltype(zts_bsd_getsockopt)>)::GetProcAddress(get_ztlib(), "zts_bsd_getsockopt");

    zts_socklen_t size = sizeof(some_flag);


    static std::set<int> optnames = { SO_RCVTIMEO, SO_SNDTIMEO, SO_SNDBUF, SO_RCVBUF };


    if (level == SOL_SOCKET && !optnames.contains(optname))
    {
      level = ZTS_SOL_SOCKET;
    }
    else
    {
      get_log() << "Setting a regular socket setting " << optname << "\n";
    }

    static auto* zt_setsockopt = (std::add_pointer_t<decltype(zts_bsd_setsockopt)>)::GetProcAddress(get_ztlib(), "zts_bsd_setsockopt");
    auto zt_result = zt_setsockopt((int)s, level, optname, optval, optlen);

    return zt_to_winsock_result(zt_result);
  }

  auto result = wsock_setsockopt(s, level, optname, optval, optlen);

  if (result != 0)
  {
    get_log() << "setsockopt WSAGetLastError " << wsock_WSAGetLastError() << '\n';
  }

  return result;
}

int __stdcall siege_getsockopt(SOCKET s, int level, int optname, char* optval, int* optlen)
{
  get_log() << "siege_getsockopt" << (int)s << " " << optname << std::endl;
  if (use_zero_tier())
  {
    get_log() << "zts_bsd_setsockopt, level: " << level << " optname: " << optname << '\n';

    if (level != SOL_SOCKET)
    {
      get_log() << "Potentially unsupported socket level " << level << "\n";
    }

    BOOL some_flag = -1;
    zts_socklen_t size = 0;

    if (optlen)
    {
      size = *optlen;
    }


    static std::set<int> optnames = { SO_RCVTIMEO, SO_SNDTIMEO, SO_SNDBUF, SO_RCVBUF };

    if (level == SOL_SOCKET && !optnames.contains(optname))
    {
      level = ZTS_SOL_SOCKET;
    }
    else
    {
      get_log() << "Getting a regular socket setting " << optname << "\n";
    }

    static auto* zt_getsockopt = (std::add_pointer_t<decltype(zts_bsd_getsockopt)>)::GetProcAddress(get_ztlib(), "zts_bsd_getsockopt");
    auto zt_result = zt_getsockopt((int)s, level, optname, optval, &size);

    if (optlen)
    {
      *optlen = (int)size;
    }

    return zt_to_winsock_result(zt_result);
  }

  auto result = wsock_getsockopt(s, level, optname, optval, optlen);

  if (result != 0)
  {
    get_log() << "getsockopt WSAGetLastError " << wsock_WSAGetLastError() << '\n';
  }

  return result;
}

int __stdcall siege_recvfrom(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromLen)
{
  get_log() << "siege_recvfrom " << (int)s << std::endl;
  if (use_zero_tier())
  {
    static auto* zt_recvfrom = (std::add_pointer_t<decltype(zts_bsd_recvfrom)>)::GetProcAddress(get_ztlib(), "zts_bsd_recvfrom");
    static auto* zt_addr_get_str = (std::add_pointer_t<decltype(zts_addr_get_str)>)::GetProcAddress(get_ztlib(), "zts_addr_get_str");

    zts_sockaddr* zt_from_final = nullptr;
    zts_socklen_t* zt_from_len_final = nullptr;

    zts_sockaddr_in zt_addr{
      .sin_len = sizeof(zts_sockaddr_in)
    };

    zts_socklen_t zt_size = sizeof(zt_addr);

    if (from)
    {
      zt_from_final = (zts_sockaddr*)&zt_addr;
    }

    if (fromLen)
    {
      zt_from_len_final = &zt_size;
    }

    auto zt_result = (int)zt_recvfrom((int)s, buf, len, to_zt_msg_flags(flags), zt_from_final, zt_from_len_final);

    if (zt_result == ZTS_ERR_SOCKET || zt_result == ZTS_ERR_SERVICE || zt_result == ZTS_ERR_ARG)
    {
      get_log() << "zts_bsd_recvfrom had an error\n";

      return zt_to_winsock_result(zt_result);
    }

    sockaddr_in from_in = from_zts(zt_addr);

    if (from)
    {
      std::memcpy(from, &from_in, (fromLen && *fromLen > 0) ? *fromLen : sizeof(from_in));
    }

    if (fromLen)
    {
      *fromLen = sizeof(from_in);
    }

    return (int)zt_result;
  }


  auto result = wsock_recvfrom(s, buf, len, flags, from, fromLen);

  if (result < 0)
  {
    get_log() << "recvfrom WSAGetLastError " << wsock_WSAGetLastError() << '\n';
  }

  return result;
}

int __stdcall siege_getsockname(SOCKET s, sockaddr* name, int* length)
{
  get_log() << "siege_getsockname\n";
  if (use_zero_tier())
  {
    static auto* zt_getsockname = (std::add_pointer_t<decltype(zts_bsd_getsockname)>)::GetProcAddress(get_ztlib(), "zts_bsd_getsockname");

    if (name && length)
    {
      get_log() << "zts_bsd_getsockname\n";
      zts_sockaddr_in zt_addr{
        .sin_len = sizeof(zts_sockaddr_in)
      };

      zts_socklen_t zt_size = sizeof(zt_addr);

      auto zt_result = zt_getsockname((int)s, (zts_sockaddr*)&zt_addr, &zt_size);

      sockaddr_in from_in = from_zts(zt_addr);
      std::memcpy(name, &from_in, length && *length > 0 ? *length : sizeof(from_in));
      *length = sizeof(from_in);

      char* buffer = wsock_inet_ntoa(from_in.sin_addr);

      if (buffer)
      {
        get_log() << "zts_bsd_getsockname address is " << buffer << std::endl;
      }

      return zt_to_winsock_result(zt_result);
    }

    auto zt_result = zt_getsockname((int)s, nullptr, nullptr);
    return zt_to_winsock_result(zt_result);
  }
  return wsock_getsockname(s, name, length);
}

static_assert(FIONREAD == ZTS_FIONREAD);
static_assert(FIONBIO == ZTS_FIONBIO);
static_assert(IOCPARM_MASK == ZTS_IOCPARM_MASK);
static_assert(IOC_VOID == ZTS_IOC_VOID);
static_assert(IOC_OUT == ZTS_IOC_OUT);
static_assert(IOC_IN == ZTS_IOC_IN);
static_assert(IOC_INOUT == ZTS_IOC_INOUT);
int __stdcall siege_ioctlsocket(SOCKET s, long cmd, u_long* argp)
{
  get_log() << "siege_ioctlsocket, cmd: " << cmd << " " << (std::size_t)wsock_ioctlsocket << std::endl;
  if (use_zero_tier())
  {
    get_log() << "zts_bsd_ioctl\n";
    static auto* zt_ioctl = (std::add_pointer_t<decltype(zts_bsd_ioctl)>)::GetProcAddress(get_ztlib(), "zts_bsd_ioctl");

    auto zt_result = zt_ioctl((int)s, cmd, argp);

    return zt_to_winsock_result(zt_result);
  }
  auto result = wsock_ioctlsocket(s, cmd, argp);

  get_log() << "siege_ioctlsocket finished" << std::endl;

  return result;
}

int __stdcall siege_listen(SOCKET s, int backlog)
{
  get_log() << "siege_listen\n";
  if (use_zero_tier())
  {
    static auto* zt_listen = (std::add_pointer_t<decltype(zts_bsd_listen)>)::GetProcAddress(get_ztlib(), "zts_bsd_listen");

    auto zt_result = zt_listen((int)s, backlog);
    return zt_to_winsock_result(zt_result);
  }
  return wsock_listen(s, backlog);
}

SOCKET __stdcall siege_accept(SOCKET s, sockaddr* name, int* namelen)
{
  get_log() << "siege_accept\n";
  if (use_zero_tier())
  {
    get_log() << "zts_bsd_accept\n";
    static auto* zt_accept = (std::add_pointer_t<decltype(zts_bsd_accept)>)::GetProcAddress(get_ztlib(), "zts_bsd_accept");

    if (name && namelen)
    {
      zts_sockaddr_in zt_addr{};

      zts_socklen_t zt_size = sizeof(zt_addr);

      auto zt_result = zt_accept((int)s, (zts_sockaddr*)&zt_addr, &zt_size);

      if (zt_result >= 0)
      {
        zt_size = *namelen <= zt_size ? *namelen : zt_size;
        std::memcpy(name, &zt_addr, zt_size);
        *namelen = zt_size;
      }

      return zt_to_winsock_result(zt_result);
    }

    auto zt_result = zt_accept((int)s, nullptr, nullptr);

    return zt_to_winsock_result(zt_result);
  }
  return wsock_accept(s, name, namelen);
}

int __stdcall siege_connect(SOCKET s, const sockaddr* name, int namelen)
{
  get_log() << "siege_connect " << (int)s << std::endl;
  if (name)
  {
    char* buffer = wsock_inet_ntoa(((sockaddr_in*)name)->sin_addr);

    if (buffer)
    {
      get_log() << "siege_connect " << (int)s << " " << buffer << std::endl;
    }
  }
  else
  {
    get_log() << "siege_connect " << (int)s << std::endl;
  }

  if (use_zero_tier())
  {
    get_log() << "zts_bsd_connect\n";
    static auto* zt_connect = (std::add_pointer_t<decltype(zts_bsd_connect)>)::GetProcAddress(get_ztlib(), "zts_bsd_connect");

    if (name)
    {
      zts_sockaddr_in zt_addr = to_zts(*(sockaddr_in*)name);

      zts_socklen_t zt_size = sizeof(zt_addr);

      auto zt_result = zt_connect((int)s, (zts_sockaddr*)&zt_addr, zt_size);

      return zt_to_winsock_result(zt_result);
    }

    auto zt_result = zt_connect((int)s, nullptr, namelen);

    return zt_to_winsock_result(zt_result);
  }
  return wsock_connect(s, name, namelen);
}

int __stdcall siege_bind(SOCKET s, const sockaddr* addr, int namelen)
{
  get_log() << "siege_bind " << (int)s << std::endl;
  if (addr)
  {
    char* buffer = wsock_inet_ntoa(((sockaddr_in*)addr)->sin_addr);

    if (buffer)
    {
      get_log() << "siege_bind " << (int)s << " " << buffer << std::endl;
    }
  }
  else
  {
    get_log() << "siege_bind " << (int)s << std::endl;
  }

  if (use_zero_tier())
  {
    get_log() << "zts_bsd_bind\n";
    static auto* zt_bind = (std::add_pointer_t<decltype(zts_bsd_bind)>)::GetProcAddress(get_ztlib(), "zts_bsd_bind");

    if (addr)
    {
      zts_sockaddr_in zt_addr = to_zts(*(sockaddr_in*)addr);
      zts_socklen_t zt_size = sizeof(zt_addr);

      auto zt_result = zt_bind((int)s, (zts_sockaddr*)&zt_addr, zt_size);

      in_addr temp{};
      std::memcpy(&temp, &((sockaddr_in*)addr)->sin_addr, sizeof(int));
      char* buffer = wsock_inet_ntoa(temp);

      if (buffer)
      {
        get_log() << "zts_bsd_bind to " << buffer << " with result " << zt_result << '\n';
      }
      else
      {
        get_log() << "zts_bsd_bind result " << zt_result << '\n';
      }

      return zt_to_winsock_result(zt_result);
    }

    auto zt_result = zt_bind((int)s, nullptr, namelen);

    return zt_to_winsock_result(zt_result);
  }
  auto result = wsock_bind(s, addr, namelen);

  get_log() << "Bind call has error " << wsock_WSAGetLastError() << "\n";

  return result;
}

int __stdcall siege_send(SOCKET s, const char* buf, int len, int flags)
{
  get_log() << "siege_send\n";
  if (use_zero_tier())
  {
    get_log() << "zts_bsd_send\n";
    static auto* zt_send = (std::add_pointer_t<decltype(zts_bsd_send)>)::GetProcAddress(get_ztlib(), "zts_bsd_send");

    auto zt_result = zt_send((int)s, buf, (std::size_t)len, to_zt_msg_flags(flags));

    return zt_to_winsock_result(zt_result);
  }
  return wsock_send(s, buf, len, flags);
}

int __stdcall siege_sendto(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
  get_log() << "siege_sendto " << (int)s << std::endl;
  if (to)
  {
    char* buffer = wsock_inet_ntoa(((sockaddr_in*)to)->sin_addr);

    if (buffer)
    {
      get_log() << "siege_sendto " << (int)s << " " << buffer << std::endl;
    }
  }
  else
  {
    get_log() << "siege_sendto " << (int)s << std::endl;
  }

  if (use_zero_tier())
  {
    get_log() << "zts_bsd_sendto\n";
    static auto* zt_sendto = (std::add_pointer_t<decltype(zts_bsd_sendto)>)::GetProcAddress(get_ztlib(), "zts_bsd_sendto");
    static auto* zt_setsockopt = (std::add_pointer_t<decltype(zts_bsd_setsockopt)>)::GetProcAddress(get_ztlib(), "zts_bsd_setsockopt");
    static auto* zt_net_get_broadcast = (std::add_pointer_t<decltype(zts_net_get_broadcast)>)::GetProcAddress(get_ztlib(), "zts_net_get_broadcast");
    static auto* zt_getsockopt = (std::add_pointer_t<decltype(zts_bsd_getsockopt)>)::GetProcAddress(get_ztlib(), "zts_bsd_getsockopt");

    if (to)
    {
      zts_sockaddr_in zt_addr = to_zts(*(sockaddr_in*)to);
      zts_socklen_t zt_size = sizeof(zt_addr);

      if (((sockaddr_in*)to)->sin_addr.S_un.S_addr == ZTS_IPADDR_BROADCAST)
      {
        get_log() << "Trying to broadcast\n";
        int socket_can_broadcast = 0;
        int socket_type = 0;
        zts_socklen_t size = sizeof(int);

        zt_getsockopt((int)s, ZTS_SOL_SOCKET, ZTS_SO_BROADCAST, &socket_can_broadcast, &size);
        zt_getsockopt((int)s, ZTS_SOL_SOCKET, ZTS_SO_TYPE, &socket_type, &size);

        if (zt_net_get_broadcast(*get_zero_tier_network_id()) && socket_type == ZTS_SOCK_DGRAM)
        {
          get_log() << "Network can broadcast and socket is datagram\n";
          if (!socket_can_broadcast)
          {
            get_log() << "Socket cannot broadcast. Setting broadcast flag.\n";
            socket_can_broadcast = 1;
            zt_setsockopt((int)s, ZTS_SOL_SOCKET, ZTS_SO_BROADCAST, &socket_can_broadcast, size);
          }

          auto zt_result = zt_sendto((int)s, buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&zt_addr, zt_size);

          if (auto ip = get_zero_tier_fallback_broadcast_ip_v4(); ip && zt_result < 0)
          {
            get_log() << "Could not broadcast. Trying direct IP.\n";
            zt_addr.sin_addr.S_addr = ip->S_un.S_addr;
            zt_result = zt_sendto((int)s, buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&zt_addr, zt_size);
          }

          if (zt_result < 0)
          {
            get_log() << "Still could not broadcast.\n";
          }

          return zt_to_winsock_result(zt_result);
        }
        else if (auto ip = get_zero_tier_fallback_broadcast_ip_v4(); ip)
        {
          zt_addr.sin_addr.S_addr = ip->S_un.S_addr;
          auto zt_result = zt_sendto((int)s, buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&zt_addr, zt_size);

          return zt_to_winsock_result(zt_result);
        }
        else
        {
          wsock_WSASetLastError(WSAEALREADY);
          return SOCKET_ERROR;
        }
      }


      get_log() << "Doing a regular sendto\n";
      auto zt_result = zt_sendto((int)s, buf, len, to_zt_msg_flags(flags), (zts_sockaddr*)&zt_addr, zt_size);

      return zt_to_winsock_result(zt_result);
    }

    get_log() << "Somehow doing a bad send to\n";
    auto zt_result = zt_sendto((int)s, buf, len, to_zt_msg_flags(flags), nullptr, 0);

    return zt_to_winsock_result(zt_result);
  }
  return wsock_sendto(s, buf, len, flags, to, tolen);
}

#ifdef SD_RECEIVE
static_assert(SD_RECEIVE == ZTS_SHUT_RD);
static_assert(SD_SEND == ZTS_SHUT_WR);
static_assert(SD_BOTH == ZTS_SHUT_RDWR);
#endif
int __stdcall siege_shutdown(SOCKET s, int how)
{
  get_log() << "siege_shutdown\n";
  if (use_zero_tier())
  {
    get_log() << "zts_bsd_shutdown\n";
    static auto* zt_shutdown = (std::add_pointer_t<decltype(zts_bsd_shutdown)>)::GetProcAddress(get_ztlib(), "zts_bsd_shutdown");

    auto zt_result = zt_shutdown((int)s, how);

    return zt_to_winsock_result(zt_result);
  }
  return wsock_shutdown(s, how);
}

int __stdcall siege_closesocket(SOCKET s)
{
  get_log() << "siege_closesocket\n";
  if (use_zero_tier())
  {
    get_log() << "zts_bsd_close\n";
    static auto* zt_close = (std::add_pointer_t<decltype(zts_bsd_close)>)::GetProcAddress(get_ztlib(), "zts_bsd_close");
    auto zt_result = zt_close((int)s);

    return zt_to_winsock_result(zt_result);
  }
  return wsock_closesocket(s);
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout)
{
  get_log() << "siege_select " << std::endl;
  if (use_zero_tier())
  {
    static auto* zt_select = (std::add_pointer_t<decltype(zts_bsd_select)>)::GetProcAddress(get_ztlib(), "zts_bsd_select");
    zts_fd_set zt_read{};
    zts_fd_set* final_read = nullptr;

    auto transfer_set = [](auto& source, auto& dest) {
      ZTS_FD_ZERO(&dest);
      for (auto i = 0; i < source.fd_count; ++i)
      {
        if (get_zero_tier_handles().contains((int)source.fd_array[i]))
        {
          ZTS_FD_SET((int)source.fd_array[i], &dest);
        }
        else
        {
          get_log() << "Non Zero Tier handle detected " << (int)source.fd_array[i] << std::endl;
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

    auto count = zt_select(ZTS_FD_SETSIZE, final_read, final_write, final_except, final_timeval);

    if (count == ZTS_ERR_SOCKET || count == ZTS_ERR_SERVICE)
    {
      return zt_to_winsock_result(count);
    }

    return count;
  }
  return wsock_select(value, read, write, except, timeout);
}

int __stdcall siege___WSAFDIsSet(SOCKET s, fd_set* set)
{
  get_log() << "siege___WSAFDIsSet " << std::endl;
  if (use_zero_tier())
  {
    if (set)
    {
      zts_fd_set zt_set{};
      ZTS_FD_ZERO(&zt_set);
      for (auto i = 0; i < set->fd_count; ++i)
      {
        if (get_zero_tier_handles().contains((int)set->fd_array[i]))
        {
          ZTS_FD_SET((int)set->fd_array[i], &zt_set);
        }
      }
      return ZTS_FD_ISSET(s, &zt_set);
    }
    return 0;
  }

  return wsock___WSAFDIsSet(s, set);
}

hostent* __stdcall siege_gethostbyname(const char* name)
{
  load_system_wsock();
  if (name)
  {
    get_log() << "siege_gethostbyname: " << name << "\n";
  }
  else
  {
    get_log() << "siege_gethostbyname with no name \n";
  }

  if (get_zero_tier_network_id() && get_ztlib() && name && get_node_online_status())
  {
    get_log() << "Calling zts_bsd_gethostbyname\n";
    static auto* zt_host = (std::add_pointer_t<decltype(zts_bsd_gethostbyname)>)::GetProcAddress(get_ztlib(), "zts_bsd_gethostbyname");
    static auto* zt_addr_get_str = (std::add_pointer_t<decltype(zts_addr_get_str)>)::GetProcAddress(get_ztlib(), "zts_addr_get_str");
    auto result = zt_host(name);
    static std::map<std::string, hostent> host_cache;

    if (result)
    {

      host_cache[name] = from_zts(*result);
      get_log() << "Contains valid result\n";
      return &host_cache[name];
    }
    else if (name)
    {
      auto result = wsock_gethostbyname(name);

      if (!result)
      {
        return nullptr;
      }

      std::array<char, 255> temp{};

      if (wsock_gethostname(temp.data(), temp.size()) == 0 && std::string_view(name) == temp.data())
      {
        host_cache[name] = *result;

        if (host_cache[name].h_addr_list[0])
        {
          char ipstr[ZTS_IP_MAX_STR_LEN] = { 0 };
          zt_addr_get_str(*get_zero_tier_network_id(), ZTS_AF_INET, ipstr, ZTS_IP_MAX_STR_LEN);

          static std::array<char, sizeof(in_addr)> raw_ip{};
          auto ip_int = wsock_inet_addr(ipstr);
          std::memcpy(raw_ip.data(), &ip_int, raw_ip.size());
          host_cache[name].h_addr_list[0] = raw_ip.data();
        }
        return &host_cache[name];
      }

      return nullptr;
    }
  }

  return wsock_gethostbyname(name);
}

int __stdcall siege_gethostname(char* name, int namelen)
{
  load_system_wsock();
  get_log() << "siege_gethostname " << std::endl;
  return wsock_gethostname(name, namelen);
}

int __stdcall siege_WSAGetLastError()
{
  get_log() << "siege_WSAGetLastError " << std::endl;
  return wsock_WSAGetLastError();
}

u_long __stdcall siege_htonl(u_long value)
{
  get_log() << "siege_htonl " << std::endl;
  return wsock_htonl(value);
}

u_short __stdcall siege_htons(u_short value)
{
  get_log() << "siege_htons " << std::endl;
  return wsock_htons(value);
}

u_long __stdcall siege_ntohl(u_long value)
{
  get_log() << "siege_ntohl " << std::endl;
  return wsock_ntohl(value);
}

u_short __stdcall siege_ntohs(u_short value)
{
  get_log() << "siege_ntohs " << std::endl;
  return wsock_ntohs(value);
}

unsigned long __stdcall siege_inet_addr(const char* addr)
{
  get_log() << "siege_inet_addr " << std::endl;
  return wsock_inet_addr(addr);
}

char* __stdcall siege_inet_ntoa(in_addr in)
{
  get_log() << "siege_inet_ntoa " << std::endl;
  return wsock_inet_ntoa(in);
}

auto __stdcall siege_WSASetBlockingHook(FARPROC proc)
{
  get_log() << "siege_WSASetBlockingHook " << std::endl;
  return wsock_WSASetBlockingHook(proc);
}

auto __stdcall siege_WSAUnhookBlockingHook()
{
  get_log() << "siege_WSAUnhookBlockingHook " << std::endl;
  return wsock_WSAUnhookBlockingHook();
}

auto __stdcall siege_WSACancelBlockingCall()
{
  get_log() << "siege_WSACancelBlockingCall " << std::endl;
  return wsock_WSACancelBlockingCall();
}
}

void load_system_wsock()
{
  if (wsock_module)
  {
    return;
  }

  get_log() << "load_system_wsock " << std::endl;
  auto module_path = win32::module_ref::current_module().GetModuleFileName();

  auto dll_name = fs::path(module_path).filename().wstring();

  get_log() << "current module name: " << fs::path(module_path).filename().string() << std::endl;

  if (dll_name.contains(L"-"))
  {
    dll_name = dll_name.substr(0, dll_name.find(L"-"));
  }

  std::wstring temp(1024, L'\0');
  if (auto size = ::GetSystemDirectoryW(temp.data(), temp.size()); size == 0)
  {
    ::ExitProcess(-1);
  }
  else
  {
    temp.resize(size);
  }

  auto final_path = fs::path(temp) / dll_name;

  if (!final_path.has_extension())
  {
    final_path.replace_extension(".dll");
  }

  get_log() << "final dll path: " << final_path << std::endl;

  wsock_module = LoadLibraryExW(final_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

  if (!wsock_module)
  {
    return;
  }

  get_log() << "System module loaded. Getting proc addresses" << std::endl;

  wsock_WSAStartup = (decltype(wsock_WSAStartup))::GetProcAddress(wsock_module, "WSAStartup");
  wsock_WSACleanup = (decltype(wsock_WSACleanup))::GetProcAddress(wsock_module, "WSACleanup");
  wsock_socket = (decltype(wsock_socket))::GetProcAddress(wsock_module, "socket");
  wsock_setsockopt = (decltype(wsock_setsockopt))::GetProcAddress(wsock_module, "setsockopt");
  wsock_getsockname = (decltype(wsock_getsockname))::GetProcAddress(wsock_module, "getsockname");
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
}

int zt_to_winsock_result(int code)
{
  switch (code)
  {
  case ZTS_ERR_OK:
    return 0;
  case ZTS_ERR_SOCKET: {
    get_log() << "Received ZTS_ERR_SOCKET\n";
    wsock_WSASetLastError(zt_to_winsock_error(get_zts_errno()));
    return SOCKET_ERROR;
  }
  case ZTS_ERR_SERVICE: {
    get_log() << "Received ZTS_ERR_SERVICE\n";
    wsock_WSASetLastError(WSANOTINITIALISED);
    return SOCKET_ERROR;
  }
  case ZTS_ERR_ARG: {
    get_log() << "Received ZTS_ERR_ARG\n";
    wsock_WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }
  case ZTS_ERR_NO_RESULT: {
    get_log() << "Received ZTS_ERR_NO_RESULT\n";
    return 0;
  }
  case ZTS_ERR_GENERAL: {
    get_log() << "Received ZTS_ERR_GENERAL\n";
    wsock_WSASetLastError(WSASYSNOTREADY);
    return 0;
  }
  }

  return 0;
}

std::optional<in_addr> get_zero_tier_fallback_broadcast_ip_v4()
{
  static std::optional<in_addr> result = []() -> std::optional<in_addr> {
    get_log() << "get_zero_tier_fallback_broadcast_ip_v4\n";


    if (auto env_size = ::GetEnvironmentVariableA("ZERO_TIER_FALLBACK_BROADCAST_IP_V4", nullptr, 0); env_size >= 1)
    {
      std::string network_ip(env_size - 1, '\0');
      ::GetEnvironmentVariableA("ZERO_TIER_FALLBACK_BROADCAST_IP_V4", network_ip.data(), network_ip.size() + 1);

      get_log() << "Zero Tier fallback broadcast IP is " << network_ip << '\n';
      in_addr result{};
      result.S_un.S_addr = wsock_inet_addr(network_ip.c_str());
      return result;
    }

    get_log() << "No zero tier fallback broadcast IP\n";
    return std::nullopt;
  }();

  return result;
}
