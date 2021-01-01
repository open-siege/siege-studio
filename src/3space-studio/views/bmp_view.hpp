#ifndef DARKSTARDTSCONVERTER_BMP_VIEW_HPP
#define DARKSTARDTSCONVERTER_BMP_VIEW_HPP

#include "graphics_view.hpp"

class bmp_view : public graphics_view
{
public:
  bmp_view(std::basic_istream<std::byte>& image_stream);
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {};}
  void setup_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override;
  void render_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override {}
  void render_ui(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override;
private:
  sf::Image loaded_image;
  sf::Texture texture;
  sf::Sprite sprite;
};

#endif//DARKSTARDTSCONVERTER_BMP_VIEW_HPP
