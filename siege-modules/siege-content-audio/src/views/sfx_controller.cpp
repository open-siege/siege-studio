#include <siege/platform/wave.hpp>
#include "sfx_controller.hpp"

namespace siege::views
{
  bool sfx_controller::is_sfx(std::istream& stream)
  {
    return siege::platform::wave::is_wav(stream)
           || siege::content::sfx::is_flac(stream)
           || siege::content::sfx::is_ogg(stream);
  }

  std::size_t sfx_controller::load_sound(std::istream& sound_stream) noexcept
  {
    using namespace siege::content;

    try
    {
      original_sound.emplace(sound_stream);
    }
    catch (...)
    {
    }

    if (original_sound)
    {
      return original_sound->track_count();
    }

    return 1;
  }

  std::optional<std::filesystem::path> sfx_controller::get_sound_path(std::size_t index)
  {
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

  std::optional<std::span<std::byte>> sfx_controller::get_sound_data(std::size_t index)
  {
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