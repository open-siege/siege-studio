#ifndef DARKSTAR_EXTENDER_WX_MUSIC_FILE_HPP
#define DARKSTAR_EXTENDER_WX_MUSIC_FILE_HPP

#include <cstdint>
#include <array>
#include <filesystem>
#include <fstream>
#include <thread>
#include <string_view>
#include <string>
#include <mio/mmap.hpp>


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

namespace fs = std::filesystem;
struct wx_remote_music_player
{
    static_assert(sizeof(char) == sizeof(std::int8_t), "Char size is not 8 bits");

    constexpr static auto base_mem_path = std::string_view("player.ipc");

    constexpr static auto max_path_size = 1024;

    static inline int channel_count = 0;

    mio::mmap_sink channel;

    inline wx_remote_music_player()
    {
      std::string mem_path = std::string(base_mem_path) + "." + std::to_string(channel_count++);
      std::ofstream file(mem_path, std::ios_base::binary | std::ios_base::trunc);
      {
        std::array<function_info, function_length> functions{};
        file.write(reinterpret_cast<char*>(functions.data()), function_length * sizeof(function_info));
      }

      {
        std::array<char, max_path_size> path{};
        file.write(path.data(),  path.size());
      }
      file.close();

      std::error_code error;
      channel.map(std::string(mem_path), error);
      // TODO need to create process here
    }

    inline std::array<function_info, function_length>* functions()
    {
      return reinterpret_cast<std::array<function_info, function_length>*>(channel.data());
    }

    inline function_info& func_info(function index)
    {
      auto* volatile funcs = functions();

      return funcs->at(int(index));
    }

    inline char* path_data()
    {
      return reinterpret_cast<char*>(channel.data() + sizeof(std::array<function_info, function_length>));
    }

    inline static void wait_for_function(volatile function_info& info)
    {
      int retry_count = 0;
      constexpr auto max_retries = 10;
      while (info.status == function_status::busy && retry_count < max_retries)
      {
        std::this_thread::yield();
        ++retry_count;
      }

      if (retry_count == max_retries)
      {
        info.result.success = 0;
      }
    }

    inline bool load(const std::filesystem::path& path)
    {
      volatile function_info& info = func_info(function::load);
      info.function = function::load;


      auto path_str = path.string();
      info.arg.length = static_cast<std::uint32_t>(path_str.size());
      auto length = path_str.size() >= max_path_size ? max_path_size - 1 : path_str.size();
      std::copy(path_str.data(), path_str.data() + length, path_data());

      info.status = function_status::busy;

      wait_for_function(info);

      return info.result.success == 1;
    }

    inline bool play()
    {
      volatile function_info& info = func_info(function::play);
      info.function = function::play;
      info.status = function_status::busy;

      wait_for_function(info);

      return info.result.success == 1;
    }

    inline bool pause()
    {
      volatile function_info& info = func_info(function::pause);
      info.function = function::pause;
      info.status = function_status::busy;

      wait_for_function(info);

      return info.result.success == 1;
    }

    inline bool set_volume(float volume)
    {
      volatile function_info& info = func_info(function::set_volume);
      info.function = function::set_volume;
      info.status = function_status::busy;
      info.arg.volume = volume;

      wait_for_function(info);

      return info.result.success == 1;
    }

    inline float get_volume()
    {
      volatile function_info& info = func_info(function::get_volume);
      info.function = function::get_volume;
      info.status = function_status::busy;

      wait_for_function(info);

      return info.result.volume;
    }

    inline std::uint32_t length()
    {
      volatile function_info& info = func_info(function::length);
      info.function = function::length;
      info.status = function_status::busy;

      wait_for_function(info);

      return info.result.length;
    }

    inline std::uint32_t tell()
    {
      volatile function_info& info = func_info(function::tell);
      info.function = function::tell;
      info.status = function_status::busy;

      wait_for_function(info);

      return info.result.position;
    }

    inline std::uint32_t seek(std::uint32_t position)
    {
      volatile function_info& info = func_info(function::seek);
      info.function = function::seek;
      info.arg.position = position;
      info.status = function_status::busy;

      wait_for_function(info);

      return info.result.position;
    }
};


#endif// DARKSTAR_EXTENDER_WX_MUSIC_FILE_HPP
