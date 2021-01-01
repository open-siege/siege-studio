#ifndef DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
#define DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP

#include "graphics_view.hpp"

class default_view : public graphics_view
{
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() { return {};}
  void setup_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) {}
  void render_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) {}
  void render_ui(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) {}
};

#endif//DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
