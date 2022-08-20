#include <variant>
#include <array>
#include <string_view>
#include <memory>
#include <SFML/Audio.hpp>
#include "music_player.hpp"

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
  std::variant<std::monostate, sf::Music> player;
  music_player_state current_state;

  bool load(const std::filesystem::path& path)
  {
    pause();
    auto ext = path.extension();
    auto is_sfml = std::find(sfml_extensions.begin(), sfml_extensions.end(), ext);
    if (is_sfml == sfml_extensions.end())
    {
      
      return false;
    }

    auto& sf_player = player.emplace<sf::Music>();

    return sf_player.openFromFile(path.string());
  }

  bool play()
  {
    return std::visit(
      overloaded{
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
        [](sf::Music& self) {
          self.pause();
          return true;
        },
        [](std::monostate&) {
          return false;
        }},
      player);
  }

  bool stop()
  {
    return std::visit(
      overloaded{
          [](sf::Music& self) {
          self.stop();
          return true;
          },
          [](std::monostate&) {
          return false;
        }},
        player);
  }

  bool volume(float volume) noexcept
  {
    return std::visit(
      overloaded{
        [volume](sf::Music& self) {
          self.setVolume(volume);
          return true;
        },
        [](std::monostate&) {
          return false;
        }
      },
      player);
  }

  float volume() const noexcept
  {
    return std::visit(
      overloaded{
        [](const sf::Music& self) {
          try
          {
            return self.getVolume();
          }
          catch (...)
          {
            return 0.0f;
          }
        },
        [](const std::monostate&) {
          return 0.0f;
        } },
      player);
  }

  std::uint32_t length() const noexcept
  {
    return std::visit(
      overloaded{
        [](const sf::Music& self) {
          try
          {
            return std::uint32_t(self.getDuration().asMilliseconds());
          }
          catch (...)
          {
            return 0u;
          }
        },
        [](const std::monostate&) -> std::uint32_t {
          return 0u;
        } },
      player);
  }

  std::uint32_t tell() const noexcept
  {
    return std::visit(
      overloaded{
        [](const sf::Music& self) {
          try
          {
            return std::uint32_t(self.getPlayingOffset().asMilliseconds());
          }
          catch (...)
          {
            return 0u;
          }
        },
        [](const std::monostate&) -> std::uint32_t {
          return 0u;
        } },
      player);
  }

  std::uint32_t seek(std::uint32_t position) noexcept
  {
    return std::visit(
      overloaded{
        [position](sf::Music& self) {
          try
          {
            self.setPlayingOffset(sf::milliseconds(position));
            return std::uint32_t(self.getPlayingOffset().asMilliseconds());
          }
          catch(...)
          {
            return 0u;
          }
        },
        [](std::monostate&) -> std::uint32_t {
          return 0u;
        } },
      player);
  }

  music_player_state state() const noexcept
  {
    return current_state;
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

bool music_player::stop()
{
  return instance->stop();
}

music_player_state music_player::state() const noexcept
{
  return instance->state();
}

bool music_player::volume(float volume) noexcept
{
  return instance->volume(volume);
}

float music_player::volume() const noexcept
{
  return instance->volume();
}

std::uint32_t music_player::length() const noexcept
{
  return instance->length();
}

std::uint32_t music_player::tell() const noexcept
{
  return instance->tell();
}

std::uint32_t music_player::seek(std::uint32_t position) const noexcept
{
  return instance->seek(position);
}
