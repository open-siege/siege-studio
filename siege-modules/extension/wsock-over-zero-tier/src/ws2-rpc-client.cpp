#include "ws2-rpc.hpp"
#include <WinSock2.h>
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

void load_system_wsock();
std::ostream& get_log();
std::optional<std::uint64_t> get_zero_tier_network_id();
bool use_zero_tier();

static HWND server = nullptr;

extern "C" {
HMODULE wsock_module = nullptr;
decltype(::WSAStartup)* wsock_WSAStartup = nullptr;
decltype(::WSACleanup)* wsock_WSACleanup = nullptr;
decltype(::socket)* wsock_socket = nullptr;
decltype(::setsockopt)* wsock_setsockopt = nullptr;
decltype(::getsockopt)* wsock_getsockopt = nullptr;
decltype(::getsockname)* wsock_getsockname = nullptr;
decltype(::getpeername)* wsock_getpeername = nullptr;
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
decltype(::WSAAsyncGetHostByName)* wsock_WSAAsyncGetHostByName = nullptr;
decltype(::WSACancelAsyncRequest)* wsock_WSACancelAsyncRequest = nullptr;
decltype(::WSAAsyncSelect)* wsock_WSAAsyncSelect = nullptr;
decltype(::WSAStringToAddressA)* wsock_WSAStringToAddressA = nullptr;
decltype(::WSAGetOverlappedResult)* wsock_WSAGetOverlappedResult = nullptr;
decltype(::WSACreateEvent)* wsock_WSACreateEvent = nullptr;
decltype(::WSAResetEvent)* wsock_WSAResetEvent = nullptr;
decltype(::WSACloseEvent)* wsock_WSACloseEvent = nullptr;
decltype(::WSAWaitForMultipleEvents)* wsock_WSAWaitForMultipleEvents = nullptr;
decltype(::WSASendTo)* wsock_WSASendTo = nullptr;
decltype(::WSASend)* wsock_WSASend = nullptr;
decltype(::WSARecvFrom)* wsock_WSARecvFrom = nullptr;
decltype(::WSAEventSelect)* wsock_WSAEventSelect = nullptr;
decltype(::WSAEnumNetworkEvents)* wsock_WSAEnumNetworkEvents = nullptr;

int __stdcall siege_WSAStartup(WORD version, LPWSADATA data)
{
  load_system_wsock();
  get_log() << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version) << '\n';
  auto result = wsock_WSAStartup(version, data);

  if (auto network_id = get_zero_tier_network_id(); network_id)
  {
    if (result == 0)
    {
      wsock_WSACleanup();
    }

    // TODO start helper process
    // TODO get handle to window
    // TODO preallocate storage
  }

  get_log().flush();
  return result;
}

int __stdcall siege_WSACleanup()
{
  load_system_wsock();
  get_log() << "siege_WSACleanup" << '\n';

  if (use_zero_tier())
  {
    return 0;
  }

  return wsock_WSACleanup();
}

SOCKET __stdcall siege_socket(int af, int type, int protocol)
{
  load_system_wsock();
  get_log() << "siege_socket af: " << af_to_string(af) << ", type: " << type_to_string(type) << ", protocol: " << protocol_to_string(protocol) << ", thread: " << GetCurrentThreadId() << '\n';

  if (use_zero_tier())
  {
    // TODO this is just conceptual code
    socket_params params{ .address_family = af, .type = type, .protocol = protocol };

    auto out_handle = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(params));

    void* data = ::GlobalLock(out_handle);
    std::memcpy(data, &params, sizeof(params));
    ::GlobalUnlock(out_handle);
    DWORD_PTR new_socket = 0;
    ::SendMessageTimeoutW(server, socket_params::message_id, 0, (LPARAM)out_handle, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &new_socket);

    ::GlobalFree(out_handle);

    if ((SOCKET)new_socket == INVALID_SOCKET)
    {
      DWORD_PTR last_error = WSAESOCKTNOSUPPORT;
      ::SendMessageTimeoutW(server, general_params::get_last_error_message_id, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &last_error);
      wsock_WSASetLastError((int)last_error);
      return INVALID_SOCKET;
    }

    return (SOCKET)new_socket;
  }


  auto result = wsock_socket(af, type, protocol);
  get_log() << "Created winsock socket successfully (" << (int)result << ")" << '\n';
  return result;
}

int __stdcall siege_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen)
{
  if (use_zero_tier())
  {
  }

  auto result = wsock_setsockopt(ws, level, optname, optval, optlen);

  if (result != 0)
  {
    get_log() << "setsockopt WSAGetLastError " << wsock_WSAGetLastError() << '\n';
  }

  return result;
}

int __stdcall siege_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen)
{
  if (use_zero_tier())
  {
  }

  auto result = wsock_getsockopt(ws, level, optname, optval, optlen);

  if (result != 0)
  {
    get_log() << "getsockopt WSAGetLastError " << wsock_WSAGetLastError() << '\n';
  }

  return result;
}


int __stdcall siege_bind(SOCKET ws, const sockaddr* addr, int namelen)
{
  get_log() << "siege_bind " << '\n';

  if (use_zero_tier())
  {
  }
  auto result = wsock_bind(ws, addr, namelen);

  get_log() << "Bind call has error " << wsock_WSAGetLastError() << "\n";

  return result;
}

int __stdcall siege_ioctlsocket(SOCKET ws, long cmd, u_long* argp)
{
  get_log() << "siege_ioctlsocket, cmd: " << ioctl_cmd_to_string(cmd) << '\n';
  if (use_zero_tier())
  {
  }
  auto result = wsock_ioctlsocket(ws, cmd, argp);

  get_log() << "siege_ioctlsocket finished" << '\n';

  return result;
}


int __stdcall siege_recv(SOCKET ws, char* buf, int len, int flags)
{
  if (use_zero_tier())
  {
  }
  return wsock_recv(ws, buf, len, flags);
}

int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen)
{
  if (use_zero_tier())
  {
    // TODO this is just conceptual code
    recvfrom_params params{.flags = flags};
    
    if (buf && len)
    {
      params.buffer = ::GlobalAlloc(GMEM_MOVEABLE, len);
      params.buffer_length = len;
      void* data = ::GlobalLock(params.buffer);
      std::memcpy(data, &params, sizeof(params));
      ::GlobalUnlock(params.buffer);
    }

    auto out_handle = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(params));

    void* data = ::GlobalLock(out_handle);
    std::memcpy(data, &params, sizeof(params));
    ::GlobalUnlock(out_handle);
    DWORD_PTR received = SOCKET_ERROR;
    ::SendMessageTimeoutW(server, recvfrom_params::message_id, (WPARAM)ws, (LPARAM)out_handle, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &received);

    if (received != SOCKET_ERROR)
    {
      void* data = ::GlobalLock(out_handle);
      std::memcpy(&params, data, sizeof(params));
      ::GlobalUnlock(out_handle);

      if (buf && len)
      {
        auto* result = ::GlobalLock(params.buffer);
        std::memcpy(buf, result, len);
        ::GlobalUnlock(params.buffer);
      }

      if (from && fromLen)
      {
        std::memcpy(from, params.from_address, *fromLen);
      }
    }

    ::GlobalFree(out_handle);

    if (received == SOCKET_ERROR)
    {
      DWORD_PTR last_error = WSAECONNRESET;
      ::SendMessageTimeoutW(server, general_params::get_last_error_message_id, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &last_error);
      wsock_WSASetLastError((int)last_error);
      return SOCKET_ERROR;
    }

    return (int)received;
  }

  return wsock_recvfrom(ws, buf, len, flags, from, fromLen);
}

int __stdcall siege_getsockname(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getsockname\n";
  if (use_zero_tier())
  {
  }
  return wsock_getsockname(ws, name, length);
}

int __stdcall siege_getpeername(SOCKET ws, sockaddr* name, int* length)
{
  get_log() << "siege_getpeername\n";
  if (use_zero_tier())
  {
  }
  return wsock_getpeername(ws, name, length);
}

int __stdcall siege_listen(SOCKET ws, int backlog)
{
  get_log() << "siege_listen\n";
  if (use_zero_tier())
  {
  }
  return wsock_listen(ws, backlog);
}

SOCKET __stdcall siege_accept(SOCKET ws, sockaddr* name, int* namelen)
{
  get_log() << "siege_accept\n";
  if (use_zero_tier())
  {
  }

  return wsock_accept(ws, name, namelen);
}

int __stdcall siege_connect(SOCKET ws, const sockaddr* name, int namelen)
{
  get_log() << "siege_connect" << '\n';

  if (use_zero_tier())
  {
  }
  return wsock_connect(ws, name, namelen);
}

int __stdcall siege_send(SOCKET ws, const char* buf, int len, int flags)
{
  if (use_zero_tier())
  {
  }
  return wsock_send(ws, buf, len, flags);
}

int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
  if (use_zero_tier())
  {
  }
  return wsock_sendto(ws, buf, len, flags, to, tolen);
}

int __stdcall siege_shutdown(SOCKET ws, int how)
{
  get_log() << "siege_shutdown\n";
  if (use_zero_tier())
  {
  }
  return wsock_shutdown(ws, how);
}

int __stdcall siege_closesocket(SOCKET ws)
{
  get_log() << "siege_closesocket\n";
  if (use_zero_tier())
  {
  }
  return wsock_closesocket(ws);
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout)
{
  if (use_zero_tier())
  {
  }
  return wsock_select(value, read, write, except, timeout);
}

int __stdcall siege___WSAFDIsSet(SOCKET ws, fd_set* set)
{
  if (use_zero_tier())
  {
  }

  return wsock___WSAFDIsSet(ws, set);
}


hostent* __stdcall siege_gethostbyaddr(const char* addr, int len, int type)
{
  load_system_wsock();

  get_log() << "siege_gethostbyaddr\n";

  return wsock_gethostbyaddr(addr, len, type);
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

  return wsock_gethostbyname(name);
}

auto __stdcall siege_WSAAsyncGetHostByName(HWND window, u_int message, const char* name, char* buffer, int buffer_length)
{
  if (use_zero_tier())
  {
    get_log() << "siege_WSAAsyncGetHostByName.\n";
    ::MessageBoxW(nullptr, L"The game tried to use WSAAsyncGetHostByName, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return wsock_WSAAsyncGetHostByName(window, message, name, buffer, buffer_length);
}

auto __stdcall siege_WSACancelAsyncRequest(HANDLE request)
{
  if (use_zero_tier())
  {
    get_log() << "siege_WSACancelAsyncRequest.\n";
    ::MessageBoxW(nullptr, L"The game tried to use WSACancelAsyncRequest, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return wsock_WSACancelAsyncRequest(request);
}

auto __stdcall siege_WSAAsyncSelect(SOCKET socket, HWND window, u_int message, long flags)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSAAsyncSelect, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);

    ::ExitProcess(-1);
  }

  return wsock_WSAAsyncSelect(socket, window, message, flags);
}

auto __stdcall siege_WSAGetOverlappedResult(SOCKET socket, OVERLAPPED* overlapped, DWORD* transfer, BOOL wait, DWORD* flags)
{
  if (use_zero_tier())
  {
    get_log() << "siege_WSAGetOverlappedResult called. quitting.\n";
    ::ExitProcess(-1);
    // cancel get host by name task
  }
  return wsock_WSAGetOverlappedResult(socket, overlapped, transfer, wait, flags);
}

auto __stdcall siege_WSARecvFrom(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD* flags, sockaddr* from, INT* from_len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSARecvFrom, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return wsock_WSARecvFrom(socket, buffers, buffer_count, bytes_received, flags, from, from_len, overlapped, completion_handler);
}

auto __stdcall siege_WSASend(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD flags, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSASend, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return wsock_WSASend(socket, buffers, buffer_count, bytes_received, flags, overlapped, completion_handler);
}

auto __stdcall siege_WSASendTo(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD flags, const sockaddr* to, int len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSASendTo, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }
  return wsock_WSASendTo(socket, buffers, buffer_count, bytes_received, flags, to, len, overlapped, completion_handler);
}

auto __stdcall siege_WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSAEventSelect, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }

  return wsock_WSAEventSelect(s, hEventObject, lNetworkEvents);
}

auto __stdcall siege_WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents)
{
  if (use_zero_tier())
  {
    ::MessageBoxW(nullptr, L"The game tried to use WSAEnumNetworkEvents, which is currently not implemented. Please disable Zero Tier in the settings.", L"Function not implemented", MB_ICONERROR);
    ::ExitProcess(-1);
  }
  return wsock_WSAEnumNetworkEvents(s, hEventObject, lpNetworkEvents);
}

auto __stdcall siege_WSACreateEvent()
{
  load_system_wsock();
  return wsock_WSACreateEvent();
}

auto __stdcall siege_WSAResetEvent(HANDLE event)
{
  return wsock_WSAResetEvent(event);
}

auto __stdcall siege_WSACloseEvent(HANDLE event)
{
  return wsock_WSACloseEvent(event);
}

auto __stdcall siege_WSAWaitForMultipleEvents(DWORD event_count, const HANDLE* events, BOOL wait_all, DWORD timeout, BOOL alertable)
{
  return wsock_WSAWaitForMultipleEvents(event_count, events, wait_all, timeout, alertable);
}

auto __stdcall siege_gethostname(char* name, int namelen)
{
  load_system_wsock();
  return wsock_gethostname(name, namelen);
}

auto __stdcall siege_WSAGetLastError()
{
  load_system_wsock();
  return wsock_WSAGetLastError();
}

auto __stdcall siege_htonl(u_long value)
{
  load_system_wsock();
  return wsock_htonl(value);
}

auto __stdcall siege_htons(u_short value)
{
  load_system_wsock();
  return wsock_htons(value);
}

auto __stdcall siege_ntohl(u_long value)
{
  load_system_wsock();
  return wsock_ntohl(value);
}

auto __stdcall siege_ntohs(u_short value)
{
  load_system_wsock();
  return wsock_ntohs(value);
}

auto __stdcall siege_inet_addr(const char* addr)
{
  load_system_wsock();
  return wsock_inet_addr(addr);
}

auto __stdcall siege_inet_ntoa(in_addr in)
{
  load_system_wsock();
  return wsock_inet_ntoa(in);
}

auto __stdcall siege_WSAStringToAddressA(LPSTR address_str, INT family, LPWSAPROTOCOL_INFOA info, LPSOCKADDR out_address, LPINT out_len)
{
  load_system_wsock();
  return wsock_WSAStringToAddressA(address_str, family, info, out_address, out_len);
}

auto __stdcall siege_WSASetLastError(int error)
{
  load_system_wsock();
  return wsock_WSASetLastError(error);
}

auto __stdcall siege_WSASetBlockingHook(FARPROC proc)
{
  load_system_wsock();
  get_log() << "siege_WSASetBlockingHook " << '\n';
  return wsock_WSASetBlockingHook(proc);
}

auto __stdcall siege_WSAUnhookBlockingHook()
{
  load_system_wsock();
  get_log() << "siege_WSAUnhookBlockingHook " << '\n';
  return wsock_WSAUnhookBlockingHook();
}

auto __stdcall siege_WSACancelBlockingCall()
{
  load_system_wsock();
  get_log() << "siege_WSACancelBlockingCall " << '\n';
  return wsock_WSACancelBlockingCall();
}
}

void load_system_wsock()
{
  if (wsock_module)
  {
    return;
  }

  get_log() << "load_system_wsock " << '\n';
  auto module_path = win32::module_ref::current_module().GetModuleFileName();

  auto dll_name = fs::path(module_path).filename().wstring();

  get_log() << "current module name: " << fs::path(module_path).filename().string() << '\n';

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

  get_log() << "final dll path: " << final_path << '\n';

  wsock_module = LoadLibraryExW(final_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

  if (!wsock_module)
  {
    return;
  }

  get_log() << "System module loaded. Getting proc addresses" << '\n';

  wsock_WSAStartup = (decltype(wsock_WSAStartup))::GetProcAddress(wsock_module, "WSAStartup");
  wsock_WSACleanup = (decltype(wsock_WSACleanup))::GetProcAddress(wsock_module, "WSACleanup");
  wsock_socket = (decltype(wsock_socket))::GetProcAddress(wsock_module, "socket");
  wsock_setsockopt = (decltype(wsock_setsockopt))::GetProcAddress(wsock_module, "setsockopt");
  wsock_getsockname = (decltype(wsock_getsockname))::GetProcAddress(wsock_module, "getsockname");
  wsock_getpeername = (decltype(wsock_getpeername))::GetProcAddress(wsock_module, "getpeername");
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
  wsock_WSAAsyncGetHostByName = (decltype(wsock_WSAAsyncGetHostByName))::GetProcAddress(wsock_module, "WSAAsyncGetHostByName");
  wsock_WSACancelAsyncRequest = (decltype(wsock_WSACancelAsyncRequest))::GetProcAddress(wsock_module, "WSACancelAsyncRequest");
  wsock_WSASetBlockingHook = (decltype(wsock_WSASetBlockingHook))::GetProcAddress(wsock_module, "WSASetBlockingHook");
  wsock_WSAUnhookBlockingHook = (decltype(wsock_WSAUnhookBlockingHook))::GetProcAddress(wsock_module, "WSAUnhookBlockingHook");
  wsock_WSACancelBlockingCall = (decltype(wsock_WSACancelBlockingCall))::GetProcAddress(wsock_module, "WSACancelBlockingCall");
  wsock_WSAAsyncSelect = (decltype(wsock_WSAAsyncSelect))::GetProcAddress(wsock_module, "WSAAsyncSelect");
  wsock_WSAStringToAddressA = (decltype(wsock_WSAStringToAddressA))::GetProcAddress(wsock_module, "WSAStringToAddressA");
  wsock_WSAGetOverlappedResult = (decltype(wsock_WSAGetOverlappedResult))::GetProcAddress(wsock_module, "WSAGetOverlappedResult");
  wsock_WSACreateEvent = (decltype(wsock_WSACreateEvent))::GetProcAddress(wsock_module, "WSACreateEvent");
  wsock_WSAResetEvent = (decltype(wsock_WSAResetEvent))::GetProcAddress(wsock_module, "WSAResetEvent");
  wsock_WSACloseEvent = (decltype(wsock_WSACloseEvent))::GetProcAddress(wsock_module, "WSACloseEvent");
  wsock_WSAWaitForMultipleEvents = (decltype(wsock_WSAWaitForMultipleEvents))::GetProcAddress(wsock_module, "WSAWaitForMultipleEvents");
  wsock_WSASendTo = (decltype(wsock_WSASendTo))::GetProcAddress(wsock_module, "WSASendTo");
  wsock_WSASend = (decltype(wsock_WSASend))::GetProcAddress(wsock_module, "WSASend");
  wsock_WSARecvFrom = (decltype(wsock_WSARecvFrom))::GetProcAddress(wsock_module, "WSARecvFrom");
  wsock_WSAEventSelect = (decltype(wsock_WSAEventSelect))::GetProcAddress(wsock_module, "WSAEventSelect");
  wsock_WSAEnumNetworkEvents = (decltype(wsock_WSAEnumNetworkEvents))::GetProcAddress(wsock_module, "WSAEnumNetworkEvents");
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

bool use_zero_tier()
{
  return get_zero_tier_network_id().has_value();
}

std::string af_to_string(int af)
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

std::string type_to_string(int type)
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

std::string protocol_to_string(int protocol)
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

std::string ioctl_cmd_to_string(long cmd)
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