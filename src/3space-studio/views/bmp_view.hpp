#ifndef DARKSTARDTSCONVERTER_BMP_VIEW_HPP
#define DARKSTARDTSCONVERTER_BMP_VIEW_HPP

#include "graphics_view.hpp"
#include "archives/file_system_archive.hpp"
#include "content/palette.hpp"

class bmp_view : public graphics_view
{
public:
  bmp_view(const shared::archive::file_info& info, std::basic_istream<std::byte>& image_stream, const studio::fs::file_system_archive&);
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {};}
  void setup_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override;
  void render_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override {}
  void render_ui(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override;
private:
  std::vector<darkstar::pal::colour> default_colours;
  std::list<std::string> sort_order;
  std::map<std::string_view, std::vector<darkstar::pal::palette>> loaded_palettes;
  std::string_view selected_palette_name;
  std::size_t selected_palette_index = std::string::npos;

  int selected_bitmap_index = 0;

  std::string_view default_palette_name;
  std::size_t default_palette_index = std::string::npos;


  void refresh_image();

  enum class strategy : int
  {
    do_nothing,
    remap,
    remap_unique
  };
  int colour_strategy = 0;

  std::vector<std::vector<std::byte>> original_pixels;

  sf::Image loaded_image;
  sf::Texture texture;
  sf::Sprite sprite;
};

#endif//DARKSTARDTSCONVERTER_BMP_VIEW_HPP
