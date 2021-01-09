#ifndef DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
#define DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP

#include "graphics_view.hpp"
#include "archives/resource_explorer.hpp"

class default_view : public graphics_view
{
public:
  default_view(shared::archive::file_info info);
  bool requires_gl() const override { return false; }
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {};}
  void setup_view(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override;
  void render_gl(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override {}
  void render_ui(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override {}
private:
  shared::archive::file_info info;
};

#endif//DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
