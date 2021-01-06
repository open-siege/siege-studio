#ifndef DARKSTARDTSCONVERTER_VOL_VIEW_HPP
#define DARKSTARDTSCONVERTER_VOL_VIEW_HPP

#include "graphics_view.hpp"
#include "archives/file_system_archive.hpp"
#include <future>

class vol_view : public graphics_view
{
public:
  vol_view(const shared::archive::file_info& info, const studio::fs::file_system_archive& archive);
  [[nodiscard]] bool requires_gl() const override { return false; }
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override { return {};}
  void setup_view(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override;
  void render_gl(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override {}
  void render_ui(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override {};
private:
  const studio::fs::file_system_archive& archive;
  std::filesystem::path archive_path;
  std::vector<shared::archive::file_info> files;
  std::future<bool> pending_save;
};

#endif//DARKSTARDTSCONVERTER_VOL_VIEW_HPP
