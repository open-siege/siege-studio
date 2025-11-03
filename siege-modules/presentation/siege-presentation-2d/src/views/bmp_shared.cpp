#include <siege/content/bmp/bitmap.hpp>
#include <siege/content/tim.hpp>
#include <siege/platform/image.hpp>
#include <siege/platform/stream.hpp>
#include <deque>
#include <fstream>
#include <spanstream>
#include <filesystem>
#include <future>
#include <execution>
#include "2d_shared.hpp"

namespace siege::views
{
  struct bitmap_data
  {
    std::optional<platform::bitmap::platform_image> original_image;
    std::deque<palette_info> palettes;
    std::deque<palette_info>::iterator selected_palette_file;
    std::size_t selected_palette;
  };

  bitmap_data* get(bmp_context& state)
  {
    if (!state.has_value())
    {
      state.emplace<bitmap_data>();
    }
    return std::any_cast<bitmap_data>(&state);
  }

  const bitmap_data* get(const bmp_context& state)
  {
    return std::any_cast<bitmap_data>(&state);
  }

  std::span<const siege::fs_string_view> get_bmp_formats() noexcept
  {
    constexpr static auto formats = std::array<siege::fs_string_view, 22>{ { FSL ".jpg",
      FSL ".jpeg",
      FSL ".gif",
      FSL ".png",
      FSL ".tag",
      FSL ".bmp",
      FSL ".pbm",
      FSL ".dds",
      FSL ".ico",
      FSL ".tif",
      FSL ".tiff",
      FSL ".dib",
      FSL ".tim",
      FSL ".pba",
      FSL ".dmb",
      FSL ".db0",
      FSL ".db1",
      FSL ".db2",
      FSL ".hba",
      FSL ".hb0",
      FSL ".hb1",
      FSL ".hb2" } };

    return formats;
  }

  bool is_bmp(std::istream& image_stream) noexcept
  {
    auto path = siege::platform::get_stream_path(image_stream);

    if (path && (path->extension() == ".ico" || path->extension() == ".ICO"))
    {
      return true;
    }

    if (path && (path->extension() == ".tiff" || path->extension() == ".TIFF"))
    {
      return true;
    }

    if (path && (path->extension() == ".tif" || path->extension() == ".TIF"))
    {
      return true;
    }

    auto result = siege::content::bmp::is_earthsiege_bmp(image_stream)
                  || siege::platform::bitmap::is_microsoft_bmp(image_stream)
                  || siege::content::tim::is_tim(image_stream)
                  || siege::content::bmp::is_phoenix_bmp(image_stream)
                  || siege::platform::bitmap::is_jpg(image_stream)
                  || siege::platform::bitmap::is_gif(image_stream)
                  || siege::platform::bitmap::is_png(image_stream);

#if WIN32
    if (!result && !path)
    {
      platform::istream_pos_resetter resetter(image_stream);
      auto file = std::filesystem::temp_directory_path() / L"siege-temp.ico";
      {
        std::ofstream temp(file, std::ios::trunc | std::ios::binary);

        auto size = siege::platform::get_stream_size(image_stream);

        std::vector<std::byte> data(size, std::byte{});

        image_stream.read((char*)data.data(), size);
        temp.write((char*)data.data(), size);
      }

      auto icon = ::LoadImageW(nullptr, file.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

      if (icon)
      {
        ::DestroyIcon((HICON)icon);
        return true;
      }
    }
#endif
    return result;
  }

  auto get_default_palette()
  {
    siege::content::pal::palette temp;

    temp.colours.reserve(255);

    for (auto i = 0; i < 256; ++i)
    {
      temp.colours.emplace_back(std::byte(i), std::byte(i), std::byte(i), std::byte(i));
    }

    return temp;
  }

  std::shared_future<const std::deque<palette_info>&> load_palettes_async(bmp_context& state, std::optional<std::filesystem::path> folder_hint, std::move_only_function<get_embedded_pal_filenames> get_palettes, std::move_only_function<resolve_embedded_pal> resolve_data)
  {
    namespace fs = std::filesystem;

    return std::async(std::launch::async,
      [self = get(state), get_palettes = std::move(get_palettes), resolve_data = std::move(resolve_data)]() mutable -> const std::deque<palette_info>& {
        std::deque<fs::path> pal_paths;
        std::deque<fs::path> embedded_pal_paths;

        for (auto entry = fs::recursive_directory_iterator(fs::current_path());
          entry != fs::recursive_directory_iterator();
          ++entry)
        {
          if (entry.depth() > 3)
          {
            entry.disable_recursion_pending();
            continue;
          }

          if (!fs::is_directory(entry->path()))
          {
            auto is_pal = std::ranges::any_of(get_pal_formats(), [&](auto& ext) { return entry->path().extension() == ext; });

            if (is_pal)
            {
              pal_paths.emplace_back(entry->path());
            }
            else
            {
              auto palettes = get_palettes(entry->path());
              embedded_pal_paths.insert(embedded_pal_paths.end(), palettes.cbegin(), palettes.cend());
            }
          }
        }

        self->palettes.resize(pal_paths.size() + embedded_pal_paths.size());

        auto load_palette = [&](std::filesystem::path&& path, std::istream& temp) -> palette_info {
          std::vector<content::pal::palette> results;
          if (siege::platform::palette::is_microsoft_pal(temp))
          {
            results.emplace_back().colours = siege::platform::palette::get_pal_data(temp);
          }
          else if (siege::content::pal::is_earthsiege_pal(temp))
          {
            results.emplace_back().colours = siege::content::pal::get_earthsiege_pal(temp);
          }
          else if (siege::content::pal::is_phoenix_pal(temp))
          {
            return { path, siege::content::pal::get_ppl_data(temp) };
          }

          if (results.empty())
          {
            results.emplace_back(get_default_palette());
          }

          return { path, results };
        };

        auto task_a = std::async(std::launch::async, [&]() {
          std::transform(embedded_pal_paths.begin(),
            embedded_pal_paths.end(),
            self->palettes.begin() + pal_paths.size(),
            [&resolve_data, &load_palette](auto& path) mutable {
              auto data = resolve_data(path);
              std::spanstream span_data(data);
              return load_palette(std::move(path), span_data);
            });
        });

        std::transform(std::execution::par_unseq, pal_paths.begin(), pal_paths.end(), self->palettes.begin(), [&load_palette](auto& path) mutable {
          std::fstream file_data(path);
          return load_palette(std::move(path), file_data);
        });


        task_a.wait();

        self->selected_palette_file = self->palettes.begin();
        self->selected_palette = 0;

        return self->palettes;
      })
      .share();
  }

  std::size_t load_bitmap(bmp_context& state, std::istream& image_stream, std::shared_future<const std::deque<palette_info>&> pending_load) noexcept
  {
    using namespace siege::content;

    auto* self = get(state);

    try
    {
      if (platform::bitmap::is_microsoft_bmp(image_stream))
      {
        self->original_image.emplace(platform::bitmap::get_bmp_data(image_stream));
      }
      else if (bmp::is_phoenix_bmp(image_stream))
      {
        auto image = bmp::get_pbmp_data(image_stream);

        platform::bitmap::windows_bmp_data dest{};

        dest.info.bit_depth = 32;
        dest.info.width = image.bmp_header.width;
        dest.info.height = image.bmp_header.height;

        pending_load.wait();
        if (!self->palettes.empty())
        {
          self->selected_palette = 0;
          self->selected_palette_file = std::find_if(self->palettes.begin(), self->palettes.end(), [&](palette_info& group) {
            return std::any_of(group.children.begin(), group.children.end(), [&](pal::palette& pal) {
              return pal.index == image.palette_index;
            });
          });

          if (self->selected_palette_file != self->palettes.end())
          {
            auto expected_pal = std::find_if(self->selected_palette_file->children.begin(), self->selected_palette_file->children.end(), [&](pal::palette& pal) {
              return pal.index == image.palette_index;
            });

            self->selected_palette = std::distance(self->selected_palette_file->children.begin(), expected_pal);
            dest.colours = expected_pal->colours;
          }
          else
          {
            self->selected_palette_file = self->palettes.begin();
            dest.colours = self->palettes.begin()->children.begin()->colours;
          }
        }
        else
        {
          dest.colours = get_default_palette().colours;
        }

        dest.indexes.reserve(image.pixels.size());
        std::transform(image.pixels.begin(), image.pixels.end(), std::back_inserter(dest.indexes), [](auto& value) {
          return std::int32_t(value);
        });

        self->original_image.emplace(std::move(dest));
      }
      else if (tim::is_tim(image_stream))
      {
        self->original_image.emplace(tim::get_tim_data_as_bitmap(image_stream));
      }
      else if (bmp::is_earthsiege_bmp(image_stream))
      {
      }
      else
      {
#if WIN32
        auto file = std::filesystem::temp_directory_path() / L"siege-temp.ico";
        {
          platform::istream_pos_resetter resetter(image_stream);

          std::ofstream temp(file, std::ios::trunc | std::ios::binary);

          auto size = siege::platform::get_stream_size(image_stream);

          std::vector<std::byte> data(size, std::byte{});

          image_stream.read((char*)data.data(), size);
          temp.write((char*)data.data(), size);
        }

        auto icon = ::LoadImageW(nullptr, file.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

        if (icon)
        {
          ::DestroyIcon((HICON)icon);
          self->original_image.emplace(file);
        }
        else
#endif
        {
          self->original_image.emplace(image_stream, true);
        }
      }
    }
    catch (...)
    {
    }

    if (self->original_image)
    {
      return self->original_image->frame_count();
    }

    return 0;
  }

  using size = siege::platform::bitmap::size;

  size get_size(const bmp_context& state, std::size_t frame) noexcept
  {
    auto* self = get(state);
    if (self && self->original_image)
    {
      return self->original_image->get_size(frame);
    }

    return {};
  }

  std::size_t get_frame_count(const bmp_context& state) noexcept
  {
    auto* self = get(state);
    if (self && self->original_image)
    {
      return self->original_image->frame_count();
    }

    return {};
  }

  std::pair<const palette_info&, std::size_t> get_selected_palette(const bmp_context& state)
  {
    auto* self = get(state);
    if (!self)
    {
      throw std::runtime_error("Could not get state");
    }

    return std::make_pair(*self->selected_palette_file, self->selected_palette);
  }

#if WIN32
  win32::wic::bitmap_source& get_frame(bmp_context& state, std::size_t frame)
  {
    auto* self = get(state);
    return self->original_image->at(frame);
  }
#endif
}// namespace siege::views