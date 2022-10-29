#include "config.hpp"
#include "dts_view.hpp"
#include "bmp_view.hpp"
#include "pal_view.hpp"
#include "pal_mapping_view.hpp"
#include "vol_view.hpp"
#include "sfx_view.hpp"
#include "content/dts/3space.hpp"
#include "content/mis/mission.hpp"
#include "content/dts/darkstar.hpp"
#include "content/bmp/bitmap.hpp"
#include "content/sfx/wave.hpp"
#include "content/image/image.hpp"
#include "content/pal/pal_settings.hpp"
#include "resources/darkstar_volume.hpp"
#include "resources/three_space_volume.hpp"
#include "resources/trophy_bass_volume.hpp"
#include "resources/zip_volume.hpp"
#include "resources/cyclone_volume.hpp"
#include "resources/sword_volume.hpp"

namespace dio
{
  namespace mis = studio::resources::mis;
  namespace vol = studio::resources::vol;
  namespace zip = studio::resources::zip;
  namespace cln = studio::resources::cln;
  namespace atd = studio::resources::atd;
}

namespace studio::views
{
  view_factory create_default_view_factory(const studio::resources::resource_explorer& explorer)
  {
    view_factory view_factory;
    view_factory.add_file_type(content::dts::darkstar::is_darkstar_dts, [](auto context, auto& stream) -> studio_view { return graphics_view(darkstar_dts_view(context, stream)); });
    view_factory.add_file_type(content::dts::three_space::v1::is_3space_dts, [](auto context, auto& stream) -> studio_view { return graphics_view(darkstar_dts_view(context, stream)); });
    view_factory.add_file_type(content::bmp::is_microsoft_bmp, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); });
    view_factory.add_file_type(content::bmp::is_phoenix_bmp, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); });
    view_factory.add_file_type(content::bmp::is_earthsiege_bmp, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); });
    view_factory.add_file_type(content::bmp::is_earthsiege_bmp_array, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); });
    view_factory.add_file_type(content::bmp::is_phoenix_bmp_array, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); });
    view_factory.add_file_type(content::bmp::is_png, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); });
    view_factory.add_file_type(content::bmp::is_gif, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); });
    view_factory.add_file_type(content::bmp::is_jpg, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); }, true);
    view_factory.add_file_type(content::bmp::is_tga, [](auto context, auto& stream) -> studio_view { return graphics_view(bmp_view(context, stream)); }, true);

    view_factory.add_file_type(content::pal::is_pal_settings_file, [](auto context, auto&) -> studio_view { return normal_view(pal_mapping_view(context)); }, true);
    view_factory.add_file_type(content::pal::is_old_pal, [](auto, auto& stream) -> studio_view { return graphics_view(pal_view(stream)); });
    view_factory.add_file_type(content::pal::is_microsoft_pal, [](auto, auto& stream) -> studio_view { return graphics_view(pal_view(stream)); });
    view_factory.add_file_type(content::pal::is_phoenix_pal, [](auto, auto& stream) -> studio_view { return graphics_view(pal_view(stream)); });
    view_factory.add_file_type(content::pal::is_earthsiege_pal, [](auto, auto& stream) -> studio_view { return graphics_view(pal_view(stream)); });

    view_factory.add_file_type(content::sfx::is_sfx_file, [](auto context, auto& stream) -> studio_view { return graphics_view(sfx_view(context.file_info, stream, context.explorer)); }, true);
    view_factory.add_file_type(content::sfx::is_wav_file, [](auto context, auto& stream) -> studio_view { return graphics_view(sfx_view(context.file_info, stream, context.explorer)); });
    view_factory.add_file_type(content::sfx::is_ogg_file, [](auto context, auto& stream) -> studio_view { return graphics_view(sfx_view(context.file_info, stream, context.explorer)); });
    view_factory.add_file_type(content::sfx::is_flac_file, [](auto context, auto& stream) -> studio_view { return graphics_view(sfx_view(context.file_info, stream, context.explorer)); });

    view_factory.add_file_type(dio::mis::darkstar::mis_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); });
    view_factory.add_file_type(dio::zip::zip_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); });
    view_factory.add_file_type(dio::vol::darkstar::vol_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); });

    view_factory.add_file_type(dio::vol::three_space::vol_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); });
    view_factory.add_file_type(dio::vol::three_space::rmf_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); });
    view_factory.add_file_type(dio::vol::three_space::dyn_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); });
    view_factory.add_file_type(dio::vol::trophy_bass::rbx_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); });
    view_factory.add_file_type(dio::vol::trophy_bass::tbv_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); });
    view_factory.add_file_type(dio::cln::cln_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); }, true);
    view_factory.add_file_type(dio::atd::atd_file_archive::is_supported, [](auto context, auto& stream) -> studio_view { return normal_view(vol_view(context)); }, true);

    view_factory.add_extension(".dts", content::dts::darkstar::is_darkstar_dts);
    view_factory.add_extension(".dts", content::dts::three_space::v1::is_3space_dts);

    view_factory.add_extension(".jpg", content::bmp::is_jpg);
    view_factory.add_extension(".jpeg", content::bmp::is_jpg);
    view_factory.add_extension(".gif", content::bmp::is_gif);
    view_factory.add_extension(".png", content::bmp::is_png);
    view_factory.add_extension(".tga", content::bmp::is_tga);
    view_factory.add_extension(".bmp", content::bmp::is_microsoft_bmp);
    view_factory.add_extension(".bmp", content::bmp::is_phoenix_bmp);
    view_factory.add_extension(".dib", content::bmp::is_microsoft_bmp);
    view_factory.add_extension(".dib", content::bmp::is_phoenix_bmp);
    view_factory.add_extension(".pba", content::bmp::is_phoenix_bmp_array);
    view_factory.add_extension(".dbm", content::bmp::is_earthsiege_bmp);
    view_factory.add_extension(".dba", content::bmp::is_earthsiege_bmp_array);
    view_factory.add_extension(".db0", content::bmp::is_earthsiege_bmp_array);
    view_factory.add_extension(".db1", content::bmp::is_earthsiege_bmp_array);
    view_factory.add_extension(".db2", content::bmp::is_earthsiege_bmp_array);
    view_factory.add_extension(".hba", content::bmp::is_earthsiege_bmp_array);
    view_factory.add_extension(".hb0", content::bmp::is_earthsiege_bmp_array);
    view_factory.add_extension(".hb1", content::bmp::is_earthsiege_bmp_array);
    view_factory.add_extension(".hb2", content::bmp::is_earthsiege_bmp_array);

    view_factory.add_extension(".sfx", content::sfx::is_sfx_file);
    view_factory.add_extension(".wav", content::sfx::is_wav_file);
    view_factory.add_extension(".wave", content::sfx::is_wav_file);
    view_factory.add_extension(".ogg", content::sfx::is_ogg_file);
    view_factory.add_extension(".flac", content::sfx::is_flac_file);

    view_factory.add_extension("palettes.settings.json", content::pal::is_pal_settings_file);
    view_factory.add_extension(".pal", content::pal::is_old_pal);
    view_factory.add_extension(".pal", content::pal::is_microsoft_pal);
    view_factory.add_extension(".ipl", content::pal::is_microsoft_pal);
    view_factory.add_extension(".ppl", content::pal::is_microsoft_pal);
    view_factory.add_extension(".pal", content::pal::is_phoenix_pal);
    view_factory.add_extension(".ipl", content::pal::is_phoenix_pal);
    view_factory.add_extension(".ppl", content::pal::is_phoenix_pal);
    view_factory.add_extension(".dpl", content::pal::is_earthsiege_pal);

    view_factory.add_extension(".zip", dio::zip::zip_file_archive::is_supported);
    view_factory.add_extension(".vl2", dio::zip::zip_file_archive::is_supported);
    view_factory.add_extension(".pk3", dio::zip::zip_file_archive::is_supported);

    view_factory.add_extension(".mis", dio::mis::darkstar::mis_file_archive::is_supported);
    view_factory.add_extension(".vol", dio::vol::darkstar::vol_file_archive::is_supported);

    view_factory.add_extension(".vol", dio::vol::three_space::vol_file_archive::is_supported);
    view_factory.add_extension(".rmf", dio::vol::three_space::rmf_file_archive::is_supported);
    view_factory.add_extension(".map", dio::vol::three_space::rmf_file_archive::is_supported);
    view_factory.add_extension(".vga", dio::vol::three_space::rmf_file_archive::is_supported);

    view_factory.add_extension(".dyn", dio::vol::three_space::dyn_file_archive::is_supported);
    view_factory.add_extension(".rbx", dio::vol::trophy_bass::rbx_file_archive::is_supported);
    view_factory.add_extension(".tbv", dio::vol::trophy_bass::tbv_file_archive::is_supported);

    view_factory.add_extension(".cln", dio::cln::cln_file_archive::is_supported);
    view_factory.add_extension(".atd", dio::atd::atd_file_archive::is_supported);

    view_factory.add_extension_category("All Supported Formats", { "ALL" }, true);
    view_factory.add_extension_category("All Palettes", { ".ppl", ".ipl", ".pal", ".dpl" }, true);
    view_factory.add_extension_category("All Images", { ".bmp", ".dib", ".pba", ".dbm", ".dba", ".db0", ".db1", ".db2", ".hba", ".hb0", ".hb1", ".hb2", ".jpg", ".gif", ".png", ".tga" }, true);
    view_factory.add_extension_category("All Archives", explorer.get_archive_extensions(), true);
    view_factory.add_extension_category("All 3D Models", { ".dts" }, true);
    view_factory.add_extension_category("all_palettes", view_factory.get_extensions_by_category("All Palettes"), false);
    view_factory.add_extension_category("all_images", view_factory.get_extensions_by_category("All Images"), false);
    view_factory.add_extension_category("all_archives", view_factory.get_extensions_by_category("All Archives"), false);

    return view_factory;
  }

  studio::resources::resource_explorer create_default_resource_explorer()
  {
    studio::resources::resource_explorer archive;

    archive.add_archive_type(".mis", std::make_unique<dio::mis::darkstar::mis_file_archive>(), dio::mis::darkstar::mis_file_archive::supported_extensions);
    archive.add_archive_type(".tbv", std::make_unique<dio::vol::trophy_bass::tbv_file_archive>());
    archive.add_archive_type(".rbx", std::make_unique<dio::vol::trophy_bass::rbx_file_archive>());
    archive.add_archive_type(".rmf", std::make_unique<dio::vol::three_space::rmf_file_archive>());
    archive.add_archive_type(".map", std::make_unique<dio::vol::three_space::rmf_file_archive>());
    archive.add_archive_type(".vga", std::make_unique<dio::vol::three_space::rmf_file_archive>());
    archive.add_archive_type(".zip", std::make_unique<dio::zip::zip_file_archive>());
    archive.add_archive_type(".vl2", std::make_unique<dio::zip::zip_file_archive>());
    archive.add_archive_type(".pk3", std::make_unique<dio::zip::zip_file_archive>());

    archive.add_archive_type(".dyn", std::make_unique<dio::vol::three_space::dyn_file_archive>());
    archive.add_archive_type(".vol", std::make_unique<dio::vol::three_space::vol_file_archive>());
    archive.add_archive_type(".vol", std::make_unique<dio::vol::darkstar::vol_file_archive>());

    archive.add_archive_type(".cln", std::make_unique<dio::cln::cln_file_archive>());
    archive.add_archive_type(".atd", std::make_unique<dio::atd::atd_file_archive>());

    return archive;
  }
}
