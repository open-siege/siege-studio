#ifndef DARKSTARDTSCONVERTER_GRAPHICS_HANDLER_HPP
#define DARKSTARDTSCONVERTER_GRAPHICS_HANDLER_HPP

#include <optional>
#include <map>
#include <functional>
#include <istream>
#include <string>

#include <wx/wx.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include "resources/archive_plugin.hpp"

namespace studio::views
{
  struct studio_view : wxClientData
  {
  };

  struct normal_view : studio_view
  {
    virtual void setup_view(wxWindow& parent) = 0;
  };

  struct graphics_view : studio_view
  {
    virtual std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() = 0;

    virtual void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& gui_context) = 0;

    virtual void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) = 0;

    virtual void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) = 0;
  };
}// namespace studio::views

#endif//DARKSTARDTSCONVERTER_GRAPHICS_HANDLER_HPP
