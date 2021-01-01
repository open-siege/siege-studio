#include "view_factory.hpp"
#include "darkstar_dts_view.hpp"
#include "bmp_view.hpp"
#include "dts_io.hpp"
#include "archives/bitmap.hpp"

namespace dts = darkstar::dts;

view_factory create_default_view_factory()
{
  view_factory view_factory;
  view_factory.add_file_type(dts::is_darkstar_dts, [](auto& stream) { return static_cast<graphics_view*>(new darkstar_dts_view(stream)); });
  view_factory.add_file_type(darkstar::bmp::is_windows_bmp, [](auto& stream) { return static_cast<graphics_view*>(new bmp_view(stream)); });

  view_factory.add_extension(".dts", dts::is_darkstar_dts);
  view_factory.add_extension(".DTS", dts::is_darkstar_dts);

  view_factory.add_extension(".bmp", darkstar::bmp::is_windows_bmp);
  view_factory.add_extension(".BMP", darkstar::bmp::is_windows_bmp);

  return view_factory;
}