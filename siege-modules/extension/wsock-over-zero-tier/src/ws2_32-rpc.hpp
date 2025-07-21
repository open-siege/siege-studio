#ifndef WS2_RPC_HPP
#define WS2_RPC_HPP

#include <WinSock2.h>
#include <siege/platform/win/file.hpp>
#include <array>

struct general_params
{
  constexpr static auto get_last_error_message_id = WM_APP + 1;
};

struct socket_params
{
  constexpr static auto message_id = general_params::get_last_error_message_id + 1;

  int address_family;
  int type;
  int protocol;
};
static_assert(std::is_trivially_copyable_v<socket_params>);

struct ioctl_params
{
  constexpr static auto message_id = socket_params::message_id + 1;

  long command;
  u_long argument;
};

static_assert(std::is_trivially_copyable_v<ioctl_params>);

struct sockopt_params
{
  constexpr static auto get_message_id = ioctl_params::message_id + 1;
  constexpr static auto set_message_id = get_message_id + 1;

  int level;
  int optname;
  int option_length;
  std::array<char, 32> option_data;
};

static_assert(std::is_trivially_copyable_v<sockopt_params>);

struct bind_params
{
  constexpr static auto message_id = sockopt_params::set_message_id + 1;

  int address_size;
  sockaddr_storage address;
};

static_assert(std::is_trivially_copyable_v<bind_params>);


struct sendto_params
{
  constexpr static auto message_id = bind_params::message_id + 1;

  int buffer_length;
  HGLOBAL buffer;
  int flags;
  int to_address_size;
  sockaddr_storage to_address;
};

static_assert(std::is_trivially_copyable_v<sendto_params>);


struct recvfrom_params
{
  constexpr static auto message_id = sendto_params::message_id + 1;

  int buffer_length;
  HGLOBAL buffer;
  int flags;
  int from_address_size;
  sockaddr_storage from_address;
};

struct select_params
{
  constexpr static auto message_id = recvfrom_params::message_id + 1;

  int fd_set_count;
  fd_set read_set;
  fd_set write_set;
  fd_set except_set;
  timeval timeout;
};

static_assert(std::is_trivially_copyable_v<select_params>);

#endif