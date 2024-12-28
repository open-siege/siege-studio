#include "vol_controller.hpp"
#include <siege/resource/resource_maker.hpp>
#include <siege/platform/stream.hpp>
#include <fstream>
#include <spanstream>
#include <mutex>
#include <siege/platform/wave.hpp>
#include <siege/platform/bitmap.hpp>

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

    if (!path)
    {
      path = siege::platform::get_stream_path(vol_stream);
    }

    if (resource && path)
    {
      auto temp = resource->get_content_listing(vol_stream, platform::listing_query{ .archive_path = *path, .folder_path = *path });

      contents.clear();
      contents.reserve(temp.size() * 2);

      std::function<void(const decltype(temp)&)> get_full_listing = [&](const auto& items) {
        for (auto& info : items)
        {
          if (auto folder_info = std::get_if<siege::platform::folder_info>(&info); folder_info)
          {
            auto children = resource->get_content_listing(vol_stream, platform::listing_query{ .archive_path = *path, .folder_path = folder_info->full_path });
            get_full_listing(children);
          }

          if (auto file_info = std::get_if<siege::platform::file_info>(&info); file_info)
          {
            contents.emplace_back(*file_info);
          }
        }
      };

      get_full_listing(temp);

      storage = std::move(*path);

      return contents.size();
    }
    else if (resource)
    {
      // TODO copy stream to memory
    }
    return 0;
  }

  static std::mutex stream_mutex;

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

      if (file->metadata.type() == typeid(siege::platform::wave::format_header))
      {
        auto* header = std::any_cast<siege::platform::wave::format_header>(&file->metadata);
        siege::platform::wave::write_wav_header(output, *header, file->size);
      }

      if (file->metadata.type() == typeid(siege::platform::wave::header_settings))
      {
        auto* header = std::any_cast<siege::platform::wave::header_settings>(&file->metadata);
        siege::platform::wave::write_wav_header(output, *header, (file->size * 8) / header->bits_per_sample);
      }

      if (file->metadata.type() == typeid(siege::platform::bitmap::bitmap_offset_settings))
      {
        auto* settings = std::any_cast<siege::platform::bitmap::bitmap_offset_settings>(&file->metadata);

        std::vector<std::byte> pixels(file->size);
        std::span<char> pixel_span(reinterpret_cast<char*>(pixels.data()), pixels.size());
        std::ospanstream temp_output(pixel_span);

        if (auto* path = std::get_if<std::filesystem::path>(&storage); path)
        {
          std::ifstream fstream{ *path, std::ios_base::binary };
          resource->extract_file_contents(fstream, *file, temp_output);
        }
        else if (auto* memory = std::get_if<std::stringstream>(&storage); memory)
        {
          resource->extract_file_contents(*memory, *file, temp_output);
        }

        pixels.erase(pixels.begin(), pixels.begin() + settings->offset);

        results.resize(pixels.size() + (settings->colours.size() * 4) + 64, char{});
        std::ospanstream output(results);

        siege::platform::bitmap::write_bmp_data(output,
          settings->colours,
          std::move(pixels),
          settings->width,
          settings->height,
          settings->bit_depth,
          settings->auto_flip);

        return results;
      }

      if (auto* path = std::get_if<std::filesystem::path>(&storage); path)
      {
        std::ifstream fstream{ *path, std::ios_base::binary };
        resource->extract_file_contents(fstream, *file, output);
      }
      else if (auto* memory = std::get_if<std::stringstream>(&storage); memory)
      {
        std::lock_guard<std::mutex> guard(stream_mutex);
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