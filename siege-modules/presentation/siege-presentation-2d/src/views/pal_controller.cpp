#include "pal_controller.hpp"
#include <siege/content/bmp/bitmap.hpp>

namespace siege::views
{
  bool pal_controller::is_pal(std::istream& image_stream)
  {
    return siege::platform::palette::is_microsoft_pal(image_stream) ||
           siege::content::pal::is_earthsiege_pal(image_stream) ||
           siege::content::pal::is_old_pal(image_stream) ||
           siege::content::pal::is_phoenix_pal(image_stream);
  }

  std::size_t pal_controller::load_palettes(std::istream& image_stream)
  {
    std::vector<std::vector<pal_controller::colour_entry>> all_rectangles;

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
        rectangles.emplace_back(pal_controller::colour_entry {
          .position = pal_controller::rect {
            .x = x,
            .y = y,
            .width = size,
            .height = size
          },
          .colour = colour,
          .name = name + std::u8string(temp.begin(), temp.end())
        });

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

    this->palettes = std::move(all_rectangles);
    return this->palettes.size();
  }

  const std::vector<pal_controller::colour_entry>& pal_controller::get_palette(std::size_t index) const
  {
    return palettes[index];
  }

  
}// namespace siege::views