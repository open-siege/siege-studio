#ifndef DARKSTARDTSCONVERTER_DTS_WIDGET_HPP
#define DARKSTARDTSCONVERTER_DTS_WIDGET_HPP

#include "views/graphics_view.hpp"

auto canvas_painter(const std::shared_ptr<wxWindow>& parent, const std::shared_ptr<sf::RenderWindow>& window, ImGuiContext& gui_context, const std::shared_ptr<graphics_view>& handler)
{
  sf::Clock clock;

  auto callbacks = handler->get_callbacks();

  handler->setup_view(*parent, *window, gui_context);

  return [parent, window, gui_context = &gui_context, handler, clock, callbacks](auto& wx_event) mutable {
    wxPaintDC Dc(parent.get());

    sf::Event event{};

    ImGui::SetCurrentContext(gui_context);

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

    handler->render_gl(*parent, *window, *gui_context);

    window->pushGLStates();
    ImGui::SFML::Update(*window, clock.restart());

    handler->render_ui(*parent, *window, *gui_context);

    ImGui::SFML::Render(*window);
    window->popGLStates();
    window->display();
  };
}

#endif//DARKSTARDTSCONVERTER_DTS_WIDGET_HPP
