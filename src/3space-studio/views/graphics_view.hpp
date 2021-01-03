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
#include "archives/archive.hpp"

struct graphics_view : wxClientData
{
  virtual std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() = 0;

  virtual void setup_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) = 0;

  virtual void render_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) = 0;

  virtual void render_ui(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext) = 0;
};


#endif//DARKSTARDTSCONVERTER_GRAPHICS_HANDLER_HPP
