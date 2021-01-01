#include "view_factory.hpp"
#include "darkstar_dts_view.hpp"
#include "dts_io.hpp"

namespace dts = darkstar::dts;

view_factory create_default_view_factory()
{
  view_factory view_factory;
  view_factory.add_file_type(dts::is_darkstar_dts, [](auto& stream) { return static_cast<graphics_view*>(new darkstar_dts_view(stream)); });
  return view_factory;
}