#include "view_factory.hpp"
#include "darkstar_dts_view.hpp"
#include "bmp_view.hpp"
#include "pal_view.hpp"
#include "dts_io.hpp"
#include "archives/bitmap.hpp"

namespace dts = darkstar::dts;

view_factory create_default_view_factory()
{
  view_factory view_factory;
  view_factory.add_file_type(dts::is_darkstar_dts, [](auto& stream, auto&) { return static_cast<graphics_view*>(new darkstar_dts_view(stream)); });
  view_factory.add_file_type(darkstar::bmp::is_microsoft_bmp, [](auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_phoenix_bmp, [](auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(stream, manager)); });
  view_factory.add_file_type(darkstar::pal::is_microsoft_pal, [](auto& stream, auto&) { return static_cast<graphics_view*>(new pal_view(stream)); });
  view_factory.add_file_type(darkstar::pal::is_phoenix_pal, [](auto& stream, auto&) { return static_cast<graphics_view*>(new pal_view(stream)); });

  view_factory.add_extension(".dts", dts::is_darkstar_dts);
  view_factory.add_extension(".DTS", dts::is_darkstar_dts);

  view_factory.add_extension(".bmp", darkstar::bmp::is_microsoft_bmp);
  view_factory.add_extension(".BMP", darkstar::bmp::is_microsoft_bmp);
  view_factory.add_extension(".bmp", darkstar::bmp::is_phoenix_bmp);
  view_factory.add_extension(".BMP", darkstar::bmp::is_phoenix_bmp);

  view_factory.add_extension(".pal", darkstar::pal::is_microsoft_pal);
  view_factory.add_extension(".PAL", darkstar::pal::is_microsoft_pal);
  view_factory.add_extension(".pal", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".PAL", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".ipl", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".IPL", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".ppl", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".PPL", darkstar::pal::is_phoenix_pal);

  return view_factory;
}