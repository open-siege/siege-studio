#ifndef DARKSTARDTSCONVERTER_SFX_VIEW_HPP
#define DARKSTARDTSCONVERTER_SFX_VIEW_HPP

#include "graphics_view.hpp"
#include "siege/resource/resource_explorer.hpp"
#include <SFML/Audio.hpp>

namespace siege::views
{
  class sfx_view
  {
  public:
    explicit sfx_view(siege::platform::file_info, std::istream&, const siege::resource::resource_explorer&);
    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() { return {}; }
    void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);
    void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) {}
    void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);

  private:
    static std::filesystem::path export_path;
    const siege::resource::resource_explorer& explorer;
    siege::platform::file_info info;
    std::string original_data;
    sf::SoundBuffer buffer;
    sf::Sound sound;
    bool opened_folder = false;
  };
}

#endif//DARKSTARDTSCONVERTER_SFX_VIEW_HPP
