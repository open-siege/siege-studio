module;

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <siege/platform/win/module.hpp>

export module wsock32.shared.client;

import wsock32.shared;
import std;

extern "C++" bool use_custom_backend();

extern "C" {
int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept;
int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept;
}

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

extern "C" {
hostent* __stdcall siege_gethostbyaddr(const char* addr, int len, int type)
{
  ensure_imports();

  get_log() << "siege_gethostbyaddr\n";

  return imports->gethostbyaddr(addr, len, type);
}

auto __stdcall siege_WSAAsyncGetHostByName(HWND window, u_int message, const char* name, char* buffer, int buffer_length)
{
  if (use_custom_backend())
  {
    get_log() << "siege_WSAAsyncGetHostByName.\n";
    ::MessageBoxW(nullptr, L"The game tried to use WSAAsyncGetHostByName, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSAAsyncGetHostByName(window, message, name, buffer, buffer_length);
}

auto __stdcall siege_WSACancelAsyncRequest(HANDLE request)
{
  if (use_custom_backend())
  {
    get_log() << "siege_WSACancelAsyncRequest.\n";
    ::MessageBoxW(nullptr, L"The game tried to use WSACancelAsyncRequest, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSACancelAsyncRequest(request);
}

auto __stdcall siege_WSAAsyncSelect(SOCKET socket, HWND window, u_int message, long flags)
{
  if (use_custom_backend())
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


int __stdcall siege_recv(SOCKET ws, char* buf, int len, int flags) noexcept
{
  if (use_custom_backend())
  {
    return siege_recvfrom(ws, buf, len, flags, nullptr, nullptr);
  }

  return imports->recv(ws, buf, len, flags);
}


int __stdcall siege_send(SOCKET ws, const char* buf, int len, int flags) noexcept
{
  if (use_custom_backend())
  {
    return siege_sendto(ws, buf, len, flags, nullptr, 0);
  }

  return imports->send(ws, buf, len, flags);
}

#ifdef USE_WINSOCK2

SOCKET __stdcall siege_WSASocketW(int af, int type, int protocol, LPWSAPROTOCOL_INFOW lpProtocolInfo, GROUP g, DWORD dwFlags)
{
  get_log() << "siege_WSASocketW " << '\n';
  if (use_custom_backend())
  {
    ::MessageBoxW(nullptr, L"The game tried to use siege_WSASocketW, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSASocketW(af, type, protocol, lpProtocolInfo, g, dwFlags);
}

int __stdcall siege_WSAIoctl(SOCKET s, DWORD controlCode, LPVOID inBuffer, DWORD inBufferCount, LPVOID outBuffer, DWORD outBufferCount, LPDWORD bytesReturned, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine)
{
  if (use_custom_backend())
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
  if (use_custom_backend())
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
  if (use_custom_backend())
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
  if (use_custom_backend())
  {
    return siege_WSARecvFrom(ws, buffers, bufferCount, numberOfBytesRecvd, flags, nullptr, 0, lpOverlapped, completionRoutine);
  }

  return imports->WSARecv(ws, buffers, bufferCount, numberOfBytesRecvd, flags, lpOverlapped, completionRoutine);
}

// TODO implement a version that deals with multiple buffers.
// This is for our first candidate using this API, Alien vs Predator
auto __stdcall siege_WSASendTo(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_sent, DWORD flags, const sockaddr* to, int len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_custom_backend())
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

auto __stdcall siege_WSASend(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_sent, DWORD flags, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler) noexcept
{
  if (use_custom_backend())
  {
    return siege_WSASendTo(socket, buffers, buffer_count, bytes_sent, flags, nullptr, 0, overlapped, completion_handler);
  }

  return imports->WSASend(socket, buffers, buffer_count, bytes_sent, flags, overlapped, completion_handler);
}

auto __stdcall siege_WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents) noexcept
{
  get_log() << "siege_WSAEventSelect";
  if (use_custom_backend())
  {
    get_log() << "siege_WSAEventSelect not supported. Showing message and closing app.";
    get_log().flush();
    ::MessageBoxW(nullptr, L"The game tried to use WSAEventSelect, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return imports->WSAEventSelect(s, hEventObject, lNetworkEvents);
}

auto __stdcall siege_WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents) noexcept
{
  get_log() << "siege_WSAEnumNetworkEvents";
  if (use_custom_backend())
  {
    get_log() << "siege_WSAEnumNetworkEvents not supported. Showing message and closing app.";
    get_log().flush();
    ::MessageBoxW(nullptr, L"The game tried to use WSAEnumNetworkEvents, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }
  return imports->WSAEnumNetworkEvents(s, hEventObject, lpNetworkEvents);
}

auto __stdcall siege_WSACreateEvent() noexcept
{
  ensure_imports();
  return imports->WSACreateEvent();
}

auto __stdcall siege_WSAResetEvent(HANDLE event) noexcept
{
  return imports->WSAResetEvent(event);
}

auto __stdcall siege_WSACloseEvent(HANDLE event) noexcept
{
  return imports->WSACloseEvent(event);
}

auto __stdcall siege_WSAWaitForMultipleEvents(DWORD event_count, const HANDLE* events, BOOL wait_all, DWORD timeout, BOOL alertable) noexcept
{
  return imports->WSAWaitForMultipleEvents(event_count, events, wait_all, timeout, alertable);
}
#endif


auto __stdcall siege_gethostname(char* name, int namelen) noexcept
{
  ensure_imports();
  return imports->gethostname(name, namelen);
}

auto __stdcall siege_WSAGetLastError() noexcept
{
  ensure_imports();
  return imports->WSAGetLastError();
}

auto __stdcall siege_htonl(u_long value) noexcept
{
  ensure_imports();
  return imports->htonl(value);
}

auto __stdcall siege_htons(u_short value) noexcept
{
  ensure_imports();
  return imports->htons(value);
}

auto __stdcall siege_ntohl(u_long value) noexcept
{
  ensure_imports();
  return imports->ntohl(value);
}

auto __stdcall siege_ntohs(u_short value) noexcept
{
  ensure_imports();
  return imports->ntohs(value);
}

auto __stdcall siege_inet_addr(const char* addr) noexcept
{
  ensure_imports();
  return imports->inet_addr(addr);
}

auto __stdcall siege_inet_ntoa(in_addr in) noexcept
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
