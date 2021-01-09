#ifndef DARKSTARDTSCONVERTER_DARKSTAR_DTS_VIEW_HPP
#define DARKSTARDTSCONVERTER_DARKSTAR_DTS_VIEW_HPP

#include <glm/gtx/quaternion.hpp>

#include "graphics_view.hpp"
#include "renderable_shape.hpp"
#include "archives/resource_explorer.hpp"

class darkstar_dts_view : public graphics_view
{
public:
  darkstar_dts_view(const shared::archive::file_info&, std::basic_istream<std::byte>& shape_stream, const studio::fs::resource_explorer& archive);
  bool requires_gl() const override { return true; }
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override;
  void setup_view(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override;
  void render_gl(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override;
  void render_ui(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext) override;

private:
  static std::filesystem::path export_path;
  const studio::fs::resource_explorer& archive;
  shared::archive::file_info info;
  std::unique_ptr<renderable_shape> shape;

  glm::vec3 translation;
  darkstar::dts::vector3f rotation;

  std::map<std::string, std::function<void(const sf::Event&)>> actions;

  std::map<std::optional<std::string>, std::map<std::string, bool>> visible_nodes;
  std::map<std::string, std::map<std::string, bool>> visible_objects;
  std::vector<std::size_t> detail_level_indexes = { 0 };
  std::vector<sequence_info> sequences;
  std::vector<std::string> detail_levels;

  bool root_visible = true;
  bool opened_folder = false;
};

#endif//DARKSTARDTSCONVERTER_DARKSTAR_DTS_VIEW_HPP
