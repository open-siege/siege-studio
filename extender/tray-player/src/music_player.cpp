#include <variant>
#include <array>
#include <string_view>
#include <algorithm>
#include <SFML/Audio.hpp>
#include "music_player.hpp"
#include "wx_remote_music_player.hpp"


template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

constexpr auto sfml_extensions = std::array<std::string_view, 3>{
  std::string_view(".wav"),
  std::string_view(".flac"),
  std::string_view(".ogg")
};

struct music_player::music_player_impl
{
  std::variant<std::monostate, wx_remote_music_player, sf::Music> player;

  bool load(const std::filesystem::path& path)
  {
    auto ext = path.extension();
    auto is_sfml = std::find(sfml_extensions.begin(), sfml_extensions.end(), ext);
    if (is_sfml == sfml_extensions.end())
    {
      auto& wx_player = player.emplace<wx_remote_music_player>();
      return wx_player.load(path);
    }

    auto& sf_player = player.emplace<sf::Music>();

    return sf_player.openFromFile(path.string());
  }

  bool play()
  {
    return std::visit(
      overloaded{
        [](wx_remote_music_player& player) {
          return player.play();
        },
        [](sf::Music& player) {
          player.play();
          return true;
        },
        [](std::monostate&) {
          return false;
        }},
      player);
  }

  bool pause()
  {
    return std::visit(
      overloaded{
        [](wx_remote_music_player& player) {
          return player.pause();
        },
        [](sf::Music& player) {
          player.pause();
          return true;
        },
        [](std::monostate&) {
          return false;
        }},
      player);
  }

  bool set_volume(float volume)
  {
    return std::visit(
      overloaded{
        [volume](wx_remote_music_player& player) {
          return player.set_volume(volume);
        },
        [volume](sf::Music& player) {
          player.setVolume(volume);
          return true;
        },
        [](std::monostate&) {
          return false;
        }
      },
      player);
  }

  float get_volume()
  {
    return std::visit(
      overloaded{
        [](wx_remote_music_player& player) {
          return player.get_volume();
        },
        [](sf::Music& player) {
          return player.getVolume();
        },
        [](std::monostate&) {
          return 0.0f;
        } },
      player);
  }

  std::uint32_t length()
  {
    return std::visit(
      overloaded{
        [](wx_remote_music_player& player) {
          return player.length();
        },
        [](sf::Music& player) {
          return std::uint32_t(player.getDuration().asMilliseconds());
        },
        [](std::monostate&) -> std::uint32_t {
          return 0u;
        } },
      player);
  }

  std::uint32_t tell()
  {
    return std::visit(
      overloaded{
        [](wx_remote_music_player& player) {
          return player.tell();
        },
        [](sf::Music& player) {
          return std::uint32_t(player.getPlayingOffset().asMilliseconds());
        },
        [](std::monostate&) -> std::uint32_t {
          return 0u;
        } },
      player);
  }

  std::uint32_t seek(std::uint32_t position)
  {
    return std::visit(
      overloaded{
        [position](wx_remote_music_player& player) {
          return player.seek(position);
        },
        [position](sf::Music& player) {
          player.setPlayingOffset(sf::milliseconds(position));
          return std::uint32_t(player.getPlayingOffset().asMilliseconds());
        },
        [](std::monostate&) -> std::uint32_t {
          return 0u;
        } },
      player);
  }
};

music_player::music_player() : storage(),
                               instance(music_player::instance_ptr(new (storage.data()) music_player_impl(), [](music_player_impl* self) { self->~music_player_impl(); }))
{
  static_assert(sizeof(music_player::music_player_impl) <= sizeof(std::array<std::byte, sizeof(std::int64_t) * 24>), "music_player_impl is too big for the provided storage");
}

bool music_player::load(const std::filesystem::path& path)
{
  return instance->load(path);
}

bool music_player::play()
{
  return instance->play();
}

bool music_player::pause()
{
  return instance->pause();
}

bool music_player::set_volume(float volume)
{
  return instance->set_volume(volume);
}

float music_player::get_volume()
{
  return instance->get_volume();
}

std::uint32_t music_player::length()
{
  return instance->length();
}

std::uint32_t music_player::tell()
{
  return instance->tell();
}

std::uint32_t music_player::seek(std::uint32_t position)
{
  return instance->seek(position);
}
