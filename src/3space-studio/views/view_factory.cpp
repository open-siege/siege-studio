#include "view_factory.hpp"
#include "darkstar_dts_view.hpp"
#include "bmp_view.hpp"
#include "pal_view.hpp"
#include "vol_view.hpp"
#include "dts_io.hpp"
#include "content/bitmap.hpp"
#include "archives/darkstar_volume.hpp"
#include "archives/three_space_volume.hpp"
#include "archives/trophy_bass_volume.hpp"

namespace dts = darkstar::dts;

view_factory create_default_view_factory()
{
  view_factory view_factory;
  view_factory.add_file_type(dts::is_darkstar_dts, [](auto&, auto& stream, auto&) { return static_cast<graphics_view*>(new darkstar_dts_view(stream)); });
  view_factory.add_file_type(darkstar::bmp::is_microsoft_bmp, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_phoenix_bmp, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_phoenix_bmp_array, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::pal::is_microsoft_pal, [](auto&, auto& stream, auto&) { return static_cast<graphics_view*>(new pal_view(stream)); });
  view_factory.add_file_type(darkstar::pal::is_phoenix_pal, [](auto&, auto& stream, auto&) { return static_cast<graphics_view*>(new pal_view(stream)); });

  view_factory.add_file_type(darkstar::vol::vol_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  view_factory.add_file_type(three_space::vol::vol_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  view_factory.add_file_type(three_space::vol::rmf_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  view_factory.add_file_type(three_space::vol::dyn_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  view_factory.add_file_type(trophy_bass::vol::rbx_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  view_factory.add_file_type(trophy_bass::vol::tbv_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });


  view_factory.add_extension(".dts", dts::is_darkstar_dts);

  view_factory.add_extension(".bmp", darkstar::bmp::is_microsoft_bmp);
  view_factory.add_extension(".bmp", darkstar::bmp::is_phoenix_bmp);
  view_factory.add_extension(".pba", darkstar::bmp::is_phoenix_bmp_array);

  view_factory.add_extension(".pal", darkstar::pal::is_microsoft_pal);
  view_factory.add_extension(".pal", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".ipl", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".ppl", darkstar::pal::is_phoenix_pal);

  view_factory.add_extension(".vol", darkstar::vol::vol_file_archive::is_supported);
  view_factory.add_extension(".vol", three_space::vol::vol_file_archive::is_supported);
  view_factory.add_extension(".rmf", three_space::vol::rmf_file_archive::is_supported);
  view_factory.add_extension(".map", three_space::vol::rmf_file_archive::is_supported);
  view_factory.add_extension(".vga", three_space::vol::rmf_file_archive::is_supported);
  view_factory.add_extension(".dyn", three_space::vol::dyn_file_archive::is_supported);
  view_factory.add_extension(".rbx", trophy_bass::vol::rbx_file_archive::is_supported);
  view_factory.add_extension(".tbv", trophy_bass::vol::tbv_file_archive::is_supported);

  return view_factory;
}