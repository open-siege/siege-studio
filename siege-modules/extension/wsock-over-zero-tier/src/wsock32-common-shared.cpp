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
  decltype(::getservbyname)* getservbyname = nullptr;
  decltype(::getservbyport)* getservbyport = nullptr;
  decltype(::getprotobyname)* getprotobyname = nullptr;
  decltype(::getprotobynumber)* getprotobynumber = nullptr;
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
  decltype(::WSAIsBlocking)* WSAIsBlocking = nullptr;
  decltype(::WSAGetLastError)* WSAGetLastError = nullptr;
  decltype(::WSASetLastError)* WSASetLastError = nullptr;
  decltype(::WSAAsyncGetHostByName)* WSAAsyncGetHostByName = nullptr;
  decltype(::WSAAsyncGetHostByAddr)* WSAAsyncGetHostByAddr = nullptr;
  decltype(::WSAAsyncGetProtoByName)* WSAAsyncGetProtoByName = nullptr;
  decltype(::WSAAsyncGetProtoByNumber)* WSAAsyncGetProtoByNumber = nullptr;
  decltype(::WSAAsyncGetServByName)* WSAAsyncGetServByName = nullptr;
  decltype(::WSAAsyncGetServByPort)* WSAAsyncGetServByPort = nullptr;
  decltype(::WSACancelAsyncRequest)* WSACancelAsyncRequest = nullptr;
  decltype(::WSAAsyncSelect)* WSAAsyncSelect = nullptr;

#ifdef USE_WINSOCK2
  decltype(::WSAStringToAddressA)* WSAStringToAddressA = nullptr;
  decltype(::WSAAddressToStringA)* WSAAddressToStringA = nullptr;
  decltype(::WSAGetOverlappedResult)* WSAGetOverlappedResult = nullptr;
  decltype(::WSACreateEvent)* WSACreateEvent = nullptr;
  decltype(::WSASetEvent)* WSASetEvent = nullptr;
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
  decltype(::WSASocketA)* WSASocketA = nullptr;
  decltype(::WSAAccept)* WSAAccept = nullptr;
  decltype(::WSAIoctl)* WSAIoctl = nullptr;

  // not actually used by any games, but rather by the AMD OpenGL driver
  decltype(::getaddrinfo)* getaddrinfo = nullptr;
  decltype(::freeaddrinfo)* freeaddrinfo = nullptr;
  decltype(::inet_ntop)* inet_ntop = nullptr;
  decltype(::inet_pton)* inet_pton = nullptr;
  decltype(::getnameinfo)* getnameinfo = nullptr;
  decltype(::GetNameInfoW)* GetNameInfoW = nullptr;
#endif
};

export wsock_imports load_wsock_imports(HMODULE module)
{
  wsock_imports imports{
    .module = module
  };

  imports.WSAStartup = (decltype(imports.WSAStartup))::GetProcAddress(module, "WSAStartup");
  imports.WSACleanup = (decltype(imports.WSACleanup))::GetProcAddress(module, "WSACleanup");
  imports.socket = (decltype(imports.socket))::GetProcAddress(module, "socket");
  imports.setsockopt = (decltype(imports.setsockopt))::GetProcAddress(module, "setsockopt");
  imports.getsockname = (decltype(imports.getsockname))::GetProcAddress(module, "getsockname");
  imports.getpeername = (decltype(imports.getpeername))::GetProcAddress(module, "getpeername");
  imports.getsockopt = (decltype(imports.getsockopt))::GetProcAddress(module, "getsockopt");
  imports.gethostbyaddr = (decltype(imports.gethostbyaddr))::GetProcAddress(module, "gethostbyaddr");
  imports.gethostname = (decltype(imports.gethostname))::GetProcAddress(module, "gethostname");
  imports.gethostbyname = (decltype(imports.gethostbyname))::GetProcAddress(module, "gethostbyname");
  imports.getservbyname = (decltype(imports.getservbyname))::GetProcAddress(module, "getservbyname");
  imports.getservbyport = (decltype(imports.getservbyport))::GetProcAddress(module, "getservbyport");
  imports.getprotobyname = (decltype(imports.getprotobyname))::GetProcAddress(module, "getprotobyname");
  imports.getprotobynumber = (decltype(imports.getprotobynumber))::GetProcAddress(module, "getprotobynumber");
  imports.htons = (decltype(imports.htons))::GetProcAddress(module, "htons");
  imports.htonl = (decltype(imports.htonl))::GetProcAddress(module, "htonl");
  imports.ntohl = (decltype(imports.ntohl))::GetProcAddress(module, "ntohl");
  imports.ntohs = (decltype(imports.ntohs))::GetProcAddress(module, "ntohs");
  imports.inet_addr = (decltype(imports.inet_addr))::GetProcAddress(module, "inet_addr");
  imports.inet_ntoa = (decltype(imports.inet_ntoa))::GetProcAddress(module, "inet_ntoa");
  imports.recv = (decltype(imports.recv))::GetProcAddress(module, "recv");
  imports.recvfrom = (decltype(imports.recvfrom))::GetProcAddress(module, "recvfrom");
  imports.send = (decltype(imports.send))::GetProcAddress(module, "send");
  imports.sendto = (decltype(imports.sendto))::GetProcAddress(module, "sendto");
  imports.ioctlsocket = (decltype(imports.ioctlsocket))::GetProcAddress(module, "ioctlsocket");
  imports.bind = (decltype(imports.bind))::GetProcAddress(module, "bind");
  imports.connect = (decltype(imports.connect))::GetProcAddress(module, "connect");
  imports.accept = (decltype(imports.accept))::GetProcAddress(module, "accept");
  imports.listen = (decltype(imports.listen))::GetProcAddress(module, "listen");
  imports.shutdown = (decltype(imports.shutdown))::GetProcAddress(module, "shutdown");
  imports.select = (decltype(imports.select))::GetProcAddress(module, "select");
  imports.closesocket = (decltype(imports.closesocket))::GetProcAddress(module, "closesocket");
  imports.WSAGetLastError = (decltype(imports.WSAGetLastError))::GetProcAddress(module, "WSAGetLastError");
  imports.WSASetLastError = (decltype(imports.WSASetLastError))::GetProcAddress(module, "WSASetLastError");
  imports.__WSAFDIsSet = (decltype(imports.__WSAFDIsSet))::GetProcAddress(module, "__WSAFDIsSet");
  imports.WSAAsyncGetHostByName = (decltype(imports.WSAAsyncGetHostByName))::GetProcAddress(module, "WSAAsyncGetHostByName");
  imports.WSAAsyncGetHostByAddr = (decltype(imports.WSAAsyncGetHostByAddr))::GetProcAddress(module, "WSAAsyncGetHostByAddr");
  imports.WSAAsyncGetProtoByName = (decltype(imports.WSAAsyncGetProtoByName))::GetProcAddress(module, "WSAAsyncGetProtoByName");
  imports.WSAAsyncGetProtoByNumber = (decltype(imports.WSAAsyncGetProtoByNumber))::GetProcAddress(module, "WSAAsyncGetProtoByNumber");
  imports.WSAAsyncGetServByName = (decltype(imports.WSAAsyncGetServByName))::GetProcAddress(module, "WSAAsyncGetServByName");
  imports.WSAAsyncGetServByPort = (decltype(imports.WSAAsyncGetServByPort))::GetProcAddress(module, "WSAAsyncGetServByPort");
  imports.WSACancelAsyncRequest = (decltype(imports.WSACancelAsyncRequest))::GetProcAddress(module, "WSACancelAsyncRequest");
  imports.WSASetBlockingHook = (decltype(imports.WSASetBlockingHook))::GetProcAddress(module, "WSASetBlockingHook");
  imports.WSAUnhookBlockingHook = (decltype(imports.WSAUnhookBlockingHook))::GetProcAddress(module, "WSAUnhookBlockingHook");
  imports.WSACancelBlockingCall = (decltype(imports.WSACancelBlockingCall))::GetProcAddress(module, "WSACancelBlockingCall");
  imports.WSAIsBlocking = (decltype(imports.WSAIsBlocking))::GetProcAddress(module, "WSAIsBlocking");
  imports.WSAAsyncSelect = (decltype(imports.WSAAsyncSelect))::GetProcAddress(module, "WSAAsyncSelect");

#ifdef USE_WINSOCK2
  imports.WSAStringToAddressA = (decltype(imports.WSAStringToAddressA))::GetProcAddress(module, "WSAStringToAddressA");
  imports.WSAAddressToStringA = (decltype(imports.WSAAddressToStringA))::GetProcAddress(module, "WSAAddressToStringA");
  imports.WSAGetOverlappedResult = (decltype(imports.WSAGetOverlappedResult))::GetProcAddress(module, "WSAGetOverlappedResult");
  imports.WSACreateEvent = (decltype(imports.WSACreateEvent))::GetProcAddress(module, "WSACreateEvent");
  imports.WSAResetEvent = (decltype(imports.WSAResetEvent))::GetProcAddress(module, "WSAResetEvent");
  imports.WSASetEvent = (decltype(imports.WSASetEvent))::GetProcAddress(module, "WSASetEvent");
  imports.WSACloseEvent = (decltype(imports.WSACloseEvent))::GetProcAddress(module, "WSACloseEvent");
  imports.WSAWaitForMultipleEvents = (decltype(imports.WSAWaitForMultipleEvents))::GetProcAddress(module, "WSAWaitForMultipleEvents");
  imports.WSAAccept = (decltype(imports.WSAAccept))::GetProcAddress(module, "WSAAccept");
  imports.WSASendTo = (decltype(imports.WSASendTo))::GetProcAddress(module, "WSASendTo");
  imports.WSASend = (decltype(imports.WSASend))::GetProcAddress(module, "WSASend");
  imports.WSARecvFrom = (decltype(imports.WSARecvFrom))::GetProcAddress(module, "WSARecvFrom");
  imports.WSARecv = (decltype(imports.WSARecv))::GetProcAddress(module, "WSARecv");
  imports.WSAEventSelect = (decltype(imports.WSAEventSelect))::GetProcAddress(module, "WSAEventSelect");
  imports.WSAEnumNetworkEvents = (decltype(imports.WSAEnumNetworkEvents))::GetProcAddress(module, "WSAEnumNetworkEvents");
  imports.WSASocketW = (decltype(imports.WSASocketW))::GetProcAddress(module, "WSASocketW");
  imports.WSASocketA = (decltype(imports.WSASocketA))::GetProcAddress(module, "WSASocketA");
  imports.WSAIoctl = (decltype(imports.WSAIoctl))::GetProcAddress(module, "WSAIoctl");
  imports.getaddrinfo = (decltype(imports.getaddrinfo))::GetProcAddress(module, "getaddrinfo");
  imports.freeaddrinfo = (decltype(imports.freeaddrinfo))::GetProcAddress(module, "freeaddrinfo");
  imports.inet_ntop = (decltype(imports.inet_ntop))::GetProcAddress(module, "inet_ntop");
  imports.inet_pton = (decltype(imports.inet_pton))::GetProcAddress(module, "inet_pton");
  imports.getnameinfo = (decltype(imports.getnameinfo))::GetProcAddress(module, "getnameinfo");
  imports.GetNameInfoW = (decltype(imports.GetNameInfoW))::GetProcAddress(module, "GetNameInfoW");
#endif

  return imports;
}

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

  auto module = ::LoadLibraryExW(final_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

  if (!module)
  {
    return std::nullopt;
  }

  return load_wsock_imports(module);
}

export std::optional<wsock_imports> load_peer_ws2()
{
  // Prefer the specific rpc client over a generic injected/system ws2_32.
  // Exhaust LoadLibrary for the specific name before GetModuleHandle(ws2_32),
  // otherwise any process that already linked system ws2_32 would short-circuit.
  auto module = ::GetModuleHandleW(L"ws2_32-rpc-client.dll");

  if (!module)
  {
    auto dir = fs::path(win32::module_ref::current_module().GetModuleFileName()).parent_path();
    module = ::LoadLibraryW((dir / L"ws2_32-rpc-client.dll").c_str());
  }

  if (!module)
  {
    module = ::GetModuleHandleW(L"ws2_32.dll");
  }

  if (!module)
  {
    auto dir = fs::path(win32::module_ref::current_module().GetModuleFileName()).parent_path();
    module = ::LoadLibraryW((dir / L"ws2_32.dll").c_str());
  }

  if (!module)
  {
    return std::nullopt;
  }

  return load_wsock_imports(module);
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

export struct safe_log
{
  std::ostream* stream = nullptr;

  template<class T>
  safe_log& operator<<(T&& value)
  {
    try
    {
      if (stream)
      {
        *stream << std::forward<T>(value);
      }
    }
    catch (...)
    {
    }
    return *this;
  }

  safe_log& operator<<(std::ostream& (*manip)(std::ostream&))
  {
    try
    {
      if (stream && manip)
      {
        *stream << manip;
      }
    }
    catch (...)
    {
    }
    return *this;
  }

  void flush()
  {
    try
    {
      if (stream)
      {
        stream->flush();
      }
    }
    catch (...)
    {
    }
  }
};

export safe_log get_log(std::string_view log_prefix = "networking")
{
  try
  {
    static const std::string stored_log_prefix = std::string{ log_prefix } + ":";

    struct debug_string_buf : public std::stringbuf
    {
    protected:
      int sync() override
      {
        try
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
        }
        catch (...)
        {
          try
          {
            str("");
          }
          catch (...)
          {
          }
        }
        return 0;
      }
    };

    thread_local debug_string_buf buffer{};
    thread_local std::ostream debug_log{ &buffer };

    if (!buffer.view().empty() && buffer.view().back() != '\n')
    {
      debug_log << '\n';
    }

    if (buffer.view().size() > 64)
    {
      debug_log.flush();
    }

    debug_log << stored_log_prefix;
    return safe_log{ &debug_log };
  }
  catch (...)
  {
    return safe_log{};
  }
}

safe_log null_log()
{
  return safe_log{};
}

safe_log log_sampled(std::atomic<std::uint64_t>& counter, std::uint64_t count)
{
  if (counter.fetch_add(1, std::memory_order_relaxed) % count == 0)
  {
    return get_log();
  }
  return null_log();
}

export safe_log log_sampled_read()
{
  static std::atomic<std::uint64_t> counter{};

  return log_sampled(counter, 20011);
}

export safe_log log_sampled_write()
{
  static std::atomic<std::uint64_t> counter{};

  return log_sampled(counter, 5003);
}

export safe_log log_sampled_check()
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

export struct packed_hostent
{
  hostent host;
  std::array<char, 256> name;
  std::array<char*, 1> aliases;
  std::array<char*, 17> addr_list;
  std::array<std::array<char, sizeof(sockaddr)>, 16> addr_data;

  packed_hostent(const hostent& other)
  {
    std::strncpy(name.data(), other.h_name ? other.h_name : "", name.size() - 1);
    name.back() = '\0';
    aliases[0] = nullptr;

    int addr_count = 0;
    for (auto* entry = other.h_addr_list; entry && *entry && addr_count < addr_data.size(); ++entry)
    {
      auto len = std::min<short>(other.h_length, sizeof(sockaddr));
      std::memcpy(addr_data[addr_count].data(), *entry, len);
      addr_list[addr_count] = addr_data[addr_count].data();
      ++addr_count;
    }
    addr_list[addr_count] = nullptr;

    host = hostent{
      .h_name = name.data(),
      .h_aliases = aliases.data(),
      .h_addrtype = other.h_addrtype,
      .h_length = other.h_length,
      .h_addr_list = addr_list.data(),
    };
  }

  packed_hostent(const packed_hostent&) = delete;
  packed_hostent& operator=(const packed_hostent&) = delete;
};
static_assert(sizeof(packed_hostent) <= MAXGETHOSTSTRUCT);

export struct packed_protoent
{
  protoent proto;
  std::array<char, NI_MAXSERV> name;
  std::array<char*, 1> aliases;

  packed_protoent(const protoent& other)
  {
    std::strncpy(name.data(), other.p_name ? other.p_name : "", name.size() - 1);
    name.back() = '\0';
    aliases[0] = nullptr;

    proto = protoent{
      .p_name = name.data(),
      .p_aliases = aliases.data(),
      .p_proto = other.p_proto,
    };
  }

  packed_protoent(const packed_protoent&) = delete;
  packed_protoent& operator=(const packed_protoent&) = delete;
};
static_assert(sizeof(packed_protoent) <= MAXGETHOSTSTRUCT);

export struct packed_servent
{
  servent serv;
  std::array<char, NI_MAXSERV> name;
  std::array<char, NI_MAXSERV> proto;
  std::array<char*, 1> aliases;

  packed_servent(const servent& other)
  {
    std::strncpy(name.data(), other.s_name ? other.s_name : "", name.size() - 1);
    name.back() = '\0';
    std::strncpy(proto.data(), other.s_proto ? other.s_proto : "", proto.size() - 1);
    proto.back() = '\0';
    aliases[0] = nullptr;

    serv = servent{
      .s_name = name.data(),
      .s_aliases = aliases.data(),
      .s_port = other.s_port,
      .s_proto = proto.data(),
    };
  }

  packed_servent(const packed_servent&) = delete;
  packed_servent& operator=(const packed_servent&) = delete;
};
static_assert(sizeof(packed_servent) <= MAXGETHOSTSTRUCT);
