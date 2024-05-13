#ifndef SIEGE_SFX_CONTROLLER_HPP
#define SIEGE_SFX_CONTROLLER_HPP

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <any>

namespace siege::views
{
  class sfx_controller
  {
  public:
    constexpr static auto formats = std::array<std::wstring_view, 5>{{ L".sfx", L".wav", L".mp3", L".ogg", L".wma"}};
    static bool is_sfx(std::istream& image_stream);

    std::size_t load_sound(std::istream& image_stream);
  private:
      std::vector<std::any> sounds;
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
