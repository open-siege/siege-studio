#ifndef DARKSTAR_EXTENDER_WX_MUSIC_FILE_HPP
#define DARKSTAR_EXTENDER_WX_MUSIC_FILE_HPP

#include <cstdint>
#include <array>
#include <filesystem>
#include <fstream>
#include <thread>
#include <string_view>
#include <string>
#include <future>
#include <cstdlib>
#include <mio/mmap.hpp>
#include "wx_remote_state.hpp"

namespace fs = std::filesystem;

PROCESS_INFORMATION* create_process()
{
  STARTUPINFO info{};
  auto process = new PROCESS_INFORMATION{};
  CreateProcessA("tray-player.exe",
    nullptr,
    nullptr,
    nullptr,
    FALSE,
    0,
    nullptr,
    nullptr,
    &info,
    process);

  return process;
}


struct wx_remote_music_player
{
  static_assert(sizeof(char) == sizeof(std::int8_t), "Char size is not 8 bits");

  constexpr static auto mem_path = std::string_view("player.ipc");

  static inline mio::mmap_sink channel;

  std::string path;

  inline wx_remote_music_player()
  {
    std::ofstream file(mem_path, std::ios_base::binary | std::ios_base::trunc);
    {
      std::vector<char> data(sizeof(process_data), char(0));
      file.write(data.data(), data.size());
    }
    file.close();

    std::error_code error;
    channel.map(std::string(mem_path), error);

    static auto process = std::shared_ptr<PROCESS_INFORMATION>(create_process(), [](PROCESS_INFORMATION* data) {
      if (!data)
      {
        return;
      }
      TerminateProcess(data->hProcess, 0);
      delete data;
    });
  }

  inline process_data* shared_data() const
  {
    return reinterpret_cast<process_data*>(channel.data());
  }

  inline player_data& player_info() const
  {
    auto* data = shared_data();

    auto find_next = [data]() {
      auto player = std::find_if(data->players.begin(), data->players.end(), [](const auto& info) {
        std::string_view other = info.path.data();
        return other.empty();
      });

      if (player == data->players.end())
      {
        return data->players.begin();
      }

      return player;
    };

    auto player = std::find_if(data->players.begin(), data->players.end(), [this](const auto& info) {
      std::string_view other = info.path.data();
      return path == other;
    });

    if (player == data->players.end())
    {
      return *find_next();
    }

    return *player;
  }

  inline function_info& func_info(function index) const
  {
    if (path.empty())
    {
      static function_info fallback{};
      return fallback;
    }
    else
    {
      auto& player = player_info();

      return player.functions.at(int(index));
    }
  }

  static void default_yield()
  {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }

  inline static void wait_for_function(function_info& info, int max_retries = 2000, void (*yield)() = default_yield)
  {
    int retry_count = 0;
    while (info.status == function_status::busy && retry_count < max_retries)
    {
      yield();
      ++retry_count;
    }

    if (retry_count == max_retries)
    {
      info.result.success = 0;
    }
  }

  inline bool load(const std::filesystem::path& music_path)
  {
    player_data& player = player_info();

    auto path_str = music_path.string();

    if (path_str.empty())
    {
      return false;
    }

    if (path_str != path)
    {
      path = std::move(path_str);
      function_info& info = player.functions.at(int(function::load));

      info.function = function::load;
      info.arg.length = static_cast<std::uint32_t>(path.size());
      auto length = path.size() >= max_path_size ? max_path_size - 1 : path.size();
      std::copy(path.data(), path.data() + length, player.path.data());

      info.status = function_status::busy;

      wait_for_function(info, 1000, []() { std::this_thread::sleep_for(std::chrono::microseconds(500)); });

      return info.result.success == 1;
    }

    return true;
  }

  inline bool play()
  {
    function_info& info = func_info(function::play);
    info.function = function::play;
    info.status = function_status::busy;

    wait_for_function(info);

    return info.result.success == 1;
  }

  inline bool pause()
  {
    function_info& info = func_info(function::pause);
    info.function = function::pause;
    info.status = function_status::busy;

    wait_for_function(info);

    return info.result.success == 1;
  }

  inline bool stop()
  {
    function_info& info = func_info(function::stop);
    info.function = function::stop;
    info.status = function_status::busy;

    wait_for_function(info);

    return info.result.success == 1;
  }

  inline bool set_volume(float volume)
  {
    function_info& info = func_info(function::set_volume);
    info.function = function::set_volume;
    info.status = function_status::busy;
    info.arg.volume = volume;

    wait_for_function(info);

    return info.result.success == 1;
  }

  inline float get_volume() const noexcept
  {
    function_info& info = func_info(function::get_volume);
    info.function = function::get_volume;
    info.status = function_status::busy;

    wait_for_function(info);

    return info.result.volume;
  }

  inline std::uint32_t length() const noexcept
  {
    function_info& info = func_info(function::length);
    info.function = function::length;
    info.status = function_status::busy;

    wait_for_function(info);

    return info.result.length;
  }

  inline std::uint32_t tell() const noexcept
  {
    function_info& info = func_info(function::tell);
    info.function = function::tell;
    info.status = function_status::busy;

    wait_for_function(info);

    return info.result.position;
  }

  inline std::uint32_t seek(std::uint32_t position)
  {
    function_info& info = func_info(function::seek);
    info.function = function::seek;
    info.arg.position = position;
    info.status = function_status::busy;

    wait_for_function(info);

    return info.result.position;
  }
};


#endif// DARKSTAR_EXTENDER_WX_MUSIC_FILE_HPP
