#ifndef DARKSTARDTSCONVERTER_BMP_VIEW_HPP
#define DARKSTARDTSCONVERTER_BMP_VIEW_HPP

#include "graphics_view.hpp"
#include "archives/file_system_archive.hpp"
#include "content/palette.hpp"

class bmp_view : public graphics_view
{
public:
  bmp_view(std::basic_istream<std::byte>& image_stream, const studio::fs::file_system_archive&);
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {};}
  void setup_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override;
  void render_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override {}
  void render_ui(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override;
private:
  std::vector<darkstar::pal::colour> default_colours;
  std::map<std::string, std::vector<darkstar::pal::palette>> loaded_palettes;
  std::vector<darkstar::pal::palette>* selected_palette = nullptr;
  std::string selected_palette_name = "";
  std::size_t selected_index = std::string::npos;

  std::string default_palette_name = "";
  std::size_t default_index = std::string::npos;


  void refresh_image();

  enum class strategy : int
  {
    do_nothing,
    remap,
    remap_unique
  };
  int colour_strategy = 0;

  std::vector<std::byte> original_pixels;

  sf::Image loaded_image;
  sf::Texture texture;
  sf::Sprite sprite;
};

#endif//DARKSTARDTSCONVERTER_BMP_VIEW_HPP
