#ifndef DARKSTARDTSCONVERTER_GRAPHICS_HANDLER_HPP
#define DARKSTARDTSCONVERTER_GRAPHICS_HANDLER_HPP

#include <optional>
#include <map>
#include <functional>
#include <istream>
#include <string>
#include <memory>


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
  struct normal_view
  {
    void setup_view(wxWindow& parent)
    {
      instance->setup_view(parent);
    }

    template<typename T>
    normal_view(T view) : instance(std::make_unique<view_virt<T>>(std::move(view))) {}

  private:
    struct normal_view_virt
    {
      virtual void setup_view(wxWindow& parent) = 0;
      virtual ~normal_view_virt() = default;
    };


    template<typename T>
    struct view_virt final : normal_view_virt
    {
      view_virt(T obj) : self(std::move(obj)) {}
      void setup_view(wxWindow& parent) override
      {
        self.setup_view(parent);
      }

      T self;
    };


    std::unique_ptr<normal_view_virt> instance;
  };

  struct graphics_view
  {
    template<typename T>
    graphics_view(T view) : instance(std::make_unique<view_virt<T>>(std::move(view))) {}

    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks()
    {
      return instance->get_callbacks();
    }

    void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
    {
      instance->setup_view(parent, window, guiContext);
    }

    void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
    {
      instance->render_gl(parent, window, guiContext);
    }

    void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
    {
      instance->render_ui(parent, window, guiContext);
    }
  private:
    struct graphics_view_impl
    {
      virtual std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() = 0;

      virtual void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& gui_context) = 0;

      virtual void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) = 0;

      virtual void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) = 0;
      virtual ~graphics_view_impl() = default;
    };


    template<typename T>
    struct view_virt final : graphics_view_impl
    {
      view_virt(T obj) : self(std::move(obj)) {}

      std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override
      {
        return self.get_callbacks();
      }

      void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& gui_context) override
      {
        self.setup_view(parent, window, gui_context);
      }

      void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
      {
        self.render_gl(parent, window, guiContext);
      }

      void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
      {
        self.render_ui(parent, window, guiContext);
      }

      T self;
    };


    std::unique_ptr<graphics_view_impl> instance;
  };
}// namespace studio::views

#endif// DARKSTARDTSCONVERTER_GRAPHICS_HANDLER_HPP
