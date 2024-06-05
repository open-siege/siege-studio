#ifndef SIEGE_PAL_CONTROLLER_HPP
#define SIEGE_PAL_CONTROLLER_HPP

#include <string>
#include <vector>
#include <any>

namespace siege::views
{
  class dts_controller
  {
  public:
    constexpr static auto formats = std::array<std::wstring_view, 4>{{ L".dts" }};
    static bool is_dts(std::istream& image_stream);

    std::size_t load_shape(std::istream& image_stream);
  private:
      std::vector<std::any> shapes;
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
