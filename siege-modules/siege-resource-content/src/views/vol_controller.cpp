#include "vol_controller.hpp"
#include <siege/resource/darkstar_resource.hpp>
#include <siege/resource/three_space_resource.hpp>
#include <siege/resource/zip_resource.hpp>
#include <siege/resource/trophy_bass_resource.hpp>
#include <siege/resource/sword_resource.hpp>
#include <siege/resource/cyclone_resource.hpp>
#include <siege/resource/iso_resource.hpp>
#include <siege/resource/seven_zip_resource.hpp>
#include <siege/resource/cab_resource.hpp>

namespace siege::views
{
    using namespace siege::resource;

  bool vol_controller::is_vol(std::istream& image_stream) noexcept
  {
      return vol::darkstar::vol_resource_reader::is_supported(image_stream) ||
          vol::three_space::vol_resource_reader::is_supported(image_stream) ||
          vol::three_space::dyn_resource_reader::is_supported(image_stream) ||
          vol::three_space::rmf_resource_reader::is_supported(image_stream) ||
          vol::three_space::vol_resource_reader::is_supported(image_stream) ||
          vol::trophy_bass::rbx_resource_reader::is_supported(image_stream) ||
          vol::trophy_bass::tbv_resource_reader::is_supported(image_stream) ||
          cln::cln_resource_reader::is_supported(image_stream) ||
          atd::atd_resource_reader::is_supported(image_stream) ||
          zip::zip_resource_reader::is_supported(image_stream) ||
          cab::cab_resource_reader::is_supported(image_stream) ||
          iso::iso_resource_reader::is_supported(image_stream);
  }

  std::size_t vol_controller::load_volume(std::istream& image_stream, std::optional<std::filesystem::path> path)
  {
    if (vol::darkstar::vol_resource_reader::is_supported(image_stream))
    {
        resource.reset(new vol::darkstar::vol_resource_reader());
    }
    else if (vol::three_space::vol_resource_reader::is_supported(image_stream))
    {
        resource.reset(new vol::three_space::vol_resource_reader());
    }
    else if (vol::three_space::dyn_resource_reader::is_supported(image_stream))
    {
        resource.reset(new vol::three_space::dyn_resource_reader());
    }
    else if (vol::three_space::rmf_resource_reader::is_supported(image_stream))
    {
        resource.reset(new vol::three_space::rmf_resource_reader());
    }
    else if (vol::trophy_bass::rbx_resource_reader::is_supported(image_stream))
    {
        resource.reset(new vol::trophy_bass::rbx_resource_reader());
    }
    else if (cln::cln_resource_reader::is_supported(image_stream))
    {
        resource.reset(new cln::cln_resource_reader());
    }
    else if (atd::atd_resource_reader::is_supported(image_stream))
    {
        resource.reset(new atd::atd_resource_reader());
    }
    else if (zip::zip_resource_reader::is_supported(image_stream))
    {
        resource.reset(new zip::zip_resource_reader());
    }
    else if (cab::cab_resource_reader::is_supported(image_stream))
    {
        resource.reset(new cab::cab_resource_reader());
    }
    else if (iso::iso_resource_reader::is_supported(image_stream))
    {
        resource.reset(new iso::iso_resource_reader());
    }

    if (resource && path)
    {
        contents = resource->get_content_listing(image_stream, platform::listing_query {
            .archive_path = *path,
            .folder_path = *path            
            });

        storage = std::move(*path);

        return 1;
    }
    else if (resource)
    {
        // TODO get path from stream or copy stream to memory
    
    }
    return 0;
  }
}