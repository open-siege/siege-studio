#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <WinSock2.h>
#include <siege/platform/win/module.hpp>

TEST_CASE("Integration tests for winsock rpc clients", "[vol.darkstar]")
{
  SECTION("ws2_32 socket tests")
  {
    ::SetEnvironmentVariableW(L"ZERO_TIER_NETWORK_ID", L"000000");
    ::SetEnvironmentVariableW(L"WSOCK_RPC_BACKEND", L"ws2_32.dll");
    auto module = win32::module("ws2_32-rpc-client.dll");
    auto startup = module.GetProcAddress<std::add_pointer_t<decltype(::WSAStartup)>>("WSAStartup");
    auto cleanup = module.GetProcAddress<std::add_pointer_t<decltype(::WSACleanup)>>("WSACleanup");
    auto socket = module.GetProcAddress<std::add_pointer_t<decltype(::socket)>>("socket");
    auto getsockopt = module.GetProcAddress<std::add_pointer_t<decltype(::getsockopt)>>("getsockopt");
    auto setsockopt = module.GetProcAddress<std::add_pointer_t<decltype(::setsockopt)>>("setsockopt");
    auto ioctlsocket = module.GetProcAddress<std::add_pointer_t<decltype(::ioctlsocket)>>("ioctlsocket");
    auto closesocket = module.GetProcAddress<std::add_pointer_t<decltype(::closesocket)>>("closesocket");
    auto getsockname = module.GetProcAddress<std::add_pointer_t<decltype(::getsockname)>>("getsockname");
    auto WSAGetLastError = module.GetProcAddress<std::add_pointer_t<decltype(::WSAGetLastError)>>("WSAGetLastError");

    WSAData info{};
    auto result = startup(MAKEWORD(2, 2), &info);
    REQUIRE(result == 0);

    auto udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    REQUIRE(result != SOCKET_ERROR);

    int value = 256;
    result = setsockopt(udp_socket, SOL_SOCKET, SO_RCVBUF, (char*)&value, sizeof(value));
    REQUIRE(result != SOCKET_ERROR);

    value = 0;
    int size = sizeof(value);
    result = getsockopt(udp_socket, SOL_SOCKET, SO_RCVBUF, (char*)&value, &size);
    REQUIRE(result != SOCKET_ERROR);
    REQUIRE(size == sizeof(value));
    REQUIRE(value == 256);


    value = 0;
    size = sizeof(value);
    result = getsockopt(udp_socket, SOL_SOCKET, SO_TYPE, (char*)&value, &size);
    REQUIRE(result != SOCKET_ERROR);
    REQUIRE(size == sizeof(value));
    REQUIRE(value == SOCK_DGRAM);

    sockaddr_in temp{};
    size = sizeof(temp);
    result = getsockname(udp_socket, (sockaddr*)&temp, &size);
    REQUIRE(result == SOCKET_ERROR);

    if (!(WSAGetLastError() == WSAEINVAL || WSAGetLastError() == WSAENOTCONN))
    {
      FAIL();
    }

    result = cleanup();
    REQUIRE(result == 0);
  }
}
