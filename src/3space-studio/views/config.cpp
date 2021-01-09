#include "config.hpp"
#include "darkstar_dts_view.hpp"
#include "bmp_view.hpp"
#include "pal_view.hpp"
#include "vol_view.hpp"
#include "dts_io.hpp"
#include "content/bitmap.hpp"
#include "resource/darkstar_volume.hpp"
#include "resource/three_space_volume.hpp"
#include "resource/trophy_bass_volume.hpp"

namespace dts = darkstar::dts;

namespace dio
{
  namespace vol = studio::resource::vol;
}

view_factory create_default_view_factory()
{
  view_factory view_factory;
  view_factory.add_file_type(dts::is_darkstar_dts, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new darkstar_dts_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_microsoft_bmp, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_phoenix_bmp, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::bmp::is_phoenix_bmp_array, [](auto& info, auto& stream, auto& manager) { return static_cast<graphics_view*>(new bmp_view(info, stream, manager)); });
  view_factory.add_file_type(darkstar::pal::is_microsoft_pal, [](auto&, auto& stream, auto&) { return static_cast<graphics_view*>(new pal_view(stream)); });
  view_factory.add_file_type(darkstar::pal::is_phoenix_pal, [](auto&, auto& stream, auto&) { return static_cast<graphics_view*>(new pal_view(stream)); });

  view_factory.add_file_type(dio::vol::darkstar::vol_file_archive::is_supported, [](auto& info, auto&, auto& archive) { return static_cast<graphics_view*>(new vol_view(info, archive)); });

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

  view_factory.add_extension(".vol", dio::vol::darkstar::vol_file_archive::is_supported);

  // TODO fix issues with extracting some of the older vol formats
  view_factory.add_extension(".vol", dio::vol::three_space::vol_file_archive::is_supported);
  view_factory.add_extension(".rmf", dio::vol::three_space::rmf_file_archive::is_supported);
  view_factory.add_extension(".map", dio::vol::three_space::rmf_file_archive::is_supported);
  view_factory.add_extension(".vga", dio::vol::three_space::rmf_file_archive::is_supported);
  view_factory.add_extension(".dyn", dio::vol::three_space::dyn_file_archive::is_supported);
  view_factory.add_extension(".rbx", dio::vol::trophy_bass::rbx_file_archive::is_supported);
  view_factory.add_extension(".tbv", dio::vol::trophy_bass::tbv_file_archive::is_supported);

  return view_factory;
}

studio::resource::resource_explorer create_default_resource_explorer(const std::filesystem::path& search_path)
{
  studio::resource::resource_explorer archive(search_path);

  // TODO fix issues with extracting some of the older vol formats
  archive.add_archive_type(".tbv", std::make_unique<dio::vol::trophy_bass::tbv_file_archive>());
  archive.add_archive_type(".rbx", std::make_unique<dio::vol::trophy_bass::rbx_file_archive>());
  archive.add_archive_type(".rmf", std::make_unique<dio::vol::three_space::rmf_file_archive>());
  archive.add_archive_type(".map", std::make_unique<dio::vol::three_space::rmf_file_archive>());
  archive.add_archive_type(".vga", std::make_unique<dio::vol::three_space::rmf_file_archive>());
  archive.add_archive_type(".dyn", std::make_unique<dio::vol::three_space::dyn_file_archive>());
  archive.add_archive_type(".vol", std::make_unique<dio::vol::three_space::vol_file_archive>());
  archive.add_archive_type(".vol", std::make_unique<dio::vol::darkstar::vol_file_archive>());

  return archive;
}