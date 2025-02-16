#include "bmp_controller.hpp"
#include "pal_controller.hpp"
#include <siege/content/bmp/bitmap.hpp>
#include <siege/content/bmp/tim.hpp>
#include <siege/platform/image.hpp>
#include <siege/platform/stream.hpp>
#include <deque>
#include <fstream>
#include <spanstream>
#include <filesystem>
#include <future>
#include <execution>

namespace siege::views
{
  bool bmp_controller::is_bmp(std::istream& image_stream) noexcept
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

    return siege::content::bmp::is_earthsiege_bmp(image_stream)
           || siege::platform::bitmap::is_microsoft_bmp(image_stream)
           || siege::content::tim::is_tim(image_stream)
           || siege::content::bmp::is_phoenix_bmp(image_stream)
           || siege::platform::bitmap::is_jpg(image_stream)
           || siege::platform::bitmap::is_gif(image_stream)
           || siege::platform::bitmap::is_png(image_stream);
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

  std::shared_future<const std::deque<palette_info>&> bmp_controller::load_palettes_async(std::optional<std::filesystem::path> folder_hint,
    std::move_only_function<get_embedded_pal_filenames> get_palettes,
    std::move_only_function<resolve_embedded_pal> resolve_data)
  {
    namespace fs = std::filesystem;

    return std::async(std::launch::async,
      [this, get_palettes = std::move(get_palettes), resolve_data = std::move(resolve_data)]() mutable -> const std::deque<palette_info>& {
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
            auto is_pal = std::any_of(pal_controller::formats.begin(), pal_controller::formats.end(), [&](auto& ext) { return entry->path().extension() == ext; });

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

        palettes.resize(pal_paths.size() + embedded_pal_paths.size());

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
            palettes.begin() + pal_paths.size(),
            [&resolve_data, &load_palette](auto& path) mutable {
              auto data = resolve_data(path);
              std::spanstream span_data(data);
              return load_palette(std::move(path), span_data);
            });
        });

        std::transform(std::execution::par_unseq, pal_paths.begin(), pal_paths.end(), palettes.begin(), [&load_palette](auto& path) mutable {
          std::fstream file_data(path);
          return load_palette(std::move(path), file_data);
        });


        task_a.wait();

        selected_palette_file = palettes.begin();
        selected_palette = 0;

        return palettes;
      }).share();
  }

  std::size_t bmp_controller::load_bitmap(std::istream& image_stream, std::shared_future<const std::deque<palette_info>&> pending_load) noexcept
  {
    using namespace siege::content;

    try
    {
      if (platform::bitmap::is_microsoft_bmp(image_stream))
      {
        original_image.emplace(platform::bitmap::get_bmp_data(image_stream));
      }
      else if (bmp::is_phoenix_bmp(image_stream))
      {
        auto image = bmp::get_pbmp_data(image_stream);

        platform::bitmap::windows_bmp_data dest{};

        dest.info.bit_depth = 32;
        dest.info.width = image.bmp_header.width;
        dest.info.height = image.bmp_header.height;

        pending_load.wait();
        if (!palettes.empty())
        {
          selected_palette = 0;
          selected_palette_file = std::find_if(palettes.begin(), palettes.end(), [&](palette_info& group) {
            return std::any_of(group.children.begin(), group.children.end(), [&](pal::palette& pal) {
              return pal.index == image.palette_index;
            });
          });

          if (selected_palette_file != palettes.end())
          {
            auto expected_pal = std::find_if(selected_palette_file->children.begin(), selected_palette_file->children.end(), [&](pal::palette& pal) {
              return pal.index == image.palette_index;
            });

            selected_palette = std::distance(selected_palette_file->children.begin(), expected_pal);
            dest.colours = expected_pal->colours;
          }
          else
          {
            selected_palette_file = palettes.begin();
            dest.colours = palettes.begin()->children.begin()->colours;
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

        original_image.emplace(std::move(dest));
      }
      else if (tim::is_tim(image_stream))
      {
        original_image.emplace(tim::get_tim_data_as_bitmap(image_stream));
      }
      else if (bmp::is_earthsiege_bmp(image_stream))
      {
      }
      else
      {
        original_image.emplace(image_stream);
      }
    }
    catch (...)
    {
    }

    if (original_image)
    {
      return original_image->frame_count();
    }

    return 0;
  }

  using size = siege::platform::bitmap::size;

  size bmp_controller::get_size(std::size_t frame) const noexcept
  {
    if (original_image)
    {
      return original_image->get_size(frame);
    }

    return {};
  }
}// namespace siege::views