export module sfx.shared;

import std;

import siege.content.sfx;
import siege.platform.shared;
import siege.platform.wave;

namespace siege::views
{
  export std::span<const siege::fs_string_view> get_sfx_formats() noexcept
  {
    constexpr static auto formats = std::array<siege::fs_string_view, 5>{ { FSL".sfx", FSL".wav", FSL".mp3", FSL".ogg", FSL".wma" } };
    return formats;
  }

  export bool is_sfx(std::istream& stream) noexcept
  {
    return siege::platform::wave::is_wav(stream)
           || siege::content::sfx::is_flac(stream)
           || siege::content::sfx::is_ogg(stream);
  }

  content::sfx::platform_sound* self(std::any& state)
  {
    return std::any_cast<content::sfx::platform_sound>(&state);
  }

  export std::size_t load_sound(std::any& state, std::istream& sound_stream) noexcept
  {
    using namespace siege::content;

    try
    {
      state.emplace<content::sfx::platform_sound>(sound_stream);
    }
    catch (...)
    {
    }

    auto* original_sound = self(state);
    if (original_sound)
    {
      return original_sound->track_count();
    }

    return 1;
  }

  export std::optional<std::filesystem::path> get_sound_path(std::any& state, std::size_t index)
  {
    auto* original_sound = self(state);
    if (original_sound)
    {
      auto data = original_sound->get_sound_data(index);

      if (auto* real_data = std::get_if<std::filesystem::path>(&data); real_data)
      {
        return *real_data;
      }
    }

    return std::nullopt;
  }

  export std::optional<std::span<std::byte>> get_sound_data(std::any& state, std::size_t index)
  {
    auto* original_sound = self(state);
    if (original_sound)
    {
      auto data = original_sound->get_sound_data(index);

      if (auto* real_data = std::get_if<std::span<std::byte>>(&data); real_data)
      {
        return *real_data;
      }
    }

    return std::nullopt;
  }
}// namespace siege::views
