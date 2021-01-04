#ifndef DARKSTARDTSCONVERTER_VOL_VIEW_HPP
#define DARKSTARDTSCONVERTER_VOL_VIEW_HPP

#include "graphics_view.hpp"
#include "archives/file_system_archive.hpp"

class vol_view : public graphics_view
{
public:
  vol_view(const shared::archive::file_info& info, const studio::fs::file_system_archive& archive);
  bool requires_gl() const override { return false; }
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {};}
  void setup_gl(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override;
  void render_gl(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override {}
  void render_ui(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override {};
private:
  std::vector<shared::archive::file_info> files;
};

#endif//DARKSTARDTSCONVERTER_VOL_VIEW_HPP
