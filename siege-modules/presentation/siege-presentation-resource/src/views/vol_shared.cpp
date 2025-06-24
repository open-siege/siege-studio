#include <siege/resource/resource_maker.hpp>
#include <siege/platform/stream.hpp>
#include <fstream>
#include <spanstream>
#include <shared_mutex>
#include <siege/platform/wave.hpp>
#include <siege/platform/bitmap.hpp>
#include <istream>
#include <vector>
#include <variant>
#include <filesystem>
#include <sstream>
#include <future>
#include "vol_shared.hpp"

#if WIN32
#include <siege/platform/win/file.hpp>
#endif

namespace siege::views
{
  using namespace siege::resource;
  using namespace siege::platform;

  std::span<const siege::fs_string_view> get_volume_formats() noexcept
  {
    constexpr static auto formats = std::array<siege::fs_string_view, 31>{ {
      FSL ".vol",
      FSL ".rmf",
      FSL ".mis",
      FSL ".map",
      FSL ".rbx",
      FSL ".tbv",
      FSL ".zip",
      FSL ".vl2",
      FSL ".pk3",
      FSL ".iso",
      FSL ".mds",
      FSL ".cue",
      FSL ".nrg",
      FSL ".7z",
      FSL ".tgz",
      FSL ".rar",
      FSL ".cab",
      FSL ".z",
      FSL ".cln",
      FSL ".pak",
      FSL ".vpk",
      FSL ".pod",
      FSL ".clm",
      FSL ".cd",
      FSL ".blo",
      FSL ".dat",
      FSL ".prj",
      FSL ".mw4",
      FSL ".rsc",
      FSL ".res",
      FSL ".atd",
    } };

    return formats;
  }

  bool is_vol(std::istream& vol_stream) noexcept
  {
    return is_resource_readable(vol_stream);
  }

  struct controller_state
  {
    std::shared_mutex lock;
    std::atomic_bool should_continue = false;
    std::future<void> pending_load;
    std::any cache;
    std::optional<siege::platform::resource_reader> resource;
    std::list<siege::platform::resource_reader::content_info> contents;
    std::variant<std::monostate, std::filesystem::path, std::stringstream> storage;
  };

  controller_state& ref(std::any& self)
  {
    if (!self.has_value() || self.type() != typeid(std::shared_ptr<controller_state>))
    {
      self = std::make_shared<controller_state>();
    }

    return *std::any_cast<std::shared_ptr<controller_state>>(self).get();
  }

  void stop_loading(std::any& self) { ref(self).should_continue = false; }

  std::optional<std::filesystem::path> get_original_path(std::any& self)
  {
    if (auto* path = std::get_if<std::filesystem::path>(&ref(self).storage); path)
    {
      return *path;
    }

    return std::nullopt;
  };


  std::size_t load_volume(std::any& self, std::istream& vol_stream, std::optional<std::filesystem::path> path, std::function<void(resource_reader::content_info&)> on_new_item)
  {
    auto& state = ref(self);
    state.resource.emplace(make_resource_reader(vol_stream));

    if (!path)
    {
      path = siege::platform::get_stream_path(vol_stream);
    }

    if (state.resource && path)
    {
      auto temp = state.resource->get_content_listing(state.cache, vol_stream, platform::listing_query{ .archive_path = *path, .folder_path = *path });

      state.contents.clear();

      state.should_continue = true;
      state.pending_load = std::async(std::launch::async, [&state, path = path, temp = std::move(temp), on_new_item = std::move(on_new_item)]() mutable {
        auto vol_stream = std::make_unique<std::ifstream>(*path, std::ios::binary);

        std::function<void(decltype(temp)&)> get_full_listing = [&](std::vector<resource_reader::content_info>& items) mutable {
          for (resource_reader::content_info& info : items)
          {
            if (!state.should_continue)
            {
              break;
            }
            if (auto folder_info = std::get_if<siege::platform::folder_info>(&info); folder_info)
            {
              std::vector<resource_reader::content_info> children;
              {
                std::shared_lock guard(state.lock);
                children = state.resource->get_content_listing(state.cache, *vol_stream, platform::listing_query{ .archive_path = *path, .folder_path = folder_info->full_path });
              }
              get_full_listing(children);
            }

            if (auto file_info = std::get_if<siege::platform::file_info>(&info); file_info)
            {
              std::unique_lock guard(state.lock);
              on_new_item(info);
              state.contents.emplace_back(info);
            }
          }
        };
        get_full_listing(temp);
      });

      state.storage = std::move(*path);

      return 1;
    }
    else if (state.resource)
    {
      // TODO copy stream to memory
    }
    return 0;
  }

  static std::mutex stream_mutex;

  std::vector<char> load_content_data(std::any& self, const siege::platform::resource_reader::content_info& content)
  {
    std::vector<char> results;

    auto& state = ref(self);
    if (!state.resource)
    {
      return results;
    }

    if (state.storage.index() == 0)
    {
      return results;
    }

    if (auto* file = std::get_if<siege::platform::file_info>(&content))
    {
      std::shared_lock guard(state.lock);
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

        if (auto* path = std::get_if<std::filesystem::path>(&state.storage); path)
        {
          std::ifstream fstream{ *path, std::ios_base::binary };
          state.resource->extract_file_contents(state.cache, fstream, *file, temp_output);
        }
        else if (auto* memory = std::get_if<std::stringstream>(&state.storage); memory)
        {
          state.resource->extract_file_contents(state.cache, *memory, *file, temp_output);
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

      if (auto* path = std::get_if<std::filesystem::path>(&state.storage); path)
      {
        std::ifstream fstream{ *path, std::ios_base::binary };
        state.resource->extract_file_contents(state.cache, fstream, *file, output);
      }
      else if (auto* memory = std::get_if<std::stringstream>(&state.storage); memory)
      {
        std::lock_guard<std::mutex> guard(stream_mutex);
        state.resource->extract_file_contents(state.cache, *memory, *file, output);
      }
    }

    return results;
  }

  std::vector<std::reference_wrapper<siege::platform::resource_reader::content_info>> get_contents(std::any& self)
  {
    auto& state = ref(self);
    std::shared_lock guard(state.lock);
    std::vector<std::reference_wrapper<siege::platform::resource_reader::content_info>> results;
    results.reserve(state.contents.size());

    auto end = state.contents.end();

    for (auto begin = state.contents.begin(); begin != end; std::advance(begin, 1))
    {
      results.emplace_back(std::ref(*begin));
    }

    return results;
  }
}// namespace siege::views