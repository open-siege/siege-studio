#ifndef SIEGE_SFX_CONTROLLER_HPP
#define SIEGE_SFX_CONTROLLER_HPP

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <any>
#include <siege/content/sfx/sound.hpp>

namespace siege::views
{
  class sfx_controller
  {
  public:
    constexpr static auto formats = std::array<siege::fs_string_view, 5>{{ FSL".sfx", FSL".wav", FSL".mp3", FSL".ogg", FSL".wma"}};
    static bool is_sfx(std::istream& image_stream);

    std::size_t load_sound(std::istream& image_stream) noexcept;

    std::optional<std::filesystem::path> get_sound_path(std::size_t index);
    std::optional<std::span<std::byte>> get_sound_data(std::size_t index);
  private:
      std::optional<content::sfx::platform_sound> original_sound;
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
