#include <WinSock2.h>
#include <ws2tcpip.h>
#include <wsnwlink.h>
#include <siege/platform/win/module.hpp>

import std;
import wsock32.shared;

namespace fs = std::filesystem;

extern "C" {
int __stdcall backend_WSAStartup(WORD version, LPWSADATA data) noexcept
{
  ensure_imports();
  get_log("backend.ws2_32") << "siege_WSAStartup " << (int)LOBYTE(version) << " " << (int)HIBYTE(version);
  return imports->WSAStartup(version, data);
}

int __stdcall backend_WSACleanup() noexcept
{
  ensure_imports();
  return imports->WSACleanup();
}

SOCKET __stdcall backend_socket(int af, int type, int protocol) noexcept
{
  ensure_imports();
  get_log() << "siege_socket af: " << af_to_string(af) << ", type: " << type_to_string(type) << ", protocol: " << protocol_to_string(protocol) << ", thread: " << GetCurrentThreadId();

  auto socket = imports->socket(af, type, protocol);

  u_long non_blocking = 1u;

  if (imports->ioctlsocket(socket, FIONBIO, &non_blocking) == SOCKET_ERROR)
  {
    imports->closesocket(socket);
    return INVALID_SOCKET;
  }

  BOOL value = TRUE;
  int size = sizeof(value);

  imports->setsockopt(socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&value), size);

  return socket;
}

int __stdcall backend_setsockopt(SOCKET ws, int level, int optname, const char* optval, int optlen) noexcept
{
  get_log() << "siege_setsockopt: " << ws << " " << optname;
  return imports->setsockopt(ws, level, optname, optval, optlen);
}

int __stdcall backend_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen) noexcept
{
  get_log() << "siege_getsockopt" << ws << " " << optname;
  return imports->getsockopt(ws, level, optname, optval, optlen);
}

int __stdcall backend_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept
{
  return imports->recvfrom(ws, buf, len, flags, from, fromLen);
}

int __stdcall backend_getsockname(SOCKET ws, sockaddr* name, int* length) noexcept
{
  get_log() << "siege_getsockname\n";
  return imports->getsockname(ws, name, length);
}

int __stdcall backend_getpeername(SOCKET ws, sockaddr* name, int* length) noexcept
{
  return imports->getpeername(ws, name, length);
}

int __stdcall backend_ioctlsocket(SOCKET ws, long cmd, u_long* argp) noexcept
{
  get_log() << "siege_ioctlsocket, cmd: " << ioctl_cmd_to_string(cmd);
  return imports->ioctlsocket(ws, cmd, argp);
}

int __stdcall backend_listen(SOCKET ws, int backlog) noexcept
{
  get_log() << "siege_listen\n";

  return imports->listen(ws, backlog);
}

SOCKET __stdcall backend_accept(SOCKET ws, sockaddr* name, int* namelen) noexcept
{
  log_sampled_check() << "siege_accept\n";
  auto result = imports->accept(ws, name, namelen);

  u_long non_blocking = 1u;

  if (imports->ioctlsocket(result, FIONBIO, &non_blocking) == SOCKET_ERROR)
  {
    imports->closesocket(result);
    return INVALID_SOCKET;
  }

  return result;
}

int __stdcall backend_connect(SOCKET ws, const sockaddr* name, int namelen) noexcept
{
  get_log() << "siege_connect " << ws;

  return imports->connect(ws, name, namelen);
}

int __stdcall backend_bind(SOCKET ws, const sockaddr* addr, int namelen) noexcept
{
  get_log() << "siege_bind " << ws << std::endl;

  return imports->bind(ws, addr, namelen);
}

int __stdcall backend_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept
{
  return imports->sendto(ws, buf, len, flags, to, tolen);
}

int __stdcall backend_shutdown(SOCKET ws, int how) noexcept
{
  get_log() << "siege_shutdown\n";
  return imports->shutdown(ws, how);
}

int __stdcall backend_closesocket(SOCKET ws) noexcept
{
  get_log() << "siege_closesocket\n";
  return imports->closesocket(ws);
}

int __stdcall backend_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout) noexcept
{
  return imports->select(value, read, write, except, timeout);
}

int __stdcall backend___WSAFDIsSet(SOCKET ws, fd_set* set) noexcept
{
  return imports->__WSAFDIsSet(ws, set);
}

hostent* __stdcall backend_gethostbyname(const char* name) noexcept
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
  return imports->gethostbyname(name);
}
}