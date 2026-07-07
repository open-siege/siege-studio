#include <WinSock.h>
#include <siege/platform/win/module.hpp>
#include <cassert>

import std;
import wsock32.shared;

namespace fs = std::filesystem;

HMODULE wsock_module = nullptr;
decltype(::WSAStartup)* wsock_WSAStartup = nullptr;
decltype(::WSACleanup)* wsock_WSACleanup = nullptr;
decltype(::socket)* wsock_socket = nullptr;
decltype(::closesocket)* wsock_closesocket = nullptr;
decltype(::shutdown)* wsock_shutdown = nullptr;
decltype(::setsockopt)* wsock_setsockopt = nullptr;
decltype(::getsockopt)* wsock_getsockopt = nullptr;
decltype(::getsockname)* wsock_getsockname = nullptr;
decltype(::getpeername)* wsock_getpeername = nullptr;
decltype(::gethostbyname)* wsock_gethostbyname = nullptr;
decltype(::recvfrom)* wsock_recvfrom = nullptr;
decltype(::sendto)* wsock_sendto = nullptr;
decltype(::ioctlsocket)* wsock_ioctlsocket = nullptr;
decltype(::bind)* wsock_bind = nullptr;
decltype(::accept)* wsock_accept = nullptr;
decltype(::listen)* wsock_listen = nullptr;
decltype(::select)* wsock_select = nullptr;
decltype(::htons)* wsock_htons = nullptr;
decltype(::__WSAFDIsSet)* wsock___WSAFDIsSet = nullptr;
decltype(::WSAGetLastError)* wsock_WSAGetLastError = nullptr;
decltype(::WSASetLastError)* wsock_WSASetLastError = nullptr;

void load_local_wsock();

void test_blocking_udp();
void test_non_blocking_udp();

int main()
{
  // ZERO_TIER_NETWORK_ID
  load_local_wsock();
  // we will do tests without zero tier, to make sure that the fallback works as expected.

  // TODO validate enviornment variables for zero tier
  // TODO clear zero tier env variables

  test_blocking_udp();

  ::SetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", "wsock-backend-ws2_32.dll");
  test_blocking_udp();

  std::cout << "Tests finished\n";
  //  TODO set zero tier env variables again
  //  test_udp();

  // TODO all good - now test tcp


  // then we will do the same tests with.
}


void test_blocking_udp()
{
  auto data = WSAData{

  };

  auto result = wsock_WSAStartup(MAKEWORD(1, 1), &data);
  assert(result == 0);

  auto client_socket = wsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  assert(client_socket != INVALID_SOCKET);

  BOOL should_broadcast = TRUE;
  result = wsock_setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&should_broadcast), static_cast<int>(sizeof(should_broadcast)));
  assert(result != INVALID_SOCKET);

  DWORD timeout = 1000;
  wsock_setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout)));
  assert(result != INVALID_SOCKET);

  wsock_setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout)));
  assert(result != SOCKET_ERROR);

  int param_size = static_cast<int>(sizeof(should_broadcast));
  should_broadcast = FALSE;
  result = wsock_getsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&should_broadcast), &param_size);
  assert(result != SOCKET_ERROR);
  assert(should_broadcast == TRUE);

  param_size = static_cast<int>(sizeof(timeout));
  timeout = 0;
  result = wsock_getsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), &param_size);
  auto last_error = wsock_WSAGetLastError();
  assert(result != SOCKET_ERROR);
  assert(timeout == 1000);


  auto server_socket = wsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  assert(server_socket != INVALID_SOCKET);

  wsock_setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout)));
  assert(result != SOCKET_ERROR);

  wsock_setsockopt(server_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout)));
  assert(result != SOCKET_ERROR);

  sockaddr_in server_addr = {
    .sin_family = AF_INET,
    .sin_port = wsock_htons(9090)
  };
//  server_addr.sin_addr.s_addr = INADDR_LOOPBACK;

  result = wsock_bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr)));
  assert(result != SOCKET_ERROR);

  auto pending_task = std::async(std::launch::async, [](SOCKET server_socket) {
      std::array<char, 4> buffer{};
      sockaddr_in addr{};
      int addr_len = static_cast<std::size_t>(sizeof(addr));
      auto received = wsock_recvfrom(server_socket, buffer.data(), static_cast<int>(buffer.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
      auto last_error = wsock_WSAGetLastError();
      assert(last_error == 0);
      assert(received == 4);
      assert(addr_len == sizeof(addr));
      assert(addr.sin_family == AF_INET);
      assert(std::string_view{ buffer } ==  "ping");

      std::string reply = "pong";
      auto sent = wsock_sendto(server_socket, reply.data(), static_cast<int>(reply.size()), 0, reinterpret_cast<sockaddr*>(&addr), addr_len);
      last_error = wsock_WSAGetLastError();
      assert(last_error == 0);
      assert(sent == 4); }, server_socket);

  std::string message = "ping";

  sockaddr_in broadcast{
    .sin_family = AF_INET,
    .sin_port = wsock_htons(9090),
  };
  broadcast.sin_addr.s_addr = INADDR_BROADCAST;

  auto sent_size = wsock_sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&broadcast), static_cast<int>(sizeof(broadcast)));
  assert(sent_size == static_cast<int>(message.size()));

  pending_task.wait();

  sockaddr_in addr{};
  int addr_len = static_cast<std::size_t>(sizeof(addr));
  auto received = wsock_recvfrom(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  last_error = wsock_WSAGetLastError();
  assert(last_error == 0);
  assert(received == 4);
  assert(addr_len == sizeof(addr));
  assert(addr.sin_family == AF_INET);
  assert(addr.sin_port == wsock_htons(9090));
  assert(message == "pong");

  result = wsock_closesocket(server_socket);
  assert(result != SOCKET_ERROR);

  result = wsock_closesocket(client_socket);
  assert(result != SOCKET_ERROR);

  result = wsock_WSACleanup();
  assert(result == 0);
}


void load_local_wsock()
{
  if (wsock_module)
  {
    return;
  }

  auto module_path = win32::module_ref::current_module().GetModuleFileName();

  auto target_path = fs::path(module_path).parent_path() / L"wsock32-in-proc-client.dll";
  fs::path lib_path;

  wsock_module = ::LoadLibraryExW(target_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

  if (!wsock_module)
  {
    throw std::runtime_error("Could not load module");
    return;
  }

  wsock_WSAStartup = (decltype(wsock_WSAStartup))::GetProcAddress(wsock_module, "WSAStartup");
  wsock_WSACleanup = (decltype(wsock_WSACleanup))::GetProcAddress(wsock_module, "WSACleanup");
  wsock_socket = (decltype(wsock_socket))::GetProcAddress(wsock_module, "socket");
  wsock_closesocket = (decltype(wsock_closesocket))::GetProcAddress(wsock_module, "closesocket");
  wsock_shutdown = (decltype(wsock_shutdown))::GetProcAddress(wsock_module, "shutdown");
  wsock_setsockopt = (decltype(wsock_setsockopt))::GetProcAddress(wsock_module, "setsockopt");
  wsock_getsockname = (decltype(wsock_getsockname))::GetProcAddress(wsock_module, "getsockname");
  wsock_getpeername = (decltype(wsock_getpeername))::GetProcAddress(wsock_module, "getpeername");
  wsock_getsockopt = (decltype(wsock_getsockopt))::GetProcAddress(wsock_module, "getsockopt");
  wsock_gethostbyname = (decltype(wsock_gethostbyname))::GetProcAddress(wsock_module, "gethostbyname");
  wsock_recvfrom = (decltype(wsock_recvfrom))::GetProcAddress(wsock_module, "recvfrom");
  wsock_sendto = (decltype(wsock_sendto))::GetProcAddress(wsock_module, "sendto");
  wsock_ioctlsocket = (decltype(wsock_ioctlsocket))::GetProcAddress(wsock_module, "ioctlsocket");
  wsock_bind = (decltype(wsock_bind))::GetProcAddress(wsock_module, "bind");
  wsock_accept = (decltype(wsock_accept))::GetProcAddress(wsock_module, "accept");
  wsock_listen = (decltype(wsock_listen))::GetProcAddress(wsock_module, "listen");
  wsock_select = (decltype(wsock_select))::GetProcAddress(wsock_module, "select");
  wsock_htons = (decltype(wsock_htons))::GetProcAddress(wsock_module, "htons");
  wsock_WSAGetLastError = (decltype(wsock_WSAGetLastError))::GetProcAddress(wsock_module, "WSAGetLastError");
  wsock_WSASetLastError = (decltype(wsock_WSASetLastError))::GetProcAddress(wsock_module, "WSASetLastError");
  wsock___WSAFDIsSet = (decltype(wsock___WSAFDIsSet))::GetProcAddress(wsock_module, "__WSAFDIsSet");
}