// Thin wsock32 RPC client: forwards all Winsock calls into ws2_32-rpc-client
// (or an injected ws2_32). We solely depend on that peer DLL because dual-DLL
// games (wsock32 + ws2_32) must share one client state and one RPC server owner;
// keeping separate state in this DLL would desync sockets across the two modules.

#include <WinSock2.h>

#include <siege/platform/win/module.hpp>

import std;
import wsock32.shared;

std::optional<wsock_imports> peer;

void ensure_peer();

extern "C" {
int __stdcall siege_WSAStartup(WORD version, LPWSADATA data)
{
  ensure_peer();
  get_log("wsock32-rpc-client") << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version);
  return peer->WSAStartup(version, data);
}

int __stdcall siege_WSACleanup()
{
  ensure_peer();
  get_log() << "siege_WSACleanup";
  return peer->WSACleanup();
}

SOCKET __stdcall siege_socket(int af, int type, int protocol) noexcept
{
  ensure_peer();
  get_log() << "siege_socket af: " << af_to_string(af) << ", type: " << type_to_string(type) << ", protocol: " << protocol_to_string(protocol);
  return peer->socket(af, type, protocol);
}

int __stdcall siege_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen)
{
  ensure_peer();
  get_log() << "siege_setsockopt: " << ws << " " << optname;
  return peer->setsockopt(ws, level, optname, optval, optlen);
}

int __stdcall siege_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen) noexcept
{
  ensure_peer();
  get_log() << "siege_getsockopt" << ws << " " << optname;
  return peer->getsockopt(ws, level, optname, optval, optlen);
}

int __stdcall siege_getsockname(SOCKET ws, sockaddr* name, int* length) noexcept
{
  ensure_peer();
  return peer->getsockname(ws, name, length);
}

int __stdcall siege_getpeername(SOCKET ws, sockaddr* name, int* length) noexcept
{
  ensure_peer();
  return peer->getpeername(ws, name, length);
}

int __stdcall siege_recv(SOCKET ws, char* buf, int len, int flags) noexcept
{
  ensure_peer();
  return peer->recv(ws, buf, len, flags);
}

int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept
{
  ensure_peer();
  return peer->recvfrom(ws, buf, len, flags, from, fromLen);
}

int __stdcall siege_send(SOCKET ws, const char* buf, int len, int flags) noexcept
{
  ensure_peer();
  return peer->send(ws, buf, len, flags);
}

int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept
{
  ensure_peer();
  return peer->sendto(ws, buf, len, flags, to, tolen);
}

int __stdcall siege_ioctlsocket(SOCKET ws, long cmd, u_long* argp)
{
  ensure_peer();
  return peer->ioctlsocket(ws, cmd, argp);
}

int __stdcall siege_bind(SOCKET ws, const sockaddr* addr, int namelen)
{
  ensure_peer();
  return peer->bind(ws, addr, namelen);
}

int __stdcall siege_connect(SOCKET ws, const sockaddr* name, int namelen)
{
  ensure_peer();
  return peer->connect(ws, name, namelen);
}

SOCKET __stdcall siege_accept(SOCKET ws, sockaddr* from, int* fromLen) noexcept
{
  ensure_peer();
  return peer->accept(ws, from, fromLen);
}

int __stdcall siege_listen(SOCKET ws, int backlog)
{
  ensure_peer();
  return peer->listen(ws, backlog);
}

int __stdcall siege_shutdown(SOCKET ws, int how)
{
  ensure_peer();
  return peer->shutdown(ws, how);
}

int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout) noexcept
{
  ensure_peer();
  return peer->select(value, read, write, except, timeout);
}

int __stdcall siege_closesocket(SOCKET ws)
{
  ensure_peer();
  return peer->closesocket(ws);
}

u_long __stdcall siege_htonl(u_long hostlong)
{
  ensure_peer();
  return peer->htonl(hostlong);
}

u_short __stdcall siege_htons(u_short hostshort)
{
  ensure_peer();
  return peer->htons(hostshort);
}

u_long __stdcall siege_ntohl(u_long netlong)
{
  ensure_peer();
  return peer->ntohl(netlong);
}

u_short __stdcall siege_ntohs(u_short netshort)
{
  ensure_peer();
  return peer->ntohs(netshort);
}

unsigned long __stdcall siege_inet_addr(const char* cp)
{
  ensure_peer();
  return peer->inet_addr(cp);
}

char* __stdcall siege_inet_ntoa(in_addr in)
{
  ensure_peer();
  return peer->inet_ntoa(in);
}

hostent* __stdcall siege_gethostbyaddr(const char* addr, int len, int type)
{
  ensure_peer();
  return peer->gethostbyaddr(addr, len, type);
}

hostent* __stdcall siege_gethostbyname(const char* name) noexcept
{
  ensure_peer();
  return peer->gethostbyname(name);
}

int __stdcall siege_gethostname(char* name, int namelen) noexcept
{
  ensure_peer();
  return peer->gethostname(name, namelen);
}

int __stdcall siege_WSAAsyncSelect(SOCKET socket, HWND window, u_int message, long flags)
{
  ensure_peer();
  return peer->WSAAsyncSelect(socket, window, message, flags);
}

HANDLE __stdcall siege_WSAAsyncGetHostByName(HWND window, u_int message, const char* name, char* buffer, int buffer_length)
{
  ensure_peer();
  return peer->WSAAsyncGetHostByName(window, message, name, buffer, buffer_length);
}

int __stdcall siege_WSACancelAsyncRequest(HANDLE request)
{
  ensure_peer();
  return peer->WSACancelAsyncRequest(request);
}

int __stdcall siege_WSAGetLastError() noexcept
{
  ensure_peer();
  return peer->WSAGetLastError();
}

void __stdcall siege_WSASetLastError(int error)
{
  ensure_peer();
  peer->WSASetLastError(error);
}

FARPROC __stdcall siege_WSASetBlockingHook(FARPROC proc)
{
  ensure_peer();
  return peer->WSASetBlockingHook(proc);
}

int __stdcall siege_WSAUnhookBlockingHook()
{
  ensure_peer();
  return peer->WSAUnhookBlockingHook();
}

int __stdcall siege_WSACancelBlockingCall()
{
  ensure_peer();
  return peer->WSACancelBlockingCall();
}

int __stdcall siege___WSAFDIsSet(SOCKET ws, fd_set* set)
{
  ensure_peer();
  return peer->__WSAFDIsSet(ws, set);
}
}

void ensure_peer()
{
  if (peer)
  {
    return;
  }

  get_log("wsock32-rpc-client") << "Loading peer ws2_32 client";
  peer = load_peer_ws2();

  if (!peer)
  {
    get_log() << "Could not load peer ws2_32 client";
    ::ExitProcess(-1);
  }
}
