#ifndef DARKSTAR_EXTENDER_MUSIC_HPP
#define DARKSTAR_EXTENDER_MUSIC_HPP

#include <cstdint>
#include <filesystem>
#include <array>
#include <memory>

struct music_player
{
  music_player();
  bool load(const std::filesystem::path& path);
  bool play();
  bool pause();
  bool set_volume(float volume);
  float get_volume();
  std::uint32_t length();
  std::uint32_t tell();
  std::uint32_t seek(std::uint32_t position);

private:
  struct music_player_impl;
  using instance_ptr = std::unique_ptr<music_player_impl, void(*)(music_player_impl*)>;

  std::array<std::byte, sizeof(std::int64_t) * 24> storage;
  instance_ptr instance;
};


#endif// DARKSTAR_EXTENDER_MUSIC_HPP
