#include "pal_view.hpp"
#include "content/bitmap.hpp"

namespace studio::views
{

  pal_view::pal_view(std::basic_istream<std::byte>& image_stream)
  {
    auto generate_rectangles = [&](auto& colours) {
      rectangles = &all_rectangles.emplace_back();

      rectangles->reserve(colours.size());

      float x = 0;
      float y = 0;
      auto size = colours.size() / 8;

      for (auto& colour : colours)
      {
        rectangles->emplace_back(sf::Vector2f(size, size));
        auto& rect = rectangles->back();

        rect.setPosition(x, y);
        rect.setFillColor(sf::Color(int(colour.red), int(colour.green), int(colour.blue)));

        y += size;

        if (y == colours.size())
        {
          y = 0;
          x += size;
        }
      }
    };

    if (content::pal::is_microsoft_pal(image_stream))
    {
      auto colours = content::pal::get_pal_data(image_stream);
      generate_rectangles(colours);
    }
    else if (content::pal::is_phoenix_pal(image_stream))
    {
      auto palettes = content::pal::get_ppl_data(image_stream);

      for (auto& palette : palettes)
      {
        generate_rectangles(palette.colours);
      }

      if (!all_rectangles.empty())
      {
        rectangles = &all_rectangles.front();
      }
    }
  }

  void pal_view::render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
  {
    window.clear();

    if (rectangles)
    {
      for (auto& rectangle : *rectangles)
      {
        window.draw(rectangle);
      }

      if (all_rectangles.size() > 1)
      {
        ImGui::Begin("Palettes");
        int index = int(rectangles - all_rectangles.data());

        std::string name = "Palette ";

        for (auto i = 0u; i < all_rectangles.size(); ++i)
        {
          ImGui::RadioButton((name + std::to_string(i + 1)).c_str(), &index, i);
        }

        rectangles = all_rectangles.data() + index;

        ImGui::End();
      }
    }
  }

  void pal_view::setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext&)
  {
    if (rectangles)
    {
      auto [width, height] = parent.GetClientSize();

      auto image_width = rectangles->size() / 8 * rectangles->size();
      auto image_height = rectangles->size() / 8 * (rectangles->size() / 8);

      auto width_ratio = float(image_width) / float(width);
      auto height_ratio = float(image_height) / float(height);

      sf::FloatRect visibleArea(0, 0, image_width, image_height);
      sf::View view(visibleArea);
      view.setViewport(sf::FloatRect(0.5 - width_ratio / 16, 0.5 - height_ratio / 8, width_ratio, height_ratio));
      window.setView(view);
    }
  }
}// namespace studio::views