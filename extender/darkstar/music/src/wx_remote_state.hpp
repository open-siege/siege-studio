#ifndef DARKSTAR_EXTENDER_WX_REMOTE_STATE_HPP
#define DARKSTAR_EXTENDER_WX_REMOTE_STATE_HPP

#include <cstdint>

enum struct function_status : std::int32_t
{
  idle,
  busy
};

constexpr auto function_length = 9;

enum struct function : std::int32_t
{
  load,
  play,
  pause,
  stop,
  get_volume,
  set_volume,
  length,
  tell,
  seek
};

union function_arg
{
  std::uint32_t position;
  std::uint32_t length;
  float volume;
};

union function_result
{
  std::uint32_t success;
  std::uint32_t position;
  std::uint32_t length;
  float volume;
};

struct alignas(std::int32_t) function_info
{
  function function;
  function_status status;
  function_arg arg;
  function_result result;
};

constexpr static auto max_path_size = 1024;

struct alignas(std::int32_t) player_data
{
  std::array<function_info, function_length> functions;
  std::array<char, max_path_size> path;
};

constexpr auto max_players = 80;

struct alignas(std::int32_t) process_data
{
  function_status process_status;
  std::array<player_data, max_players> players;
};





#endif// DARKSTAR_EXTENDER_WX_REMOTE_STATE_HPP
