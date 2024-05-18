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
#include <fstream>
#include <spanstream>

#if WIN32
    #include <siege/platform/win/core/file.hpp>
#endif

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

#if WIN32
    if (!path)
    {
        if (std::spanstream* span_stream = dynamic_cast<std::spanstream*>(&image_stream); span_stream != nullptr)
        {
            auto span = span_stream->rdbuf()->span();
            auto view = win32::file_view(span.data());

            auto filename = view.GetMappedFilename();
            view.release();

            if (filename)
            {
                path = std::move(filename);
            }
        } 
    }
#endif

    if (resource && path)
    {
        contents = resource->get_content_listing(image_stream, platform::listing_query {
            .archive_path = *path,
            .folder_path = *path            
            });

        storage = std::move(*path);

        return contents.size();
    }
    else if (resource)
    {
        // TODO copy stream to memory
    
    }
    return 0;
  }

  std::vector<char> vol_controller::load_content_data(const siege::platform::resource_reader::content_info& content)
  {
    std::vector<char> results;

    if (!resource)
    {
        return results;
    }

    if (storage.index() == 0)
    {
        return results;
    }

    if (auto* file = std::get_if<siege::platform::file_info>(&content))
	{
        results.assign(file->size, char{});
        std::ospanstream output(results);

        if (auto* path = std::get_if<std::filesystem::path>(&storage); path)
        {
            std::ifstream fstream{*path, std::ios_base::binary};
            resource->extract_file_contents(fstream, *file, output);
        }
        else if (auto* memory = std::get_if<std::stringstream>(&storage); memory)
        {
            resource->extract_file_contents(*memory, *file, output);
        }
    }

    return results;
  }

  std::span<siege::platform::resource_reader::content_info> vol_controller::get_contents()
  {
    return contents;
  }
}