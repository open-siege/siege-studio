module;

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <siege/platform/win/module.hpp>

export module wsock32.shared;

import std;

namespace fs = std::filesystem;

export struct backend_imports
{
  HMODULE module = nullptr;
  decltype(::WSAStartup)* WSAStartup = nullptr;
  decltype(::WSACleanup)* WSACleanup = nullptr;
  decltype(::socket)* socket = nullptr;
  decltype(::closesocket)* closesocket = nullptr;
  decltype(::shutdown)* shutdown = nullptr;
  decltype(::setsockopt)* setsockopt = nullptr;
  decltype(::getsockopt)* getsockopt = nullptr;
  decltype(::getsockname)* getsockname = nullptr;
  decltype(::getpeername)* getpeername = nullptr;
  decltype(::gethostbyname)* gethostbyname = nullptr;
  decltype(::recvfrom)* recvfrom = nullptr;
  decltype(::sendto)* sendto = nullptr;
  decltype(::ioctlsocket)* ioctlsocket = nullptr;
  decltype(::bind)* bind = nullptr;
  decltype(::connect)* connect = nullptr;
  decltype(::accept)* accept = nullptr;
  decltype(::listen)* listen = nullptr;
  decltype(::select)* select = nullptr;
  decltype(::__WSAFDIsSet)* __WSAFDIsSet = nullptr;
};

export struct wsock_imports
{
  HMODULE module = nullptr;
  decltype(::WSAStartup)* WSAStartup = nullptr;
  decltype(::WSACleanup)* WSACleanup = nullptr;
  decltype(::socket)* socket = nullptr;
  decltype(::setsockopt)* setsockopt = nullptr;
  decltype(::getsockopt)* getsockopt = nullptr;
  decltype(::getsockname)* getsockname = nullptr;
  decltype(::getpeername)* getpeername = nullptr;
  decltype(::gethostbyaddr)* gethostbyaddr = nullptr;
  decltype(::gethostname)* gethostname = nullptr;
  decltype(::gethostbyname)* gethostbyname = nullptr;
  decltype(::recv)* recv = nullptr;
  decltype(::recvfrom)* recvfrom = nullptr;
  decltype(::send)* send = nullptr;
  decltype(::sendto)* sendto = nullptr;
  decltype(::ioctlsocket)* ioctlsocket = nullptr;
  decltype(::bind)* bind = nullptr;
  decltype(::connect)* connect = nullptr;
  decltype(::accept)* accept = nullptr;
  decltype(::listen)* listen = nullptr;
  decltype(::shutdown)* shutdown = nullptr;
  decltype(::select)* select = nullptr;
  decltype(::closesocket)* closesocket = nullptr;
  decltype(::__WSAFDIsSet)* __WSAFDIsSet = nullptr;
  decltype(::htonl)* htonl = nullptr;
  decltype(::htons)* htons = nullptr;
  decltype(::ntohl)* ntohl = nullptr;
  decltype(::ntohs)* ntohs = nullptr;
  decltype(::inet_addr)* inet_addr = nullptr;
  decltype(::inet_ntoa)* inet_ntoa = nullptr;
  decltype(::WSASetBlockingHook)* WSASetBlockingHook = nullptr;
  decltype(::WSAUnhookBlockingHook)* WSAUnhookBlockingHook = nullptr;
  decltype(::WSACancelBlockingCall)* WSACancelBlockingCall = nullptr;
  decltype(::WSAGetLastError)* WSAGetLastError = nullptr;
  decltype(::WSASetLastError)* WSASetLastError = nullptr;
  decltype(::WSAAsyncGetHostByName)* WSAAsyncGetHostByName = nullptr;
  decltype(::WSACancelAsyncRequest)* WSACancelAsyncRequest = nullptr;
  decltype(::WSAAsyncSelect)* WSAAsyncSelect = nullptr;

#ifdef USE_WINSOCK2
  decltype(::WSAStringToAddressA)* WSAStringToAddressA = nullptr;
  decltype(::WSAGetOverlappedResult)* WSAGetOverlappedResult = nullptr;
  decltype(::WSACreateEvent)* WSACreateEvent = nullptr;
  decltype(::WSAResetEvent)* WSAResetEvent = nullptr;
  decltype(::WSACloseEvent)* WSACloseEvent = nullptr;
  decltype(::WSAWaitForMultipleEvents)* WSAWaitForMultipleEvents = nullptr;
  decltype(::WSASendTo)* WSASendTo = nullptr;
  decltype(::WSASend)* WSASend = nullptr;
  decltype(::WSARecvFrom)* WSARecvFrom = nullptr;
  decltype(::WSARecv)* WSARecv = nullptr;
  decltype(::WSAEventSelect)* WSAEventSelect = nullptr;
  decltype(::WSAEnumNetworkEvents)* WSAEnumNetworkEvents = nullptr;
  decltype(::WSASocketW)* WSASocketW = nullptr;
  decltype(::WSAIoctl)* WSAIoctl = nullptr;

  // not actually used by any games, but rather by the AMD OpenGL driver
  decltype(::getaddrinfo)* getaddrinfo = nullptr;
  decltype(::freeaddrinfo)* freeaddrinfo = nullptr;
  decltype(::inet_ntop)* inet_ntop = nullptr;
#endif
};

export std::optional<wsock_imports> load_system_wsock()
{
  auto module_path = win32::module_ref::current_module().GetModuleFileName();

  auto dll_name = fs::path(module_path).filename().wstring();

  if (dll_name.contains(L"-backend-"))
  {
    dll_name = L"ws2_32";
  }
  else if (dll_name.contains(L"-"))
  {
    dll_name = dll_name.substr(0, dll_name.find(L"-"));
  }

  std::wstring temp(1024, L'\0');
  if (auto size = ::GetSystemDirectoryW(temp.data(), temp.size()); size == 0)
  {
    return std::nullopt;
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

  wsock_imports imports{
    .module = ::LoadLibraryExW(final_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)
  };

  if (!imports.module)
  {
    return std::nullopt;
  }

  imports.WSAStartup = (decltype(imports.WSAStartup))::GetProcAddress(imports.module, "WSAStartup");
  imports.WSACleanup = (decltype(imports.WSACleanup))::GetProcAddress(imports.module, "WSACleanup");
  imports.socket = (decltype(imports.socket))::GetProcAddress(imports.module, "socket");
  imports.setsockopt = (decltype(imports.setsockopt))::GetProcAddress(imports.module, "setsockopt");
  imports.getsockname = (decltype(imports.getsockname))::GetProcAddress(imports.module, "getsockname");
  imports.getpeername = (decltype(imports.getpeername))::GetProcAddress(imports.module, "getpeername");
  imports.getsockopt = (decltype(imports.getsockopt))::GetProcAddress(imports.module, "getsockopt");
  imports.gethostbyaddr = (decltype(imports.gethostbyaddr))::GetProcAddress(imports.module, "gethostbyaddr");
  imports.gethostname = (decltype(imports.gethostname))::GetProcAddress(imports.module, "gethostname");
  imports.gethostbyname = (decltype(imports.gethostbyname))::GetProcAddress(imports.module, "gethostbyname");
  imports.htons = (decltype(imports.htons))::GetProcAddress(imports.module, "htons");
  imports.htonl = (decltype(imports.htonl))::GetProcAddress(imports.module, "htonl");
  imports.ntohl = (decltype(imports.ntohl))::GetProcAddress(imports.module, "ntohl");
  imports.ntohs = (decltype(imports.ntohs))::GetProcAddress(imports.module, "ntohs");
  imports.inet_addr = (decltype(imports.inet_addr))::GetProcAddress(imports.module, "inet_addr");
  imports.inet_ntoa = (decltype(imports.inet_ntoa))::GetProcAddress(imports.module, "inet_ntoa");
  imports.recv = (decltype(imports.recv))::GetProcAddress(imports.module, "recv");
  imports.recvfrom = (decltype(imports.recvfrom))::GetProcAddress(imports.module, "recvfrom");
  imports.send = (decltype(imports.send))::GetProcAddress(imports.module, "send");
  imports.sendto = (decltype(imports.sendto))::GetProcAddress(imports.module, "sendto");
  imports.ioctlsocket = (decltype(imports.ioctlsocket))::GetProcAddress(imports.module, "ioctlsocket");
  imports.bind = (decltype(imports.connect))::GetProcAddress(imports.module, "bind");
  imports.connect = (decltype(imports.connect))::GetProcAddress(imports.module, "connect");
  imports.accept = (decltype(imports.accept))::GetProcAddress(imports.module, "accept");
  imports.listen = (decltype(imports.listen))::GetProcAddress(imports.module, "listen");
  imports.shutdown = (decltype(imports.shutdown))::GetProcAddress(imports.module, "shutdown");
  imports.select = (decltype(imports.select))::GetProcAddress(imports.module, "select");
  imports.closesocket = (decltype(imports.closesocket))::GetProcAddress(imports.module, "closesocket");
  imports.WSAGetLastError = (decltype(imports.WSAGetLastError))::GetProcAddress(imports.module, "WSAGetLastError");
  imports.WSASetLastError = (decltype(imports.WSASetLastError))::GetProcAddress(imports.module, "WSASetLastError");
  imports.__WSAFDIsSet = (decltype(imports.__WSAFDIsSet))::GetProcAddress(imports.module, "__WSAFDIsSet");
  imports.WSAAsyncGetHostByName = (decltype(imports.WSAAsyncGetHostByName))::GetProcAddress(imports.module, "WSAAsyncGetHostByName");
  imports.WSACancelAsyncRequest = (decltype(imports.WSACancelAsyncRequest))::GetProcAddress(imports.module, "WSACancelAsyncRequest");
  imports.WSASetBlockingHook = (decltype(imports.WSASetBlockingHook))::GetProcAddress(imports.module, "WSASetBlockingHook");
  imports.WSAUnhookBlockingHook = (decltype(imports.WSAUnhookBlockingHook))::GetProcAddress(imports.module, "WSAUnhookBlockingHook");
  imports.WSACancelBlockingCall = (decltype(imports.WSACancelBlockingCall))::GetProcAddress(imports.module, "WSACancelBlockingCall");
  imports.WSAAsyncSelect = (decltype(imports.WSAAsyncSelect))::GetProcAddress(imports.module, "WSAAsyncSelect");

#ifdef USE_WINSOCK2
  imports.WSAStringToAddressA = (decltype(imports.WSAStringToAddressA))::GetProcAddress(imports.module, "WSAStringToAddressA");
  imports.WSAGetOverlappedResult = (decltype(imports.WSAGetOverlappedResult))::GetProcAddress(imports.module, "WSAGetOverlappedResult");
  imports.WSACreateEvent = (decltype(imports.WSACreateEvent))::GetProcAddress(imports.module, "WSACreateEvent");
  imports.WSAResetEvent = (decltype(imports.WSAResetEvent))::GetProcAddress(imports.module, "WSAResetEvent");
  imports.WSACloseEvent = (decltype(imports.WSACloseEvent))::GetProcAddress(imports.module, "WSACloseEvent");
  imports.WSAWaitForMultipleEvents = (decltype(imports.WSAWaitForMultipleEvents))::GetProcAddress(imports.module, "WSAWaitForMultipleEvents");
  imports.WSASendTo = (decltype(imports.WSASendTo))::GetProcAddress(imports.module, "WSASendTo");
  imports.WSASend = (decltype(imports.WSASend))::GetProcAddress(imports.module, "WSASend");
  imports.WSARecvFrom = (decltype(imports.WSARecvFrom))::GetProcAddress(imports.module, "WSARecvFrom");
  imports.WSARecv = (decltype(imports.WSARecv))::GetProcAddress(imports.module, "WSARecv");
  imports.WSAEventSelect = (decltype(imports.WSAEventSelect))::GetProcAddress(imports.module, "WSAEventSelect");
  imports.WSAEnumNetworkEvents = (decltype(imports.WSAEnumNetworkEvents))::GetProcAddress(imports.module, "WSAEnumNetworkEvents");
  imports.WSASocketW = (decltype(imports.WSASocketW))::GetProcAddress(imports.module, "WSASocketW");
  imports.WSAIoctl = (decltype(imports.WSAIoctl))::GetProcAddress(imports.module, "WSAIoctl");
  imports.getaddrinfo = (decltype(imports.getaddrinfo))::GetProcAddress(imports.module, "getaddrinfo");
  imports.freeaddrinfo = (decltype(imports.freeaddrinfo))::GetProcAddress(imports.module, "freeaddrinfo");
  imports.inet_ntop = (decltype(imports.inet_ntop))::GetProcAddress(imports.module, "inet_ntop");
#endif

  return imports;
}

export std::optional<wsock_imports> imports{};

export void ensure_imports()
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

export std::ostream& get_log(std::string_view log_prefix = "networking")
{
  static const std::string stored_log_prefix = std::string{ log_prefix } + ":";

  struct debug_string_buf : public std::stringbuf
  {
  protected:
    int sync() override
    {
      constexpr auto small_string_size = std::string{}.capacity();

      auto current_view = view();

      if (!current_view.empty() && current_view.size() <= small_string_size)
      {
        std::string temp(current_view.data(), current_view.size());
        ::OutputDebugStringA(temp.c_str());
        str("");
      }
      else if (!current_view.empty())
      {
        thread_local std::string temp;
        temp.reserve(current_view.size());
        temp.assign(current_view);
        ::OutputDebugStringA(temp.c_str());
        str("");
      }
      return 0;
    }
  };

  static debug_string_buf buffer{};
  static std::ostream debug_log{ &buffer };

  if (!buffer.view().empty() && buffer.view().back() != '\n')
  {
    debug_log << '\n';
  }


  if (buffer.view().size() > 64)
  {
    debug_log.flush();
  }

  debug_log << stored_log_prefix;
  return debug_log;
}

std::ostream& null_log()
{
  static thread_local std::ostream stream(nullptr);
  return stream;
}

std::ostream& log_sampled(std::atomic<std::uint64_t>& counter, std::uint64_t count)
{
  if (counter.fetch_add(1, std::memory_order_relaxed) % count == 0)
  {
    return get_log();
  }
  return null_log();
}

export std::ostream& log_sampled_read()
{
  static std::atomic<std::uint64_t> counter{};
  
  return log_sampled(counter, 20011);
}

export std::ostream& log_sampled_write()
{
  static std::atomic<std::uint64_t> counter{};

  return log_sampled(counter, 5003);
}

export std::ostream& log_sampled_check()
{
  static std::atomic<std::uint64_t> counter{};

  return log_sampled(counter, 10007);
}


export std::string level_to_string(int level)
{
  switch (level)
  {
  case SOL_SOCKET:
    return "SOL_SOCKET";
  case IPPROTO_TCP:
    return "IPPROTO_TCP";
  case IPPROTO_UDP:
    return "IPPROTO_UDP";
  default:
    return std::to_string(level);
  }
}

export std::string option_to_string(int option)
{
  switch (option)
  {
  case SO_BROADCAST:
    return "SO_BROADCAST";
  case SO_DEBUG:
    return "SO_DEBUG";
  case SO_DONTLINGER:
    return "SO_DONTLINGER";
  case SO_DONTROUTE:
    return "SO_DONTROUTE";
  case SO_KEEPALIVE:
    return "SO_KEEPALIVE";
  case SO_LINGER:
    return "SO_LINGER";
  case SO_OOBINLINE:
    return "SO_OOBINLINE";
  case SO_RCVBUF:
    return "SO_RCVBUF";
  case SO_REUSEADDR:
    return "SO_REUSEADDR";
  case SO_RCVTIMEO:
    return "SO_RCVTIMEO";
  case SO_SNDBUF:
    return "SO_SNDBUF";
  case SO_SNDTIMEO:
    return "SO_SNDTIMEO";
  default:
    return std::to_string(option);
  }
}

export std::string af_to_string(int af)
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

export std::string type_to_string(int type)
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

export std::string protocol_to_string(int protocol)
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

export std::string ioctl_cmd_to_string(long cmd)
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

export int exception_to_error_code() noexcept
{
  try
  {
    throw;
  }
  catch (const std::bad_alloc&)
  {
#ifdef USE_WINSOCK2
    return WSA_NOT_ENOUGH_MEMORY;
#else
    return WSAENOBUFS;
#endif
  }
  catch (const std::invalid_argument&)
  {
    return WSAEINVAL;
  }
  catch (const std::out_of_range&)
  {
    return WSAEFAULT;
  }
  catch (const std::length_error&)
  {
    return WSAEFAULT;
  }
  catch (const std::logic_error&)
  {
    return WSAEINVAL;
  }
  catch (...)
  {
    return WSAENETDOWN;
  }
}

export timeval ms_to_timeval(std::chrono::milliseconds total)
{
  auto secs = std::chrono::duration_cast<std::chrono::seconds>(total);
  auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(total - secs);

  return timeval{
    .tv_sec = static_cast<long>(secs.count()),
    .tv_usec = static_cast<long>(usecs.count())
  };
}

export std::chrono::milliseconds timeval_to_ms(timeval total)
{
  auto secs = std::chrono::seconds{ total.tv_sec };
  auto usecs = std::chrono::microseconds{ total.tv_usec };

  return std::chrono::duration_cast<std::chrono::milliseconds>(secs + usecs);
}