#include "bmp_view.hpp"
#include "archives/bitmap.hpp"

bmp_view::bmp_view(std::basic_istream<std::byte>& image_stream)
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

  sf::IntRect rect;

  rect.width = windows_bmp.info.width;
  rect.height = windows_bmp.info.height;
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