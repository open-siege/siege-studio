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


#endif// DARKSTAR_EXTENDER_WX_REMOTE_STATE_HPP
