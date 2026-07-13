module;

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <siege/platform/win/module.hpp>
#include <cassert>

export module wsock32.shared.client;

import wsock32.shared;
import std;

extern "C++" bool use_custom_backend();

extern "C" {
int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept;
int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept;
SOCKET __stdcall siege_socket(int af, int type, int protocol) noexcept;
int __stdcall siege_ioctlsocket(SOCKET ws, long cmd, u_long* argp) noexcept;
int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout) noexcept;
}

// TODO come up with a reasonable return type
void queue_event_notification();
void queue_window_notification();

namespace fs = std::filesystem;

// TODO will need shared memory
// because ws2_32 and wsock32 may be loaded and
// has to track this state
export struct socket_handle_info
{
  void insert(SOCKET socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    handles.emplace(socket, socket_context{});
  }

  void erase(SOCKET socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    handles.erase(socket);
  }

  void set_virtual_blocking(SOCKET socket, bool should_block)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return;
    }

    item->second.is_virtual_blocking = should_block;
  }

  void set_overlapped(SOCKET socket, bool is_overlapped)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return;
    }

    item->second.is_overlapped = is_overlapped;
  }

  bool is_virtual_blocking(SOCKET socket) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto item = handles.find(socket);

    if (item == handles.end())
    {
      return false;
    }

    return item->second.is_virtual_blocking;
  }

  bool is_overlapped(SOCKET socket) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto item = handles.find(socket);

    if (item == handles.end())
    {
      return false;
    }

    return item->second.is_overlapped;
  }

  bool contains(SOCKET socket) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return handles.contains(socket);
  }

private:
  struct socket_context
  {
    // backend sockets are non-blocking,
    // so we have to block by default on the client-side
    bool is_virtual_blocking = true;

    // purely client-side. backends shouldn't know what overlapping is.
    // definitely non-standard bsd.
    bool is_overlapped = false;
  };

  std::map<SOCKET, socket_context> handles;

  mutable std::shared_mutex mutex;
};

export socket_handle_info& get_socket_handles()
{
  static socket_handle_info info{};
  return info;
}


extern "C" {
hostent* __stdcall siege_gethostbyaddr(const char* addr, int len, int type)
{
  ensure_imports();

  get_log() << "siege_gethostbyaddr\n";

  return imports->gethostbyaddr(addr, len, type);
}

HANDLE __stdcall siege_WSAAsyncGetHostByName(HWND window, u_int message, const char* name, char* buffer, int buffer_length)
{
  get_log() << "siege_WSAAsyncGetHostByName.\n";
  if (!use_custom_backend())
  {
    return imports->WSAAsyncGetHostByName(window, message, name, buffer, buffer_length);
  }

  get_log() << "siege_WSAAsyncGetHostByName not supported.";
  get_log().flush();
  imports->WSASetLastError(WSAENETDOWN);

  return nullptr;
}

auto __stdcall siege_WSACancelAsyncRequest(HANDLE request)
{
  get_log() << "siege_WSACancelAsyncRequest";
  if (!use_custom_backend())
  {
    return imports->WSACancelAsyncRequest(request);
  }

  get_log() << "siege_WSACancelAsyncRequest not supported.";
  get_log().flush();
  imports->WSASetLastError(WSAENETDOWN);

  return SOCKET_ERROR;
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
  if (!use_custom_backend())
  {
    return imports->WSASocketW(af, type, protocol, lpProtocolInfo, g, dwFlags);
  }

  if (lpProtocolInfo)
  {
    imports->WSASetLastError(WSAEPROVIDERFAILEDINIT);
    return INVALID_SOCKET;
  }

  constexpr static auto invalid_flags = std::array<DWORD, 4>{ { WSA_FLAG_MULTIPOINT_C_ROOT, WSA_FLAG_MULTIPOINT_C_LEAF, WSA_FLAG_MULTIPOINT_D_ROOT, WSA_FLAG_MULTIPOINT_D_LEAF } };

  for (auto flag : invalid_flags)
  {
    if (dwFlags & flag)
    {
      imports->WSASetLastError(WSAEINVAL);
      return INVALID_SOCKET;
    }
  }

  if (g != 0)
  {
    imports->WSASetLastError(WSAEINVAL);
    return INVALID_SOCKET;
  }

  auto result = siege_socket(af, type, protocol);

  if (result != INVALID_SOCKET && dwFlags & WSA_FLAG_OVERLAPPED)
  {
    get_socket_handles().set_overlapped(result, true);
  }

  return result;
}

SOCKET __stdcall siege_WSASocketA(int af, int type, int protocol, LPWSAPROTOCOL_INFOA lpProtocolInfo, GROUP g, DWORD dwFlags)
{
  if (!use_custom_backend())
  {
    return imports->WSASocketA(af, type, protocol, lpProtocolInfo, g, dwFlags);
  }

  get_log() << "siege_WSASocketA " << '\n';

  if (lpProtocolInfo)
  {
    imports->WSASetLastError(WSAEPROVIDERFAILEDINIT);
    return INVALID_SOCKET;
  }

  return siege_WSASocketW(af, type, protocol, nullptr, g, dwFlags);
}

int __stdcall siege_WSAIoctl(SOCKET s, DWORD controlCode, LPVOID inBuffer, DWORD inBufferCount, LPVOID outBuffer, DWORD outBufferCount, LPDWORD bytesReturned, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSAIoctl(s, controlCode, inBuffer, inBufferCount, outBuffer, outBufferCount, bytesReturned, overlapped, completionRoutine);
  }

  if (get_socket_handles().is_overlapped(s) && (overlapped || completionRoutine))
  {
    WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (!(controlCode == FIONBIO || controlCode == FIONREAD))
  {
    imports->WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (inBufferCount > 4)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  u_long temp{};

  if (inBuffer && inBufferCount <= 4)
  {
    std::memcpy(&temp, inBuffer, inBufferCount);
  }
  auto result = siege_ioctlsocket(s, controlCode, &temp);

  auto out_size = std::clamp<std::size_t>(outBufferCount, 0, sizeof(temp));

  if (result == 0 && outBuffer && outBufferCount >= 4)
  {
    std::memcpy(outBuffer, &temp, out_size);
  }

  if (result == 0 && bytesReturned)
  {
    *bytesReturned = static_cast<DWORD>(out_size);
  }

  return result;
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
  if (!use_custom_backend())
  {
    return imports->WSAGetOverlappedResult(socket, overlapped, transfer, wait, flags);
  }

  get_log() << "siege_WSAGetOverlappedResult called. not supported.\n";
  get_log().flush();
  imports->WSASetLastError(WSAENETDOWN);

  return FALSE;
}

auto __stdcall siege_WSARecvFrom(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD* flags, sockaddr* from, INT* from_len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSARecvFrom(socket, buffers, buffer_count, bytes_received, flags, from, from_len, overlapped, completion_handler);
  }

  get_log() << "siege_WSARecvFrom called.\n";

  if (get_socket_handles().is_overlapped(socket) && (overlapped || completion_handler))
  {
    get_log() << "siege_WSARecvFrom is overlapped. Not supported.\n";
    get_log().flush();
    imports->WSASetLastError(WSAEOPNOTSUPP);

    return SOCKET_ERROR;
  }

  if (!flags)
  {
    imports->WSASetLastError(WSAEFAULT);
    return SOCKET_ERROR;
  }

  if (*flags & MSG_PARTIAL)
  {
    get_log() << "siege_WSARecvFrom MSG_PARTIAL requested. Not supported.";
    imports->WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (!buffers)
  {
    imports->WSASetLastError(WSAEFAULT);
    return SOCKET_ERROR;
  }

  for (auto i = 0; i < buffer_count; ++i)
  {
    if (!buffers[i].buf && buffers[i].len > 0)
    {
      imports->WSASetLastError(WSAEINVAL);
      return SOCKET_ERROR;
    }
  }

  // null bytes_received is only allowed when overlapped is set.
  // but since we already reject that, it has to be supplied.
  if (!bytes_received)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  constexpr static auto max_int = static_cast<std::size_t>(std::numeric_limits<int>::max());
  std::size_t size = 0;
  for (auto i = 0; i < buffer_count; ++i)
  {
    auto len = static_cast<std::size_t>(buffers[i].len);

    if (len > max_int - size)
    {
      imports->WSASetLastError(WSAEMSGSIZE);
      return SOCKET_ERROR;
    }

    size += len;
  }

  struct span_pair
  {
    std::span<char> from_buffer;
    std::span<char> to_param;
  };

  thread_local std::vector<char> temp_buffer;
  thread_local std::vector<span_pair> span_buffer;

  span_buffer.reserve(buffer_count);
  span_buffer.clear();
  temp_buffer.resize(size);

  auto begin = temp_buffer.begin();

  for (auto i = 0; i < buffer_count; ++i)
  {
    if (buffers[i].len == 0)
    {
      continue;
    }

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

  if (span_buffer.empty())
  {
    return 0;
  }

  std::size_t remaining = static_cast<std::size_t>(received_size);
  for (auto& pair : span_buffer)
  {
    auto to_copy = std::min(pair.to_param.size(), remaining);

    if (to_copy == 0)
    {
      continue;
    }

    assert(pair.to_param.size() == pair.from_buffer.size());
    std::memcpy(pair.to_param.data(), pair.from_buffer.data(), to_copy);
    remaining -= to_copy;
  }

  return 0;
}

auto __stdcall siege_WSARecv(SOCKET ws, LPWSABUF buffers, DWORD bufferCount, LPDWORD numberOfBytesRecvd, LPDWORD flags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine)
{
  if (!use_custom_backend())
  {
    return imports->WSARecv(ws, buffers, bufferCount, numberOfBytesRecvd, flags, lpOverlapped, completionRoutine);
  }

  return siege_WSARecvFrom(ws, buffers, bufferCount, numberOfBytesRecvd, flags, nullptr, 0, lpOverlapped, completionRoutine);
}

auto __stdcall siege_WSASendTo(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_sent, DWORD flags, const sockaddr* to, int len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSASendTo(socket, buffers, buffer_count, bytes_sent, flags, to, len, overlapped, completion_handler);
  }

  if (get_socket_handles().is_overlapped(socket) && (overlapped || completion_handler))
  {
    get_log() << "siege_WSASendTo is overlapped. Not supported.";
    imports->WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (flags & MSG_PARTIAL)
  {
    get_log() << "siege_WSASendTo MSG_PARTIAL requested. Not supported.";
    imports->WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (!buffers)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  for (auto i = 0; i < buffer_count; ++i)
  {
    if (!buffers[i].buf && buffers[i].len > 0)
    {
      imports->WSASetLastError(WSAEINVAL);
      return SOCKET_ERROR;
    }
  }

  // null bytes_sent is only allowed when overlapped is set.
  // but since we already reject that, it has to be supplied.
  if (!bytes_sent)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  constexpr static auto max_int = static_cast<std::size_t>(std::numeric_limits<int>::max());
  std::size_t size = 0;
  for (auto i = 0; i < buffer_count; ++i)
  {
    auto len = static_cast<std::size_t>(buffers[i].len);

    if (len > max_int - size)
    {
      imports->WSASetLastError(WSAEMSGSIZE);
      return SOCKET_ERROR;
    }

    size += len;
  }

  thread_local std::vector<char> temp_buffer;
  temp_buffer.reserve(size);
  temp_buffer.resize(0);

  for (auto i = 0; i < buffer_count; ++i)
  {
    if (!buffers[i].buf || buffers[i].len == 0)
    {
      continue;
    }

    temp_buffer.insert(temp_buffer.end(), buffers[i].buf, buffers[i].buf + buffers[i].len);
  }

  auto sent_size = siege_sendto(socket, temp_buffer.data(), static_cast<int>(temp_buffer.size()), flags, to, len);

  if (sent_size == SOCKET_ERROR)
  {
    return SOCKET_ERROR;
  }

  *bytes_sent = static_cast<DWORD>(sent_size);
  return 0;
}

auto __stdcall siege_WSASend(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_sent, DWORD flags, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSASend(socket, buffers, buffer_count, bytes_sent, flags, overlapped, completion_handler);
  }

  return siege_WSASendTo(socket, buffers, buffer_count, bytes_sent, flags, nullptr, 0, overlapped, completion_handler);
}

auto __stdcall siege_WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents) noexcept
{
  get_log() << "siege_WSAEventSelect";

  if (!use_custom_backend())
  {
    return imports->WSAEventSelect(s, hEventObject, lNetworkEvents);
  }

  get_log() << "siege_WSAEventSelect not supported.";
  get_log().flush();
  imports->WSASetLastError(WSAENETDOWN);

  return SOCKET_ERROR;
}
#endif 
auto __stdcall siege_WSAAsyncSelect(SOCKET socket, HWND window, u_int message, long flags)
{
  if (!use_custom_backend())
  {
    return imports->WSAAsyncSelect(socket, window, message, flags);
  }

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

  get_log() << "siege_WSAAsyncSelect not supported.";
  get_log().flush();
  imports->WSASetLastError(WSAENETDOWN);
  return SOCKET_ERROR;
}

#ifdef USE_WINSOCK2
auto __stdcall siege_WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents) noexcept
{
  get_log() << "siege_WSAEnumNetworkEvents";
  if (!use_custom_backend())
  {
    return imports->WSAEnumNetworkEvents(s, hEventObject, lpNetworkEvents);
  }

  get_log() << "siege_WSAEnumNetworkEvents not supported.";
  get_log().flush();
  imports->WSASetLastError(WSAENETDOWN);

  return SOCKET_ERROR;
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
  if (!name || namelen <= 0)
  {
    imports->WSASetLastError(WSAEFAULT);
    return SOCKET_ERROR;
  }
  DWORD size = static_cast<DWORD>(namelen);

  if (::GetComputerNameExA(ComputerNamePhysicalDnsHostname, name, &size))
  {
    return 0;
  }

  imports->WSASetLastError(WSAENETDOWN);
  return SOCKET_ERROR;

  // get_log() << "siege_gethostname.";
  // ensure_imports();
  // return imports->gethostname(name, namelen);
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


// TODO this is all just sketching at this point
// to get the correct logic
void worker()
{
  struct socket_work
  {
    SOCKET socket;
    int type = SOCK_STREAM;
    int requested_flags = 0;
    int notified_flags = 0;// either sent to the window or given out via enum events
    int enabled_flags = 0;// enabled until first notification, then it needs a relevant function to be called

    enum tcp_state
    {
        unset,
        listening, // server-side - FD_ACCEPT
        accepted, // server-side - FD_CLOSE
        connected // client-side - FD_CONNECT - FD_CLOSE
    };
  };

  std::vector<socket_work> sockets;

  fd_set read_set{};
  fd_set write_set{};
  fd_set except_set{};

  for (auto& work : sockets)
  {
    if (work.enabled_flags & FD_READ)
    {
      FD_SET(work.socket, &read_set);
    }

    if (work.enabled_flags & FD_WRITE)
    {
      FD_SET(work.socket, &write_set);
    }

    if (work.enabled_flags & FD_OOB)
    {
      FD_SET(work.socket, &except_set);
    }
  }

  timeval time{};
  auto count = siege_select(3, &read_set, &write_set, &except_set, &time);

  // FD_READ + FD_WRITE + FD_OOB map to select

  // FD_ACCEPT + FD_CONNECT + FD_CLOSE can map to select
  // but with extra info. we need to do them separately in order for it to make sense

  // FD_ACCEPT == listen socket state + read
  // FD_CONNECT == connecting socket state + write
  // FD_CLOSE == read + msg peak with size of 0
}

void queue_event_notification()
{
}

void queue_window_notification()
{
}