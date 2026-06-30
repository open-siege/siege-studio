module;

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <siege/platform/win/module.hpp>

export module wsock32.shared;

import std;

extern "C++" bool use_zero_tier();

extern "C" {
int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept;
int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept;
}

namespace fs = std::filesystem;

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

  if (dll_name.contains(L"-"))
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

export std::ostream& get_log(std::filesystem::path log_path = "networking.log")
{
#ifdef _DEBUG
  static std::ofstream file_log(log_path, std::ios::trunc);
#else
  static std::stringstream file_log;
  file_log.str("");
#endif
  return file_log;
}

extern "C" {


hostent* __stdcall siege_gethostbyaddr(const char* addr, int len, int type)
{
  ensure_imports();

  get_log() << "siege_gethostbyaddr\n";

  return imports->gethostbyaddr(addr, len, type);
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
    bool notify_read = flags & FD_READ;
    bool notify_write = flags & FD_WRITE;
    bool notify_oob = flags & FD_OOB;

    if (flags & FD_ACCEPT)
    {
      get_log() << "FD_ACCEPT not supported for siege_WSAAsyncSelect.\n";
    }

    if (flags & FD_CONNECT)
    {
      get_log() << "FD_CONNECT not supported for siege_WSAAsyncSelect.\n";
    }

    if (flags & FD_CLOSE)
    {
      get_log() << "FD_CLOSE not supported for siege_WSAAsyncSelect.\n";
    }

#ifdef USE_WINSOCK2
    if (flags & FD_QOS)
    {
      get_log() << "FD_QOS not supported for siege_WSAAsyncSelect.\n";
    }

    if (flags & FD_ROUTING_INTERFACE_CHANGE)
    {
      get_log() << "FD_ROUTING_INTERFACE_CHANGE not supported for siege_WSAAsyncSelect.\n";
    }

    if (flags & FD_ADDRESS_LIST_CHANGE)
    {
      get_log() << "FD_ADDRESS_LIST_CHANGE not supported for siege_WSAAsyncSelect.\n";
    }
#endif

    ::MessageBoxW(nullptr, L"The game tried to use WSAAsyncSelect, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);

    ::ExitProcess(-1);
  }

  return imports->WSAAsyncSelect(socket, window, message, flags);
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

int __stdcall siege_WSAIoctl(SOCKET s, DWORD controlCode, LPVOID inBuffer, DWORD inBufferCount, LPVOID outBuffer, DWORD outBufferCount, LPDWORD bytesReturned, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use siege_WSAIoctl, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSAIoctl(s, controlCode, inBuffer, inBufferCount, outBuffer, outBufferCount, bytesReturned, overlapped, completionRoutine);
}

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

// TODO implement a version that deals with multiple buffers.
// This is for our first candidate using this API, Alien vs Predator
auto __stdcall siege_WSARecvFrom(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD* flags, sockaddr* from, INT* from_len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    get_log() << "siege_WSARecvFrom called.\n";
    if (overlapped || completion_handler)
    {
      get_log() << "siege_WSARecvFrom is overlapped.\n";

      ::MessageBoxW(nullptr, L"The game tried to use WSARecvFrom, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
      ::ExitProcess(-1);
    }

    if (!buffers)
    {
      // TODO return error here
    }

    for (auto i = 0; i < buffer_count; ++i)
    {
      if (!buffers[i].buf)
      {
        // TODO return error here
      }
    }

    struct span_pair
    {
      std::span<char> from_buffer;
      std::span<char> to_param;
    };

    thread_local std::vector<char> temp_buffer;
    thread_local std::vector<span_pair> span_buffer;

    temp_buffer.reserve(buffer_count);
    temp_buffer.resize(0);

    std::size_t size = 0;
    for (auto i = 0; i < buffer_count; ++i)
    {
      size += buffers[i].len;
    }
    temp_buffer.resize(0);
    temp_buffer.resize(size);

    auto begin = temp_buffer.begin();

    for (auto i = 0; i < buffer_count; ++i)
    {
      span_pair& pair = span_buffer.emplace_back();
      pair.to_param = std::span{ buffers[i].buf, buffers[i].len };
      pair.from_buffer = std::span{ begin, buffers[i].len };

      if (begin + buffers[i].len > temp_buffer.end())
      {
        break;
      }
      std::advance(begin, buffers[i].len);
    }

    auto received_size = siege_recvfrom(socket, temp_buffer.data(), static_cast<int>(temp_buffer.size()), *flags, from, from_len);

    if (received_size == SOCKET_ERROR)
    {
      return SOCKET_ERROR;
    }

    *bytes_received = static_cast<DWORD>(received_size);

    for (auto& pair : span_buffer)
    {
      // TODO assert that sizes are the same.
      std::memcpy(pair.to_param.data(), pair.from_buffer.data(), pair.to_param.size());
    }

    // TODO log and/or reject or deal with MSG_PARTIAL
    // TODO map WSAEWOULDBLOCK to WSA_IO_PENDING
    // TODO make sure there isn't anything else that must go out.

    return 0;
  }

  return imports->WSARecvFrom(socket, buffers, buffer_count, bytes_received, flags, from, from_len, overlapped, completion_handler);
}

auto __stdcall siege_WSARecv(SOCKET ws, LPWSABUF buffers, DWORD bufferCount, LPDWORD numberOfBytesRecvd, LPDWORD flags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine)
{
  if (use_zero_tier())
  {
    return siege_WSARecvFrom(ws, buffers, bufferCount, numberOfBytesRecvd, flags, nullptr, 0, lpOverlapped, completionRoutine);
  }

  return imports->WSARecv(ws, buffers, bufferCount, numberOfBytesRecvd, flags, lpOverlapped, completionRoutine);
}

// TODO implement a version that deals with multiple buffers.
// This is for our first candidate using this API, Alien vs Predator
auto __stdcall siege_WSASendTo(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_sent, DWORD flags, const sockaddr* to, int len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    if (overlapped || completion_handler)
    {
      get_log() << "siege_WSASendTo is overlapped.\n";
      ::MessageBoxW(nullptr, L"The game tried to use WSASendTo, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
      ::ExitProcess(-1);
    }

    thread_local std::vector<char> temp_buffer;

    if (!buffers)
    {
      // TODO return error here
    }

    for (auto i = 0; i < buffer_count; ++i)
    {
      if (!buffers[i].buf)
      {
        // TODO return error here
      }
    }

    std::size_t size = 0;
    for (auto i = 0; i < buffer_count; ++i)
    {
      size += buffers[i].len;
    }
    temp_buffer.reserve(size);
    temp_buffer.resize(0);

    for (auto i = 0; i < buffer_count; ++i)
    {
      temp_buffer.insert(temp_buffer.end(), buffers[i].buf, buffers[i].buf + buffers[i].len);
    }
    // TODO log and/or reject or deal with MSG_PARTIAL

    // TODO map WSAEWOULDBLOCK to WSA_IO_PENDING
    auto sent_size = siege_sendto(socket, temp_buffer.data(), static_cast<int>(temp_buffer.size()), flags, to, len);

    if (sent_size == SOCKET_ERROR)
    {
      return SOCKET_ERROR;
    }

    *bytes_sent = static_cast<DWORD>(sent_size);
    return 0;
  }
  return imports->WSASendTo(socket, buffers, buffer_count, bytes_sent, flags, to, len, overlapped, completion_handler);
}

auto __stdcall siege_WSASend(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_sent, DWORD flags, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    return siege_WSASendTo(socket, buffers, buffer_count, bytes_sent, flags, nullptr, 0, overlapped, completion_handler);
  }

  return imports->WSASend(socket, buffers, buffer_count, bytes_sent, flags, overlapped, completion_handler);
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

auto __stdcall siege_inet_addr(const char* addr)
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
}
