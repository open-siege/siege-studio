#ifndef DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
#define DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP

#include "graphics_view.hpp"

class default_view : public graphics_view
{
  bool requires_gl() const override { return true; }
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() { return {};}
  void setup_view(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) {}
  void render_gl(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) {}
  void render_ui(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) {}
};

#endif//DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
