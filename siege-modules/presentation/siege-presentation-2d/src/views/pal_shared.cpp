#include "2d_shared.hpp"
#include <siege/content/bmp/bitmap.hpp>

namespace siege::views
{
  bool is_pal(std::istream& image_stream) noexcept
  {
    return siege::platform::palette::is_microsoft_pal(image_stream) || siege::content::pal::is_earthsiege_pal(image_stream) || siege::content::pal::is_old_pal(image_stream) || siege::content::pal::is_phoenix_pal(image_stream);
  }

  std::span<const siege::fs_string_view> get_pal_formats() noexcept
  {
    constexpr static auto formats = std::array<siege::fs_string_view, 4>{ { FSL ".pal", FSL ".ipl", FSL ".ppl", FSL ".dpl" } };
    return formats;
  }

  std::size_t load_palettes(pal_context& state, std::istream& image_stream)
  {
    std::vector<std::vector<colour_entry>> all_rectangles;

    auto generate_rectangles = [&](auto& colours) {
      auto& rectangles = all_rectangles.emplace_back();

      rectangles.reserve(colours.size());

      int x = 0;
      int y = 0;
      auto size = int(colours.size() / 8);

      std::u8string name = u8"Palette ";

      int index = 1;

      for (auto& colour : colours)
      {
        auto temp = std::to_string(index++);
        rectangles.emplace_back(colour_entry{
          .position = rect{
            .x = x,
            .y = y,
            .width = size,
            .height = size },
          .colour = colour,
          .name = name + std::u8string(temp.begin(), temp.end()) });

        y += size;

        if (y == colours.size())
        {
          y = 0;
          x += size;
        }
      }
    };

    if (siege::platform::palette::is_microsoft_pal(image_stream))
    {
      auto colours = siege::platform::palette::get_pal_data(image_stream);
      generate_rectangles(colours);
    }
    else if (siege::content::pal::is_earthsiege_pal(image_stream))
    {
      auto colours = siege::content::pal::get_earthsiege_pal(image_stream);
      generate_rectangles(colours);
    }
    else if (siege::content::pal::is_old_pal(image_stream))
    {
      auto palettes = siege::content::pal::get_old_pal_data(image_stream);

      for (auto& palette : palettes)
      {
        generate_rectangles(palette.colours);
      }
    }
    else if (siege::content::pal::is_phoenix_pal(image_stream))
    {
      auto palettes = siege::content::pal::get_ppl_data(image_stream);

      for (auto& palette : palettes)
      {
        generate_rectangles(palette.colours);
      }
    }

    auto size = all_rectangles.size();
    state = std::move(all_rectangles);
    return size;
  }

  std::span<const colour_entry> get_palette(const pal_context& state, std::size_t index)
  {
    auto* palettes = std::any_cast<std::vector<std::vector<colour_entry>>>(&state);
    if (!palettes)
    {
      return {};
    }

    return palettes->operator[](index);
  }


}// namespace siege::views