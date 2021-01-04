#ifndef DARKSTARDTSCONVERTER_DTS_WIDGET_HPP
#define DARKSTARDTSCONVERTER_DTS_WIDGET_HPP

#include "views/graphics_view.hpp"

using optional_istream = std::optional<std::reference_wrapper<std::basic_istream<std::byte>>>;

auto canvas_painter(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext, graphics_view* handler)
{
  sf::Clock clock;

  auto callbacks = handler->get_callbacks();

  handler->setup_view(parent, window, guiContext);

  return [=](auto& wx_event) mutable {
    wxPaintDC Dc(parent);

    sf::Event event;

    ImGui::SetCurrentContext(guiContext);

    while (window->pollEvent(event))
    {
      ImGui::SFML::ProcessEvent(event);

      if (event.type == sf::Event::KeyPressed && (event.key.code != sf::Keyboard::Escape))
      {
        const auto callback = callbacks.find(event.key.code);

        if (callback != callbacks.end())
        {
          callback->second(event);
        }
      }

      if (event.type == sf::Event::Closed)
      {
        window->close();
      }

      if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
      {
        window->close();
      }
    }

    handler->render_gl(parent, window, guiContext);

    window->pushGLStates();
    ImGui::SFML::Update(*window, clock.restart());

    handler->render_ui(parent, window, guiContext);

    ImGui::SFML::Render(*window);
    window->popGLStates();
    window->display();
  };
}

#endif//DARKSTARDTSCONVERTER_DTS_WIDGET_HPP
