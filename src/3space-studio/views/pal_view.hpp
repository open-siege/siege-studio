#ifndef DARKSTARDTSCONVERTER_PAL_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_VIEW_HPP

#include "graphics_view.hpp"

class pal_view : public graphics_view
{
public:
  pal_view(std::basic_istream<std::byte>& image_stream);
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {};}
  void setup_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override;
  void render_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override {}
  void render_ui(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) override;
private:
  std::vector<sf::RectangleShape> rectangles;
};

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
