#ifndef SIEGE_SFX_CONTROLLER_HPP
#define SIEGE_SFX_CONTROLLER_HPP

#include <string>
#include <string_view>
#include <vector>
#include <array>

namespace siege::views
{
  class cfg_controller
  {
  public:
    constexpr static auto formats = std::array<std::wstring_view, 4>{{ L".cfg", L".ini", L".txt", L".cs" }};
    static bool is_cfg(std::istream& image_stream);

    std::size_t load_config(std::istream& image_stream) noexcept;
  private:
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
