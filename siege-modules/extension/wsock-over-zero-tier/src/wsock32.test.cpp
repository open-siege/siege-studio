#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <WinSock2.h>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/win/module.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <future>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

namespace fs = std::filesystem;

struct wsock_api
{
  win32::module_ref module;

  decltype(::WSAStartup)* WSAStartup = nullptr;
  decltype(::WSACleanup)* WSACleanup = nullptr;
  decltype(::socket)* socket = nullptr;
  decltype(::closesocket)* closesocket = nullptr;
  decltype(::setsockopt)* setsockopt = nullptr;
  decltype(::getsockopt)* getsockopt = nullptr;
  decltype(::getsockname)* getsockname = nullptr;
  decltype(::getpeername)* getpeername = nullptr;
  decltype(::recvfrom)* recvfrom = nullptr;
  decltype(::sendto)* sendto = nullptr;
  decltype(::bind)* bind = nullptr;
  decltype(::connect)* connect = nullptr;
  decltype(::listen)* listen = nullptr;
  decltype(::accept)* accept = nullptr;
  decltype(::htons)* htons = nullptr;
  decltype(::ioctlsocket)* ioctlsocket = nullptr;
  decltype(::select)* select = nullptr;
  decltype(::WSAGetLastError)* WSAGetLastError = nullptr;
  decltype(::WSACreateEvent)* WSACreateEvent = nullptr;
  decltype(::WSASetEvent)* WSASetEvent = nullptr;
  decltype(::WSAResetEvent)* WSAResetEvent = nullptr;
  decltype(::WSACloseEvent)* WSACloseEvent = nullptr;
  decltype(::WSAWaitForMultipleEvents)* WSAWaitForMultipleEvents = nullptr;
  decltype(::WSAEventSelect)* WSAEventSelect = nullptr;
  decltype(::WSAEnumNetworkEvents)* WSAEnumNetworkEvents = nullptr;
  decltype(::WSAAsyncSelect)* WSAAsyncSelect = nullptr;
  decltype(::WSASocketW)* WSASocketW = nullptr;
  decltype(::WSARecvFrom)* WSARecvFrom = nullptr;
  decltype(::WSASendTo)* WSASendTo = nullptr;
  decltype(::WSAGetOverlappedResult)* WSAGetOverlappedResult = nullptr;
};

wsock_api load_client(const char* dll_name);
void terminate_rpc_server_if_running();
void test_socket_options(wsock_api& api, WORD version);
void test_blocking_udp(wsock_api& api, WORD version, u_short port);
void test_nonblocking_udp_select(wsock_api& api, WORD version, u_short port);
void test_udp_event_select(wsock_api& api, WORD version, u_short port);
void test_tcp_event_select_connect(wsock_api& api, WORD version, u_short port);
void test_tcp_event_select_connect_refused(wsock_api& api, WORD version, u_short port);
void test_event_select_close_stops_watch(wsock_api& api, WORD version, u_short port);
void test_udp_async_select(wsock_api& api, WORD version, u_short port);
void test_udp_overlapped_poll(wsock_api& api, WORD version, u_short port);
void test_udp_overlapped_event_getresult(wsock_api& api, WORD version, u_short port);
void test_udp_overlapped_event_wait(wsock_api& api, WORD version, u_short port);
void test_udp_overlapped_apc(wsock_api& api, WORD version, u_short port);

struct rpc_server_cleanup_listener : Catch::EventListenerBase
{
  using EventListenerBase::EventListenerBase;

  void testRunEnded(Catch::TestRunStats const&) override
  {
    terminate_rpc_server_if_running();
  }
};

CATCH_REGISTER_LISTENER(rpc_server_cleanup_listener)

TEST_CASE("wsock32-in-proc-client", "[wsock32][in-proc]")
{
  SECTION("system fallback")
  {
    ::SetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", nullptr);

    auto api = load_client("wsock32-in-proc-client.dll");
    test_socket_options(api, MAKEWORD(1, 1));
    test_blocking_udp(api, MAKEWORD(1, 1), 19090);
    test_nonblocking_udp_select(api, MAKEWORD(1, 1), 19190);
    test_udp_async_select(api, MAKEWORD(1, 1), 19390);
  }

  SECTION("ws2_32 backend")
  {
    ::SetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", "wsock-backend-ws2_32.dll");

    auto api = load_client("wsock32-in-proc-client.dll");
    test_socket_options(api, MAKEWORD(1, 1));
    test_blocking_udp(api, MAKEWORD(1, 1), 19091);
    test_nonblocking_udp_select(api, MAKEWORD(1, 1), 19191);
    test_udp_async_select(api, MAKEWORD(1, 1), 19391);

    ::SetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", nullptr);
  }
}

TEST_CASE("wsock32-rpc-client", "[wsock32][rpc]")
{
  SECTION("system fallback")
  {
    ::SetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", nullptr);

    auto api = load_client("wsock32-rpc-client.dll");
    test_socket_options(api, MAKEWORD(1, 1));
    test_blocking_udp(api, MAKEWORD(1, 1), 19092);
    test_nonblocking_udp_select(api, MAKEWORD(1, 1), 19192);
    test_udp_async_select(api, MAKEWORD(1, 1), 19392);
  }

  SECTION("ws2_32 backend")
  {
    // Restart so the server process inherits SIEGE_WSOCK_BACKEND.
    terminate_rpc_server_if_running();
    ::SetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", "wsock-backend-ws2_32.dll");

    auto api = load_client("wsock32-rpc-client.dll");
    test_socket_options(api, MAKEWORD(1, 1));
    test_blocking_udp(api, MAKEWORD(1, 1), 19093);
    test_nonblocking_udp_select(api, MAKEWORD(1, 1), 19193);
    test_udp_async_select(api, MAKEWORD(1, 1), 19393);
  }
}

TEST_CASE("ws2_32-rpc-client", "[ws2_32][rpc]")
{
  SECTION("system fallback")
  {
    ::SetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", nullptr);

    auto api = load_client("ws2_32-rpc-client.dll");
    test_socket_options(api, MAKEWORD(2, 2));
    test_blocking_udp(api, MAKEWORD(2, 2), 19094);
    test_nonblocking_udp_select(api, MAKEWORD(2, 2), 19194);
    test_udp_event_select(api, MAKEWORD(2, 2), 19294);
    test_tcp_event_select_connect(api, MAKEWORD(2, 2), 19694);
    test_tcp_event_select_connect_refused(api, MAKEWORD(2, 2), 19894);
    test_event_select_close_stops_watch(api, MAKEWORD(2, 2), 19794);
    test_udp_async_select(api, MAKEWORD(2, 2), 19394);
    test_udp_overlapped_poll(api, MAKEWORD(2, 2), 19494);
    test_udp_overlapped_event_getresult(api, MAKEWORD(2, 2), 19495);
    test_udp_overlapped_event_wait(api, MAKEWORD(2, 2), 19496);
    test_udp_overlapped_apc(api, MAKEWORD(2, 2), 19497);
  }

  SECTION("ws2_32 backend")
  {
    terminate_rpc_server_if_running();
    ::SetEnvironmentVariableA("SIEGE_WSOCK_BACKEND", "wsock-backend-ws2_32.dll");

    auto api = load_client("ws2_32-rpc-client.dll");
    test_socket_options(api, MAKEWORD(2, 2));
    test_blocking_udp(api, MAKEWORD(2, 2), 19095);
    test_nonblocking_udp_select(api, MAKEWORD(2, 2), 19195);
    test_udp_event_select(api, MAKEWORD(2, 2), 19295);
    test_tcp_event_select_connect(api, MAKEWORD(2, 2), 19695);
    test_tcp_event_select_connect_refused(api, MAKEWORD(2, 2), 19895);
    test_event_select_close_stops_watch(api, MAKEWORD(2, 2), 19795);
    test_udp_async_select(api, MAKEWORD(2, 2), 19395);
    test_udp_overlapped_poll(api, MAKEWORD(2, 2), 19594);
    test_udp_overlapped_event_getresult(api, MAKEWORD(2, 2), 19595);
    test_udp_overlapped_event_wait(api, MAKEWORD(2, 2), 19596);
    test_udp_overlapped_apc(api, MAKEWORD(2, 2), 19597);
  }
}

void test_socket_options(wsock_api& api, WORD version)
{
  WSAData info{};
  REQUIRE(api.WSAStartup(version, &info) == 0);

  auto udp_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(udp_socket != INVALID_SOCKET);

  int value = 256;
  REQUIRE(api.setsockopt(udp_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&value), sizeof(value)) != SOCKET_ERROR);

  value = 0;
  int size = sizeof(value);
  REQUIRE(api.getsockopt(udp_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&value), &size) != SOCKET_ERROR);
  REQUIRE(size == sizeof(value));
  REQUIRE(value == 256);

  value = 0;
  size = sizeof(value);
  REQUIRE(api.getsockopt(udp_socket, SOL_SOCKET, SO_TYPE, reinterpret_cast<char*>(&value), &size) != SOCKET_ERROR);
  REQUIRE(size == sizeof(value));
  REQUIRE(value == SOCK_DGRAM);

  sockaddr_in temp{};
  size = sizeof(temp);
  REQUIRE(api.getsockname(udp_socket, reinterpret_cast<sockaddr*>(&temp), &size) == SOCKET_ERROR);

  auto last_error = api.WSAGetLastError();
  REQUIRE((last_error == WSAEINVAL || last_error == WSAENOTCONN));

  REQUIRE(api.closesocket(udp_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_blocking_udp(wsock_api& api, WORD version, u_short port)
{
  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(client_socket != INVALID_SOCKET);

  DWORD timeout = 1000;
  REQUIRE(api.setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout))) != SOCKET_ERROR);
  REQUIRE(api.setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout))) != SOCKET_ERROR);

  int param_size = static_cast<int>(sizeof(timeout));
  timeout = 0;
  REQUIRE(api.getsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), &param_size) != SOCKET_ERROR);
  REQUIRE(timeout == 1000);

  auto server_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(server_socket != INVALID_SOCKET);

  REQUIRE(api.setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout))) != SOCKET_ERROR);
  REQUIRE(api.setsockopt(server_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout))) != SOCKET_ERROR);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

  auto pending_task = std::async(std::launch::async, [&api](SOCKET server_socket) {
    struct recv_reply
    {
      int received = SOCKET_ERROR;
      int recv_error = 0;
      int addr_len = 0;
      short family = 0;
      std::array<char, 4> buffer{};
      int sent = SOCKET_ERROR;
      int send_error = 0;
    } result;

    sockaddr_in addr{};
    result.addr_len = static_cast<int>(sizeof(addr));
    result.received = api.recvfrom(server_socket, result.buffer.data(), static_cast<int>(result.buffer.size()), 0, reinterpret_cast<sockaddr*>(&addr), &result.addr_len);
    result.recv_error = api.WSAGetLastError();
    result.family = addr.sin_family;

    if (result.received == 4 && std::string_view{ result.buffer.data(), result.buffer.size() } == "ping")
    {
      std::string reply = "pong";
      result.sent = api.sendto(server_socket, reply.data(), static_cast<int>(reply.size()), 0, reinterpret_cast<sockaddr*>(&addr), result.addr_len);
      result.send_error = api.WSAGetLastError();
    }

    return result;
  },
    server_socket);

  std::string message = "ping";
  sockaddr_in localhost{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  auto sent_size = api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&localhost), static_cast<int>(sizeof(localhost)));
  REQUIRE(sent_size == static_cast<int>(message.size()));

  auto reply = pending_task.get();
  REQUIRE(reply.recv_error == 0);
  REQUIRE(reply.received == 4);
  REQUIRE(reply.addr_len == static_cast<int>(sizeof(sockaddr_in)));
  REQUIRE(reply.family == AF_INET);
  REQUIRE(std::string_view{ reply.buffer.data(), reply.buffer.size() } == "ping");
  REQUIRE(reply.send_error == 0);
  REQUIRE(reply.sent == 4);

  sockaddr_in addr{};
  int addr_len = static_cast<int>(sizeof(addr));
  auto received = api.recvfrom(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  REQUIRE(api.WSAGetLastError() == 0);
  REQUIRE(received == 4);
  REQUIRE(addr_len == static_cast<int>(sizeof(addr)));
  REQUIRE(addr.sin_family == AF_INET);
  REQUIRE(addr.sin_port == api.htons(port));
  REQUIRE(message == "pong");

  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_nonblocking_udp_select(wsock_api& api, WORD version, u_short port)
{
  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(client_socket != INVALID_SOCKET);

  auto server_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(server_socket != INVALID_SOCKET);

  u_long non_blocking = 1;
  REQUIRE(api.ioctlsocket(client_socket, FIONBIO, &non_blocking) == 0);
  REQUIRE(api.ioctlsocket(server_socket, FIONBIO, &non_blocking) == 0);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(server_socket, &read_set);
  timeval zero{ .tv_sec = 0, .tv_usec = 0 };
  REQUIRE(api.select(0, &read_set, nullptr, nullptr, &zero) == 0);

  std::array<char, 4> peek{};
  sockaddr_in addr{};
  int addr_len = static_cast<int>(sizeof(addr));
  REQUIRE(api.recvfrom(server_socket, peek.data(), static_cast<int>(peek.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len) == SOCKET_ERROR);
  REQUIRE(api.WSAGetLastError() == WSAEWOULDBLOCK);

  std::string message = "ping";
  sockaddr_in localhost{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  auto sent_size = api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&localhost), static_cast<int>(sizeof(localhost)));
  REQUIRE(sent_size == static_cast<int>(message.size()));

  FD_ZERO(&read_set);
  FD_SET(server_socket, &read_set);
  timeval wait{ .tv_sec = 1, .tv_usec = 0 };
  REQUIRE(api.select(0, &read_set, nullptr, nullptr, &wait) == 1);
  REQUIRE(FD_ISSET(server_socket, &read_set));

  addr = {};
  addr_len = static_cast<int>(sizeof(addr));
  auto received = api.recvfrom(server_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  REQUIRE(received == 4);
  REQUIRE(addr_len == static_cast<int>(sizeof(sockaddr_in)));
  REQUIRE(addr.sin_family == AF_INET);
  REQUIRE(message == "ping");

  std::string reply = "pong";
  sent_size = api.sendto(server_socket, reply.data(), static_cast<int>(reply.size()), 0, reinterpret_cast<const sockaddr*>(&addr), addr_len);
  REQUIRE(sent_size == static_cast<int>(reply.size()));

  FD_ZERO(&read_set);
  FD_SET(client_socket, &read_set);
  wait = { .tv_sec = 1, .tv_usec = 0 };
  REQUIRE(api.select(0, &read_set, nullptr, nullptr, &wait) == 1);
  REQUIRE(FD_ISSET(client_socket, &read_set));

  addr = {};
  addr_len = static_cast<int>(sizeof(addr));
  received = api.recvfrom(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  REQUIRE(received == 4);
  REQUIRE(addr_len == static_cast<int>(sizeof(addr)));
  REQUIRE(addr.sin_family == AF_INET);
  REQUIRE(addr.sin_port == api.htons(port));
  REQUIRE(message == "pong");

  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_udp_event_select(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.WSACreateEvent);
  REQUIRE(api.WSASetEvent);
  REQUIRE(api.WSAResetEvent);
  REQUIRE(api.WSACloseEvent);
  REQUIRE(api.WSAWaitForMultipleEvents);
  REQUIRE(api.WSAEventSelect);
  REQUIRE(api.WSAEnumNetworkEvents);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(client_socket != INVALID_SOCKET);

  auto server_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(server_socket != INVALID_SOCKET);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

  auto server_event = api.WSACreateEvent();
  REQUIRE(server_event != WSA_INVALID_EVENT);
  auto client_event = api.WSACreateEvent();
  REQUIRE(client_event != WSA_INVALID_EVENT);

  REQUIRE(api.WSAEventSelect(server_socket, server_event, FD_READ) == 0);
  REQUIRE(api.WSAEventSelect(client_socket, client_event, FD_READ) == 0);

  REQUIRE(api.WSAWaitForMultipleEvents(1, &server_event, FALSE, 0, FALSE) == WSA_WAIT_TIMEOUT);

  REQUIRE(api.WSASetEvent(server_event) == TRUE);
  REQUIRE(api.WSAWaitForMultipleEvents(1, &server_event, FALSE, 0, FALSE) == WSA_WAIT_EVENT_0);
  REQUIRE(api.WSAResetEvent(server_event) == TRUE);
  REQUIRE(api.WSAWaitForMultipleEvents(1, &server_event, FALSE, 0, FALSE) == WSA_WAIT_TIMEOUT);

  std::string message = "ping";
  sockaddr_in localhost{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  auto sent_size = api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&localhost), static_cast<int>(sizeof(localhost)));
  REQUIRE(sent_size == static_cast<int>(message.size()));

  REQUIRE(api.WSAWaitForMultipleEvents(1, &server_event, FALSE, 1000, FALSE) == WSA_WAIT_EVENT_0);

  WSANETWORKEVENTS network_events{};
  REQUIRE(api.WSAEnumNetworkEvents(server_socket, server_event, &network_events) == 0);
  REQUIRE(network_events.lNetworkEvents & FD_READ);

  sockaddr_in addr{};
  int addr_len = static_cast<int>(sizeof(addr));
  auto received = api.recvfrom(server_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  REQUIRE(received == 4);
  REQUIRE(addr.sin_family == AF_INET);
  REQUIRE(message == "ping");

  std::string reply = "pong";
  sent_size = api.sendto(server_socket, reply.data(), static_cast<int>(reply.size()), 0, reinterpret_cast<const sockaddr*>(&addr), addr_len);
  REQUIRE(sent_size == static_cast<int>(reply.size()));

  REQUIRE(api.WSAWaitForMultipleEvents(1, &client_event, FALSE, 1000, FALSE) == WSA_WAIT_EVENT_0);

  network_events = {};
  REQUIRE(api.WSAEnumNetworkEvents(client_socket, client_event, &network_events) == 0);
  REQUIRE(network_events.lNetworkEvents & FD_READ);

  addr = {};
  addr_len = static_cast<int>(sizeof(addr));
  received = api.recvfrom(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  REQUIRE(received == 4);
  REQUIRE(addr.sin_family == AF_INET);
  REQUIRE(addr.sin_port == api.htons(port));
  REQUIRE(message == "pong");

  REQUIRE(api.WSAEventSelect(server_socket, nullptr, 0) == 0);
  REQUIRE(api.WSAEventSelect(client_socket, nullptr, 0) == 0);
  REQUIRE(api.WSACloseEvent(server_event) == TRUE);
  REQUIRE(api.WSACloseEvent(client_event) == TRUE);
  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

// Celtic Kings (and dpwsockx): after WSAEventSelect(FD_CONNECT), connect may
// return SOCKET_ERROR only with WSAEWOULDBLOCK — WSAEINPROGRESS aborts join.
void test_tcp_event_select_connect(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.connect);
  REQUIRE(api.listen);
  REQUIRE(api.accept);
  REQUIRE(api.WSACreateEvent);
  REQUIRE(api.WSACloseEvent);
  REQUIRE(api.WSAWaitForMultipleEvents);
  REQUIRE(api.WSAEventSelect);
  REQUIRE(api.WSAEnumNetworkEvents);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto server_socket = api.socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  REQUIRE(server_socket != INVALID_SOCKET);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);
  REQUIRE(api.listen(server_socket, 1) != SOCKET_ERROR);

  auto client_socket = api.socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  REQUIRE(client_socket != INVALID_SOCKET);

  auto client_event = api.WSACreateEvent();
  REQUIRE(client_event != WSA_INVALID_EVENT);
  REQUIRE(api.WSAEventSelect(client_socket, client_event, FD_CONNECT | FD_CLOSE) == 0);

  auto connect_result = api.connect(client_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr)));
  if (connect_result == SOCKET_ERROR)
  {
    auto connect_error = api.WSAGetLastError();
    // Native Winsock reports WOULDBLOCK for nonblocking connect-in-progress.
    // EINPROGRESS must not leak to the game (Celtic Kings rejects it).
    REQUIRE(connect_error == WSAEWOULDBLOCK);
    REQUIRE(connect_error != WSAEINPROGRESS);
  }
  else
  {
    REQUIRE(connect_result == 0);
  }

  REQUIRE(api.WSAWaitForMultipleEvents(1, &client_event, FALSE, 2000, FALSE) == WSA_WAIT_EVENT_0);

  WSANETWORKEVENTS network_events{};
  REQUIRE(api.WSAEnumNetworkEvents(client_socket, client_event, &network_events) == 0);
  REQUIRE(network_events.lNetworkEvents & FD_CONNECT);
  REQUIRE(network_events.iErrorCode[FD_CONNECT_BIT] == 0);

  auto accepted = api.accept(server_socket, nullptr, nullptr);
  REQUIRE(accepted != INVALID_SOCKET);

  REQUIRE(api.WSAEventSelect(client_socket, client_event, 0) == 0);
  REQUIRE(api.WSACloseEvent(client_event) == TRUE);
  REQUIRE(api.closesocket(accepted) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_tcp_event_select_connect_refused(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.connect);
  REQUIRE(api.getpeername);
  REQUIRE(api.WSACreateEvent);
  REQUIRE(api.WSACloseEvent);
  REQUIRE(api.WSAWaitForMultipleEvents);
  REQUIRE(api.WSAEventSelect);
  REQUIRE(api.WSAEnumNetworkEvents);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto client_socket = api.socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  REQUIRE(client_socket != INVALID_SOCKET);

  auto client_event = api.WSACreateEvent();
  REQUIRE(client_event != WSA_INVALID_EVENT);
  REQUIRE(api.WSAEventSelect(client_socket, client_event, FD_CONNECT | FD_CLOSE) == 0);

  sockaddr_in remote_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  remote_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  auto connect_result = api.connect(client_socket, reinterpret_cast<const sockaddr*>(&remote_addr), static_cast<int>(sizeof(remote_addr)));
  REQUIRE(connect_result == SOCKET_ERROR);
  REQUIRE(api.WSAGetLastError() == WSAEWOULDBLOCK);

  REQUIRE(api.WSAWaitForMultipleEvents(1, &client_event, FALSE, 10000, FALSE) == WSA_WAIT_EVENT_0);

  WSANETWORKEVENTS network_events{};
  REQUIRE(api.WSAEnumNetworkEvents(client_socket, client_event, &network_events) == 0);
  REQUIRE(network_events.lNetworkEvents & FD_CONNECT);
  REQUIRE(network_events.iErrorCode[FD_CONNECT_BIT] != 0);

  sockaddr_in peer{};
  int peer_len = static_cast<int>(sizeof(peer));
  REQUIRE(api.getpeername(client_socket, reinterpret_cast<sockaddr*>(&peer), &peer_len) == SOCKET_ERROR);

  REQUIRE(api.WSAEventSelect(client_socket, client_event, 0) == 0);
  REQUIRE(api.WSACloseEvent(client_event) == TRUE);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_event_select_close_stops_watch(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.WSACreateEvent);
  REQUIRE(api.WSACloseEvent);
  REQUIRE(api.WSAWaitForMultipleEvents);
  REQUIRE(api.WSAEventSelect);
  REQUIRE(api.WSAEnumNetworkEvents);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto dead_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(dead_socket != INVALID_SOCKET);

  auto dead_event = api.WSACreateEvent();
  REQUIRE(dead_event != WSA_INVALID_EVENT);
  REQUIRE(api.WSAEventSelect(dead_socket, dead_event, FD_READ) == 0);

  REQUIRE(api.closesocket(dead_socket) != SOCKET_ERROR);

  WSANETWORKEVENTS network_events{};
  REQUIRE(api.WSAEnumNetworkEvents(dead_socket, dead_event, &network_events) == SOCKET_ERROR);
  REQUIRE(api.WSAGetLastError() == WSAENOTSOCK);

  REQUIRE(api.WSAEventSelect(dead_socket, dead_event, FD_READ) == SOCKET_ERROR);
  REQUIRE(api.WSAGetLastError() == WSAENOTSOCK);

  REQUIRE(api.WSACloseEvent(dead_event) == TRUE);

  auto client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(client_socket != INVALID_SOCKET);
  auto server_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(server_socket != INVALID_SOCKET);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

  auto server_event = api.WSACreateEvent();
  REQUIRE(server_event != WSA_INVALID_EVENT);
  REQUIRE(api.WSAEventSelect(server_socket, server_event, FD_READ) == 0);

  std::string message = "ping";
  auto sent_size = api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr)));
  REQUIRE(sent_size == static_cast<int>(message.size()));

  REQUIRE(api.WSAWaitForMultipleEvents(1, &server_event, FALSE, 1000, FALSE) == WSA_WAIT_EVENT_0);

  network_events = {};
  REQUIRE(api.WSAEnumNetworkEvents(server_socket, server_event, &network_events) == 0);
  REQUIRE(network_events.lNetworkEvents & FD_READ);

  sockaddr_in addr{};
  int addr_len = static_cast<int>(sizeof(addr));
  auto received = api.recvfrom(server_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  REQUIRE(received == 4);
  REQUIRE(message == "ping");

  REQUIRE(api.WSAEventSelect(server_socket, nullptr, 0) == 0);
  REQUIRE(api.WSACloseEvent(server_event) == TRUE);
  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_udp_async_select(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.WSAAsyncSelect);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  constexpr UINT start_message = WM_APP + 39;
  constexpr UINT server_message = WM_APP + 40;
  constexpr UINT client_message = WM_APP + 41;
  constexpr UINT_PTR timeout_timer = 1;

  SOCKET client_socket = INVALID_SOCKET;
  SOCKET server_socket = INVALID_SOCKET;
  std::string message = "ping";

  auto dialog_result = win32::DialogBoxIndirectParamW(
    ::GetModuleHandleW(nullptr),
    win32::default_dialog({ .cx = 50, .cy = 40 }),
    win32::window_ref{},
    [&](win32::window_ref dialog, UINT message_id, WPARAM wparam, LPARAM lparam) -> std::optional<LRESULT> {
      HWND hwnd = dialog;

      switch (message_id)
      {
      case WM_INITDIALOG: {
        ::PostMessageW(hwnd, start_message, 0, 0);
        ::SetTimer(hwnd, timeout_timer, 2000, nullptr);
        return TRUE;
      }
      case start_message: {
        client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        REQUIRE(client_socket != INVALID_SOCKET);

        server_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        REQUIRE(server_socket != INVALID_SOCKET);

        sockaddr_in server_addr{
          .sin_family = AF_INET,
          .sin_port = api.htons(port),
        };
        server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

        REQUIRE(api.WSAAsyncSelect(server_socket, hwnd, server_message, FD_READ) == 0);
        REQUIRE(api.WSAAsyncSelect(client_socket, hwnd, client_message, FD_READ) == 0);

        sockaddr_in localhost{
          .sin_family = AF_INET,
          .sin_port = api.htons(port),
        };
        localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        auto sent_size = api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&localhost), static_cast<int>(sizeof(localhost)));
        REQUIRE(sent_size == static_cast<int>(message.size()));
        return 0;
      }
      case server_message: {
        REQUIRE(static_cast<SOCKET>(wparam) == server_socket);
        REQUIRE(WSAGETSELECTERROR(lparam) == 0);
        REQUIRE(WSAGETSELECTEVENT(lparam) & FD_READ);

        sockaddr_in addr{};
        int addr_len = static_cast<int>(sizeof(addr));
        auto received = api.recvfrom(server_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
        REQUIRE(received == 4);
        REQUIRE(addr.sin_family == AF_INET);
        REQUIRE(message == "ping");

        std::string reply = "pong";
        auto sent_size = api.sendto(server_socket, reply.data(), static_cast<int>(reply.size()), 0, reinterpret_cast<const sockaddr*>(&addr), addr_len);
        REQUIRE(sent_size == static_cast<int>(reply.size()));
        return 0;
      }
      case client_message: {
        REQUIRE(static_cast<SOCKET>(wparam) == client_socket);
        REQUIRE(WSAGETSELECTERROR(lparam) == 0);
        REQUIRE(WSAGETSELECTEVENT(lparam) & FD_READ);

        sockaddr_in addr{};
        int addr_len = static_cast<int>(sizeof(addr));
        auto received = api.recvfrom(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
        REQUIRE(received == 4);
        REQUIRE(addr.sin_family == AF_INET);
        REQUIRE(addr.sin_port == api.htons(port));
        REQUIRE(message == "pong");

        ::KillTimer(hwnd, timeout_timer);
        REQUIRE(api.WSAAsyncSelect(server_socket, hwnd, 0, 0) == 0);
        REQUIRE(api.WSAAsyncSelect(client_socket, hwnd, 0, 0) == 0);
        ::EndDialog(hwnd, 1);
        return 0;
      }
      case WM_TIMER: {
        ::EndDialog(hwnd, -1);
        return 0;
      }
      default:
        return std::nullopt;
      }
    });

  REQUIRE(dialog_result == 1);
  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_udp_overlapped_poll(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.WSASocketW);
  REQUIRE(api.WSARecvFrom);
  REQUIRE(api.WSAGetOverlappedResult);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(client_socket != INVALID_SOCKET);

  auto server_socket = api.WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
  REQUIRE(server_socket != INVALID_SOCKET);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

  std::array<char, 4> buffer{};
  WSABUF wsa_buf{ .len = static_cast<ULONG>(buffer.size()), .buf = buffer.data() };
  DWORD flags = 0;
  DWORD transferred = 0;
  sockaddr_in from{};
  int from_len = static_cast<int>(sizeof(from));
  WSAOVERLAPPED overlapped{};

  auto recv_result = api.WSARecvFrom(server_socket, &wsa_buf, 1, &transferred, &flags, reinterpret_cast<sockaddr*>(&from), &from_len, &overlapped, nullptr);
  REQUIRE(recv_result == SOCKET_ERROR);
  REQUIRE(api.WSAGetLastError() == WSA_IO_PENDING);

  // wait=TRUE is only defined for event-based completion; poll repeatedly instead.
  transferred = 0;
  flags = 0;
  REQUIRE(api.WSAGetOverlappedResult(server_socket, &overlapped, &transferred, FALSE, &flags) == FALSE);
  REQUIRE(api.WSAGetLastError() == WSA_IO_INCOMPLETE);

  std::string message = "ping";
  sockaddr_in localhost{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&localhost), static_cast<int>(sizeof(localhost))) == 4);

  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  BOOL completed = FALSE;
  do
  {
    transferred = 0;
    flags = 0;
    completed = api.WSAGetOverlappedResult(server_socket, &overlapped, &transferred, FALSE, &flags);
    if (completed)
    {
      break;
    }
    REQUIRE(api.WSAGetLastError() == WSA_IO_INCOMPLETE);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  } while (std::chrono::steady_clock::now() < deadline);

  REQUIRE(completed == TRUE);
  REQUIRE(transferred == 4);
  REQUIRE(std::string_view{ buffer.data(), buffer.size() } == "ping");

  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_udp_overlapped_event_getresult(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.WSASocketW);
  REQUIRE(api.WSARecvFrom);
  REQUIRE(api.WSAGetOverlappedResult);
  REQUIRE(api.WSACreateEvent);
  REQUIRE(api.WSACloseEvent);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(client_socket != INVALID_SOCKET);

  auto server_socket = api.WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
  REQUIRE(server_socket != INVALID_SOCKET);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

  std::array<char, 4> buffer{};
  WSABUF wsa_buf{ .len = static_cast<ULONG>(buffer.size()), .buf = buffer.data() };
  DWORD flags = 0;
  DWORD transferred = 0;
  sockaddr_in from{};
  int from_len = static_cast<int>(sizeof(from));
  WSAOVERLAPPED overlapped{};
  overlapped.hEvent = api.WSACreateEvent();
  REQUIRE(overlapped.hEvent != WSA_INVALID_EVENT);

  auto recv_result = api.WSARecvFrom(server_socket, &wsa_buf, 1, &transferred, &flags, reinterpret_cast<sockaddr*>(&from), &from_len, &overlapped, nullptr);
  REQUIRE(recv_result == SOCKET_ERROR);
  REQUIRE(api.WSAGetLastError() == WSA_IO_PENDING);

  std::string message = "ping";
  sockaddr_in localhost{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&localhost), static_cast<int>(sizeof(localhost))) == 4);

  transferred = 0;
  flags = 0;
  REQUIRE(api.WSAGetOverlappedResult(server_socket, &overlapped, &transferred, TRUE, &flags) == TRUE);
  REQUIRE(transferred == 4);
  REQUIRE(std::string_view{ buffer.data(), buffer.size() } == "ping");

  REQUIRE(api.WSACloseEvent(overlapped.hEvent) == TRUE);
  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

void test_udp_overlapped_event_wait(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.WSASocketW);
  REQUIRE(api.WSARecvFrom);
  REQUIRE(api.WSAGetOverlappedResult);
  REQUIRE(api.WSACreateEvent);
  REQUIRE(api.WSACloseEvent);
  REQUIRE(api.WSAWaitForMultipleEvents);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(client_socket != INVALID_SOCKET);

  auto server_socket = api.WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
  REQUIRE(server_socket != INVALID_SOCKET);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

  std::array<char, 4> buffer{};
  WSABUF wsa_buf{ .len = static_cast<ULONG>(buffer.size()), .buf = buffer.data() };
  DWORD flags = 0;
  DWORD transferred = 0;
  sockaddr_in from{};
  int from_len = static_cast<int>(sizeof(from));
  WSAOVERLAPPED overlapped{};
  overlapped.hEvent = api.WSACreateEvent();
  REQUIRE(overlapped.hEvent != WSA_INVALID_EVENT);

  auto recv_result = api.WSARecvFrom(server_socket, &wsa_buf, 1, &transferred, &flags, reinterpret_cast<sockaddr*>(&from), &from_len, &overlapped, nullptr);
  REQUIRE(recv_result == SOCKET_ERROR);
  REQUIRE(api.WSAGetLastError() == WSA_IO_PENDING);

  std::string message = "ping";
  sockaddr_in localhost{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&localhost), static_cast<int>(sizeof(localhost))) == 4);

  REQUIRE(api.WSAWaitForMultipleEvents(1, &overlapped.hEvent, FALSE, 2000, FALSE) == WSA_WAIT_EVENT_0);

  transferred = 0;
  flags = 0;
  REQUIRE(api.WSAGetOverlappedResult(server_socket, &overlapped, &transferred, FALSE, &flags) == TRUE);
  REQUIRE(transferred == 4);
  REQUIRE(std::string_view{ buffer.data(), buffer.size() } == "ping");

  REQUIRE(api.WSACloseEvent(overlapped.hEvent) == TRUE);
  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

struct overlapped_apc_op
{
  WSAOVERLAPPED overlapped{};
  std::atomic_bool done{ false };
  DWORD error = 0;
  DWORD transferred = 0;
};

void CALLBACK overlapped_apc_completion(DWORD error, DWORD transferred, LPWSAOVERLAPPED overlapped, DWORD)
{
  auto* op = reinterpret_cast<overlapped_apc_op*>(overlapped);
  op->error = error;
  op->transferred = transferred;
  op->done = true;
}

void test_udp_overlapped_apc(wsock_api& api, WORD version, u_short port)
{
  REQUIRE(api.WSASocketW);
  REQUIRE(api.WSARecvFrom);

  WSAData data{};
  REQUIRE(api.WSAStartup(version, &data) == 0);

  auto client_socket = api.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  REQUIRE(client_socket != INVALID_SOCKET);

  auto server_socket = api.WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
  REQUIRE(server_socket != INVALID_SOCKET);

  sockaddr_in server_addr{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.bind(server_socket, reinterpret_cast<const sockaddr*>(&server_addr), static_cast<int>(sizeof(server_addr))) != SOCKET_ERROR);

  std::array<char, 4> buffer{};
  WSABUF wsa_buf{ .len = static_cast<ULONG>(buffer.size()), .buf = buffer.data() };
  DWORD flags = 0;
  DWORD transferred = 0;
  sockaddr_in from{};
  int from_len = static_cast<int>(sizeof(from));
  overlapped_apc_op op{};

  auto recv_result = api.WSARecvFrom(server_socket, &wsa_buf, 1, &transferred, &flags, reinterpret_cast<sockaddr*>(&from), &from_len, &op.overlapped, overlapped_apc_completion);
  REQUIRE(recv_result == SOCKET_ERROR);
  REQUIRE(api.WSAGetLastError() == WSA_IO_PENDING);

  std::string message = "ping";
  sockaddr_in localhost{
    .sin_family = AF_INET,
    .sin_port = api.htons(port),
  };
  localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  REQUIRE(api.sendto(client_socket, message.data(), static_cast<int>(message.size()), 0, reinterpret_cast<const sockaddr*>(&localhost), static_cast<int>(sizeof(localhost))) == 4);

  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!op.done && std::chrono::steady_clock::now() < deadline)
  {
    ::SleepEx(50, TRUE);
  }

  REQUIRE(op.done);
  REQUIRE(op.error == 0);
  REQUIRE(op.transferred == 4);
  REQUIRE(std::string_view{ buffer.data(), buffer.size() } == "ping");

  REQUIRE(api.closesocket(server_socket) != SOCKET_ERROR);
  REQUIRE(api.closesocket(client_socket) != SOCKET_ERROR);
  REQUIRE(api.WSACleanup() == 0);
}

wsock_api load_client(const char* dll_name)
{
  // Pin client DLLs for the process lifetime. in-proc owns a static select
  // jthread; FreeLibrary during the suite aborts when that thread is torn down.
  static std::list<win32::module> pinned;
  static std::unordered_map<std::string, win32::module*> by_name;

  win32::module* module = nullptr;
  if (auto item = by_name.find(dll_name); item != by_name.end())
  {
    module = item->second;
  }
  else
  {
    module = &pinned.emplace_back(dll_name);
    by_name.emplace(dll_name, module);
  }

  wsock_api api{ .module = module->ref() };

  api.WSAStartup = api.module.GetProcAddress<decltype(api.WSAStartup)>("WSAStartup");
  api.WSACleanup = api.module.GetProcAddress<decltype(api.WSACleanup)>("WSACleanup");
  api.socket = api.module.GetProcAddress<decltype(api.socket)>("socket");
  api.closesocket = api.module.GetProcAddress<decltype(api.closesocket)>("closesocket");
  api.setsockopt = api.module.GetProcAddress<decltype(api.setsockopt)>("setsockopt");
  api.getsockopt = api.module.GetProcAddress<decltype(api.getsockopt)>("getsockopt");
  api.getsockname = api.module.GetProcAddress<decltype(api.getsockname)>("getsockname");
  api.getpeername = api.module.GetProcAddress<decltype(api.getpeername)>("getpeername");
  api.recvfrom = api.module.GetProcAddress<decltype(api.recvfrom)>("recvfrom");
  api.sendto = api.module.GetProcAddress<decltype(api.sendto)>("sendto");
  api.bind = api.module.GetProcAddress<decltype(api.bind)>("bind");
  api.connect = api.module.GetProcAddress<decltype(api.connect)>("connect");
  api.listen = api.module.GetProcAddress<decltype(api.listen)>("listen");
  api.accept = api.module.GetProcAddress<decltype(api.accept)>("accept");
  api.htons = api.module.GetProcAddress<decltype(api.htons)>("htons");
  api.ioctlsocket = api.module.GetProcAddress<decltype(api.ioctlsocket)>("ioctlsocket");
  api.select = api.module.GetProcAddress<decltype(api.select)>("select");
  api.WSAGetLastError = api.module.GetProcAddress<decltype(api.WSAGetLastError)>("WSAGetLastError");
  api.WSACreateEvent = api.module.GetProcAddress<decltype(api.WSACreateEvent)>("WSACreateEvent");
  api.WSASetEvent = api.module.GetProcAddress<decltype(api.WSASetEvent)>("WSASetEvent");
  api.WSAResetEvent = api.module.GetProcAddress<decltype(api.WSAResetEvent)>("WSAResetEvent");
  api.WSACloseEvent = api.module.GetProcAddress<decltype(api.WSACloseEvent)>("WSACloseEvent");
  api.WSAWaitForMultipleEvents = api.module.GetProcAddress<decltype(api.WSAWaitForMultipleEvents)>("WSAWaitForMultipleEvents");
  api.WSAEventSelect = api.module.GetProcAddress<decltype(api.WSAEventSelect)>("WSAEventSelect");
  api.WSAEnumNetworkEvents = api.module.GetProcAddress<decltype(api.WSAEnumNetworkEvents)>("WSAEnumNetworkEvents");
  api.WSAAsyncSelect = api.module.GetProcAddress<decltype(api.WSAAsyncSelect)>("WSAAsyncSelect");
  api.WSASocketW = api.module.GetProcAddress<decltype(api.WSASocketW)>("WSASocketW");
  api.WSARecvFrom = api.module.GetProcAddress<decltype(api.WSARecvFrom)>("WSARecvFrom");
  api.WSASendTo = api.module.GetProcAddress<decltype(api.WSASendTo)>("WSASendTo");
  api.WSAGetOverlappedResult = api.module.GetProcAddress<decltype(api.WSAGetOverlappedResult)>("WSAGetOverlappedResult");

  REQUIRE(api.WSAStartup);
  REQUIRE(api.WSACleanup);
  REQUIRE(api.socket);
  REQUIRE(api.closesocket);
  REQUIRE(api.setsockopt);
  REQUIRE(api.getsockopt);
  REQUIRE(api.getsockname);
  REQUIRE(api.recvfrom);
  REQUIRE(api.sendto);
  REQUIRE(api.bind);
  REQUIRE(api.htons);
  REQUIRE(api.ioctlsocket);
  REQUIRE(api.select);
  REQUIRE(api.WSAGetLastError);
  REQUIRE(api.WSAAsyncSelect);

  return api;
}

void terminate_rpc_server_if_running()
{
  for (;;)
  {
    HWND server = ::FindWindowExW(HWND_MESSAGE, nullptr, L"wsock32-rpc-server", nullptr);
    if (!server)
    {
      return;
    }

    DWORD process_id = 0;
    ::GetWindowThreadProcessId(server, &process_id);
    if (!process_id)
    {
      return;
    }

    HANDLE process = ::OpenProcess(PROCESS_TERMINATE, FALSE, process_id);
    if (!process)
    {
      return;
    }

    ::TerminateProcess(process, 1);
    ::CloseHandle(process);
    ::Sleep(50);
  }
}
