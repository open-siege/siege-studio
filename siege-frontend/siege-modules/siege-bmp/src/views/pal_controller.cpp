#include "pal_controller.hpp"
#include "content/bmp/bitmap.hpp"

namespace siege::views
{
  bool pal_controller::is_pal(std::istream& image_stream) const
  {
    return studio::content::pal::is_microsoft_pal(image_stream) ||
           studio::content::pal::is_earthsiege_pal(image_stream) ||
           studio::content::pal::is_old_pal(image_stream) ||
           studio::content::pal::is_phoenix_pal(image_stream);
  }

  std::vector<std::vector<pal_controller::palette>> pal_controller::load_palettes(std::istream& image_stream)
  {
    std::vector<std::vector<pal_controller::palette>> all_rectangles;

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
        rectangles.emplace_back(pal_controller::palette {
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

    if (studio::content::pal::is_microsoft_pal(image_stream))
    {
      auto colours = studio::content::pal::get_pal_data(image_stream);
      generate_rectangles(colours);
    }
    else if (studio::content::pal::is_earthsiege_pal(image_stream))
    {
      auto colours = studio::content::pal::get_earthsiege_pal(image_stream);
      generate_rectangles(colours);
    }
    else if (studio::content::pal::is_old_pal(image_stream))
    {
      auto palettes = studio::content::pal::get_old_pal_data(image_stream);

      for (auto& palette : palettes)
      {
        generate_rectangles(palette.colours);
      }

    }
    else if (studio::content::pal::is_phoenix_pal(image_stream))
    {
      auto palettes = studio::content::pal::get_ppl_data(image_stream);

      for (auto& palette : palettes)
      {
        generate_rectangles(palette.colours);
      }
    }

    return all_rectangles;
  }

  
}// namespace studio::views