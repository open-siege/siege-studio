#ifndef DARKSTARDTSCONVERTER_DARKSTAR_DTS_VIEW_HPP
#define DARKSTARDTSCONVERTER_DARKSTAR_DTS_VIEW_HPP

#include <glm/gtx/quaternion.hpp>

#include "graphics_view.hpp"
#include "view_context.hpp"
#include <siege/content/renderable_shape.hpp"
//#include "siege/resource/resource_explorer.hpp"
#include <siege/content/dts/darkstar_structures.hpp"

namespace siege::views
{
  class darkstar_dts_view
  {
  public:
    darkstar_dts_view(view_context context, std::istream& shape_stream);
    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks();
    void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);
    void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);
    void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);

  private:
    static std::filesystem::path export_path;
    view_context context;
    std::unique_ptr<content::renderable_shape> shape;

    glm::vec3 translation;
    content::vector3f rotation;

    std::map<std::string, std::function<void(const sf::Event&)>> actions;

    std::map<std::optional<std::string>, std::map<std::string, bool>> visible_nodes;
    std::map<std::string, std::map<std::string, bool>> visible_objects;
    std::vector<std::size_t> detail_level_indexes = { 0 };
    std::vector<content::sequence_info> sequences;
    std::vector<std::string> detail_levels;
    std::vector<content::material> materials;

    bool root_visible = true;
    bool opened_folder = false;
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_DARKSTAR_DTS_VIEW_HPP
