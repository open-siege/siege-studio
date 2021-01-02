#include "bmp_view.hpp"
#include "archives/bitmap.hpp"

bmp_view::bmp_view(std::basic_istream<std::byte>& image_stream, const studio::fs::file_system_archive& manager)
{
  sf::IntRect rect;
  if (darkstar::bmp::is_microsoft_bmp(image_stream))
  {
    auto windows_bmp = darkstar::bmp::get_bmp_data(image_stream);
    std::vector<std::uint8_t> pixels;

    pixels.reserve(windows_bmp.info.width * windows_bmp.info.height * sizeof(std::int32_t));

    for (auto index : windows_bmp.pixels)
    {
      auto& colours = windows_bmp.colours[int(index)];
      pixels.emplace_back(int(colours.red));
      pixels.emplace_back(int(colours.green));
      pixels.emplace_back(int(colours.blue));
      pixels.emplace_back(int(colours.flags));
    }

    loaded_image.create(windows_bmp.info.width, windows_bmp.info.height, pixels.data());

    rect.width = windows_bmp.info.width;
    rect.height = windows_bmp.info.height;
  }
  else if (darkstar::bmp::is_phoenix_bmp(image_stream))
  {
    auto phoenix_bmp = darkstar::bmp::get_pbmp_data(image_stream);
    std::vector<std::uint8_t> pixels;

    pixels.reserve(phoenix_bmp.bmp_header.width * phoenix_bmp.bmp_header.height * sizeof(std::int32_t));

    auto palettes = manager.find_files({".ppl", ".PPL", ".ipl", ".IPL", ".pal", ".PAL"});

    std::vector<darkstar::pal::colour> colours;

    for (auto& palette_info : palettes)
    {
      if (!colours.empty())
      {
        break;
      }

      auto raw_palette = manager.load_file(palette_info);

      if (darkstar::pal::is_phoenix_pal(*raw_palette))
      {
        auto child_palettes = darkstar::pal::get_ppl_data(*raw_palette);

        for (auto& child : child_palettes)
        {
          if (child.index == phoenix_bmp.palette_index)
          {
            colours.insert(colours.begin(), child.colours.begin(), child.colours.end());
            break;
          }
        }
      }
    }

    if (colours.empty())
    {
      colours.reserve(256);
      for (auto i = 0; i < 256; ++i)
      {
        auto colour = std::byte(i);
        colours.emplace_back(darkstar::pal::colour{colour, colour, colour, std::byte(0xFF)});
      }
    }

    for (auto index : phoenix_bmp.pixels)
    {
      auto& colour = colours[int(index)];
      pixels.emplace_back(int(colour.red));
      pixels.emplace_back(int(colour.green));
      pixels.emplace_back(int(colour.blue));
      pixels.emplace_back(int(colour.flags));
    }

    loaded_image.create(phoenix_bmp.bmp_header.width, phoenix_bmp.bmp_header.height, pixels.data());

    rect.width = phoenix_bmp.bmp_header.width;
    rect.height = phoenix_bmp.bmp_header.height;
  }

  rect.top = 0;
  rect.left = 0;
  texture.loadFromImage(loaded_image, rect);
  sprite.setTexture(texture);
}

void bmp_view::render_ui(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext)
{
  window->clear();
  window->draw(sprite);
}

void bmp_view::setup_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext)
{
  auto [width, height] = parent->GetClientSize();

  auto image_width = sprite.getTexture()->getSize().x;
  auto image_height = sprite.getTexture()->getSize().y;

  auto width_ratio = float(image_width) / width;
  auto height_ratio = float(image_height) / height;

  sf::FloatRect visibleArea(0, 0, image_width, image_height);
  sf::View view(visibleArea);
  view.setViewport(sf::FloatRect(0.5 - width_ratio / 2, 0.5 - height_ratio / 2, width_ratio, height_ratio));
  window->setView(view);
}