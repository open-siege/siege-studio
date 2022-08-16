#ifndef DARKSTARDTSCONVERTER_PAL_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_VIEW_HPP

#include "graphics_view.hpp"

namespace studio::views
{
  class pal_view
  {
  public:
    explicit pal_view(std::istream& image_stream);
    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() { return {}; }
    void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);
    void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) {}
    void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);

  private:
    std::vector<sf::RectangleShape>* rectangles = nullptr;

    std::vector<std::vector<sf::RectangleShape>> all_rectangles;
  };
}// namespace studio::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
