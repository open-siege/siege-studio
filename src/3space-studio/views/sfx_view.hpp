#ifndef DARKSTARDTSCONVERTER_SFX_VIEW_HPP
#define DARKSTARDTSCONVERTER_SFX_VIEW_HPP

#include "graphics_view.hpp"
#include "resource/resource_explorer.hpp"
#include <SFML/Audio.hpp>

namespace studio::views
{
  class sfx_view : public graphics_view
  {
  public:
    explicit sfx_view(studio::resource::file_info, std::basic_istream<std::byte>&, const studio::resource::resource_explorer&);
    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {}; }
    void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) override;
    void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) override {}
    void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) override;

  private:
    static std::filesystem::path export_path;
    const studio::resource::resource_explorer& explorer;
    studio::resource::file_info info;
    std::basic_string<std::byte> original_data;
    sf::SoundBuffer buffer;
    sf::Sound sound;
    bool opened_folder = false;
  };
}

#endif//DARKSTARDTSCONVERTER_SFX_VIEW_HPP
