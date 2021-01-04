#ifndef DARKSTARDTSCONVERTER_PAL_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_VIEW_HPP

#include "graphics_view.hpp"

class pal_view : public graphics_view
{
public:
  explicit pal_view(std::basic_istream<std::byte>& image_stream);
  bool requires_gl() const override { return true; }
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {};}
  void setup_view(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override;
  void render_gl(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override {}
  void render_ui(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override;
private:
  std::vector<sf::RectangleShape>* rectangles = nullptr;

  std::vector<std::vector<sf::RectangleShape>> all_rectangles;
};

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
