#ifndef SIEGE_SFX_CONTROLLER_HPP
#define SIEGE_SFX_CONTROLLER_HPP

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <any>

namespace siege::views
{
  class exe_controller
  {
  public:
    constexpr static auto exe_formats = std::array<std::wstring_view, 2>{{ L".exe", L".com" }};
    constexpr static auto lib_formats = std::array<std::wstring_view, 6>{{ L".dll", L".ocx", L".olb", L".lib", L".asi", L".ovl", }};
    
    static bool is_exe(std::istream& image_stream);

    std::size_t load_executable(std::istream& image_stream) noexcept;
  private:
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
