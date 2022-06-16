#ifndef DARKSTAR_EXTENDER_MUSIC_PLAYER_HPP
#define DARKSTAR_EXTENDER_MUSIC_PLAYER_HPP

#include <cstdint>
#include <filesystem>
#include <array>
#include <memory>

enum struct music_player_state
{
  stopped,
  paused,
  playing
};

struct music_player
{
  music_player();
  [[maybe_unused]] bool load(const std::filesystem::path& path);
  [[maybe_unused]] bool play();
  [[maybe_unused]] bool pause();
  [[maybe_unused]] bool stop();
  music_player_state state() const noexcept;

  [[maybe_unused]] bool volume(float volume) noexcept;
  float volume() const noexcept;

  std::uint32_t length() const noexcept;
  std::uint32_t tell() const noexcept;
  std::uint32_t seek(std::uint32_t position) const noexcept;

private:
  struct music_player_impl;
  using instance_ptr = std::unique_ptr<music_player_impl, void(*)(music_player_impl*)>;

  std::array<std::byte, sizeof(std::int64_t) * 24> storage;
  instance_ptr instance;
};


#endif// DARKSTAR_EXTENDER_MUSIC_PLAYER_HPP
