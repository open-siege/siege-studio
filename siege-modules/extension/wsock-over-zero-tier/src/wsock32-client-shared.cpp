module;

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <siege/platform/win/module.hpp>

export module wsock32.shared;

import std;

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