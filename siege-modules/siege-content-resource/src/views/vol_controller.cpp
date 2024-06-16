#include "vol_controller.hpp"
#include <siege/resource/resource_maker.hpp>
#include <fstream>
#include <spanstream>

#if WIN32
#include <siege/platform/win/core/file.hpp>
#endif

namespace siege::views
{
  using namespace siege::resource;

  bool vol_controller::is_vol(std::istream& vol_stream) noexcept
  {
    return is_resource_reader(vol_stream);
  }

  std::size_t vol_controller::load_volume(std::istream& vol_stream, std::optional<std::filesystem::path> path)
  {
    resource.reset(make_resource_reader(vol_stream).release());
#if WIN32
    if (!path)
    {
      if (std::spanstream* span_stream = dynamic_cast<std::spanstream*>(&vol_stream); span_stream != nullptr)
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
      contents = resource->get_content_listing(vol_stream, platform::listing_query{ .archive_path = *path, .folder_path = *path });

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
        std::ifstream fstream{ *path, std::ios_base::binary };
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
}// namespace siege::views