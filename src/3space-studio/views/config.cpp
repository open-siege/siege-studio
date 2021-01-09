#include "config.hpp"
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
  view_factory.add_file_type(dts::is_darkstar_dts, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new darkstar_dts_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_microsoft_bmp, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_phoenix_bmp, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_phoenix_bmp_array, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::pal::is_microsoft_pal, [](auto&, auto& stream, auto&) { return static_cast<graphics_view*>(new pal_view(stream)); });
  view_factory.add_file_type(darkstar::pal::is_phoenix_pal, [](auto&, auto& stream, auto&) { return static_cast<graphics_view*>(new pal_view(stream)); });

  view_factory.add_file_type(darkstar::vol::vol_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });

  // TODO fix issues with extracting some of the older vol formats
  //  view_factory.add_file_type(three_space::vol::vol_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  //  view_factory.add_file_type(three_space::vol::rmf_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  //  view_factory.add_file_type(three_space::vol::dyn_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  //  view_factory.add_file_type(trophy_bass::vol::rbx_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });
  //  view_factory.add_file_type(trophy_bass::vol::tbv_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });


  view_factory.add_extension(".dts", dts::is_darkstar_dts);

  view_factory.add_extension(".bmp", darkstar::bmp::is_microsoft_bmp);
  view_factory.add_extension(".bmp", darkstar::bmp::is_phoenix_bmp);
  view_factory.add_extension(".pba", darkstar::bmp::is_phoenix_bmp_array);

  view_factory.add_extension(".pal", darkstar::pal::is_microsoft_pal);
  view_factory.add_extension(".pal", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".ipl", darkstar::pal::is_phoenix_pal);
  view_factory.add_extension(".ppl", darkstar::pal::is_phoenix_pal);

  view_factory.add_extension(".vol", darkstar::vol::vol_file_archive::is_supported);

  // TODO fix issues with extracting some of the older vol formats
  //  view_factory.add_extension(".vol", three_space::vol::vol_file_archive::is_supported);
  //  view_factory.add_extension(".rmf", three_space::vol::rmf_file_archive::is_supported);
  //  view_factory.add_extension(".map", three_space::vol::rmf_file_archive::is_supported);
  //  view_factory.add_extension(".vga", three_space::vol::rmf_file_archive::is_supported);
  //  view_factory.add_extension(".dyn", three_space::vol::dyn_file_archive::is_supported);
  //  view_factory.add_extension(".rbx", trophy_bass::vol::rbx_file_archive::is_supported);
  //  view_factory.add_extension(".tbv", trophy_bass::vol::tbv_file_archive::is_supported);

  return view_factory;
}

studio::fs::resource_explorer create_default_resource_explorer(const std::filesystem::path& search_path)
{
  studio::fs::resource_explorer archive(search_path);

  // TODO fix issues with extracting some of the older vol formats
  archive.add_archive_type(".tbv", std::make_unique<trophy_bass::vol::tbv_file_archive>());
  archive.add_archive_type(".rbx", std::make_unique<trophy_bass::vol::rbx_file_archive>());
  archive.add_archive_type(".rmf", std::make_unique<three_space::vol::rmf_file_archive>());
  archive.add_archive_type(".map", std::make_unique<three_space::vol::rmf_file_archive>());
  archive.add_archive_type(".vga", std::make_unique<three_space::vol::rmf_file_archive>());
  archive.add_archive_type(".dyn", std::make_unique<three_space::vol::dyn_file_archive>());
  archive.add_archive_type(".vol", std::make_unique<three_space::vol::vol_file_archive>());
  archive.add_archive_type(".vol", std::make_unique<darkstar::vol::vol_file_archive>());

  return archive;
}