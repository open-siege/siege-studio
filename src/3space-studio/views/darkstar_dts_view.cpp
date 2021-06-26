#include <execution>

#include "darkstar_dts_view.hpp"
#include "content/dts/renderable_shape_factory.hpp"
#include "gl_renderer.hpp"
#include "sfml_keys.hpp"
#include "3space-studio/utility.hpp"
#include "content/obj_renderer.hpp"

namespace studio::views
{
  std::filesystem::path darkstar_dts_view::export_path = std::filesystem::path();

  // TODO see if this is still needed.
  //  content::dts::darkstar::shape_variant get_shape(std::basic_istream<std::byte>& shape_stream)
  //  {
  //    try
  //    {
  //
  //    }
  //    catch (const std::exception& ex)
  //    {
  //      wxMessageBox(ex.what(), "Error Loading Model.", wxICON_ERROR);
  //      return content::dts::darkstar::shape_variant{};
  //    }
  //  }

  void render_tree_view(const std::string& node, bool& node_visible, std::map<std::optional<std::string>, std::map<std::string, bool>>& visible_nodes, std::map<std::string, std::map<std::string, bool>>& visible_objects)
  {
    ImGui::Checkbox(node.c_str(), &node_visible);
    ImGui::Indent(8);

    if (visible_objects[node].size() > 1)
    {
      for (auto& [child_object, object_visible] : visible_objects[node])
      {
        if (node == child_object)
        {
          ImGui::Checkbox((child_object + " (object)").c_str(), &object_visible);
        }
        else
        {
          ImGui::Checkbox(child_object.c_str(), &object_visible);
        }
      }
    }

    for (auto& [child_node, child_node_visible] : visible_nodes[node])
    {
      render_tree_view(child_node, child_node_visible, visible_nodes, visible_objects);
    }

    ImGui::Unindent(8);
  }

  darkstar_dts_view::darkstar_dts_view(const studio::resources::file_info& info, std::basic_istream<std::byte>& shape_stream, const studio::resources::resource_explorer& archive)
    : info(info), archive(archive)
  {
    if (export_path == std::filesystem::path())
    {
      export_path = archive.get_search_path() / "exported";
      std::filesystem::create_directory(export_path);
    }

    shape = content::dts::make_shape(shape_stream);
    translation = { 0, 0, -20 };
    rotation = { 115, 180, -35 };

    actions.emplace("increase_x_rotation", [&](const sf::Event&) { rotation.x++; });
    actions.emplace("decrease_x_rotation", [&](const sf::Event&) { rotation.x--; });

    actions.emplace("increase_y_rotation", [&](const sf::Event&) { rotation.y++; });
    actions.emplace("decrease_y_rotation", [&](const sf::Event&) { rotation.y--; });
    actions.emplace("increase_z_rotation", [&](const sf::Event&) { rotation.z++; });
    actions.emplace("decrease_z_rotation", [&](const sf::Event&) { rotation.z--; });

    actions.emplace("zoom_in", [&](const sf::Event&) { translation.z++; });
    actions.emplace("zoom_out", [&](const sf::Event&) { translation.z--; });

    actions.emplace("pan_left", [&](const sf::Event&) { translation.x--; });
    actions.emplace("pan_right", [&](const sf::Event&) { translation.x++; });

    actions.emplace("pan_up", [&](const sf::Event&) { translation.y++; });
    actions.emplace("pan_down", [&](const sf::Event&) { translation.y--; });

    sequences = shape->get_sequences(detail_level_indexes);
    detail_levels = shape->get_detail_levels();
  }

  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> darkstar_dts_view::get_callbacks()
  {
    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> callbacks;

    //TODO read this from config file
    callbacks.emplace(get_key_for_name("Numpad4"), std::ref(actions.at("pan_left")));
    callbacks.emplace(get_key_for_name("Numpad6"), std::ref(actions.at("pan_right")));
    callbacks.emplace(get_key_for_name("Numpad8"), std::ref(actions.at("pan_up")));
    callbacks.emplace(get_key_for_name("Numpad2"), std::ref(actions.at("pan_down")));
    callbacks.emplace(get_key_for_name("Numpad1"), std::ref(actions.at("increase_x_rotation")));
    callbacks.emplace(get_key_for_name("Numpad3"), std::ref(actions.at("decrease_x_rotation")));
    callbacks.emplace(get_key_for_name("Numpad7"), std::ref(actions.at("increase_z_rotation")));
    callbacks.emplace(get_key_for_name("Numpad9"), std::ref(actions.at("decrease_z_rotation")));

    callbacks.emplace(get_key_for_name("Add"), std::ref(actions.at("zoom_in")));
    callbacks.emplace(get_key_for_name("Subtract"), std::ref(actions.at("zoom_out")));

    callbacks.emplace(get_key_for_name("Divide"), std::ref(actions.at("increase_y_rotation")));
    callbacks.emplace(get_key_for_name("Insert"), std::ref(actions.at("increase_y_rotation")));
    callbacks.emplace(get_key_for_name("Numpad0"), std::ref(actions.at("increase_y_rotation")));

    callbacks.emplace(get_key_for_name("Multiply"), std::ref(actions.at("decrease_y_rotation")));
    callbacks.emplace(get_key_for_name("Delete"), std::ref(actions.at("decrease_y_rotation")));
    callbacks.emplace(get_key_for_name("Period"), std::ref(actions.at("decrease_y_rotation")));

    return callbacks;
  }

  void darkstar_dts_view::setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
  {
    auto [width, height] = parent.GetClientSize();

    if (height == 0)
    {
      return;
    }

    glViewport(0, 0, width, height);

    glClearDepth(1.f);
    glClearColor(0.3f, 0.3f, 0.3f, 0.f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    perspectiveGL(90.f, double(width) / double(height), 1.f, 1200.0f);
  }

  void darkstar_dts_view::render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext&)
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(translation.x, translation.y, translation.z);

    glRotatef(rotation.x, 1.f, 0.f, 0.f);
    glRotatef(rotation.y, 0.f, 1.f, 0.f);
    glRotatef(rotation.z, 0.f, 0.f, 1.f);

    glBegin(GL_TRIANGLES);
    auto renderer = gl_renderer{ visible_nodes, visible_objects };
    shape->render_shape(renderer, detail_level_indexes, sequences);
    glEnd();
  }

  void darkstar_dts_view::render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& gui_context)
  {
    if (!detail_levels.empty())
    {
      ImGui::Begin("Export Options");

      if (ImGui::Button("Set Export Directory"))
      {
        auto dialog = std::make_unique<wxDirDialog>(nullptr, "Open a folder to export files to");

        if (dialog->ShowModal() == wxID_OK)
        {
          export_path = dialog->GetPath().c_str().AsChar();
        }
      }

      ImGui::SameLine();
      ImGui::Text("%s", export_path.string().c_str());


      if (ImGui::Button("Export to OBJ"))
      {
        for (auto i = 0u; i < detail_levels.size(); ++i)
        {
          auto new_file_name = info.filename.stem().string() + "-" + detail_levels[i] + ".obj";

          std::filesystem::create_directory(export_path);
          std::ofstream output(export_path / new_file_name, std::ios::trunc);
          auto renderer = content::obj_renderer{ output };

          std::vector<std::size_t> details{ i };
          shape->render_shape(renderer, details, sequences);
        }

        if (!opened_folder)
        {
          wxLaunchDefaultApplication(export_path.string());
          opened_folder = true;
        }
      }

      if (ImGui::Button("Export All DTS files to OBJ"))
      {
        auto files = archive.find_files({ ".dts" });

        if (!opened_folder && !files.empty())
        {
          wxLaunchDefaultApplication(export_path.string());
          opened_folder = true;
        }

        std::for_each(std::execution::par_unseq, files.begin(), files.end(), [=](const auto& shape_info) {
          auto archive_path = archive.get_archive_path(shape_info.folder_path);
          auto shape_stream = archive.load_file(shape_info);

          auto real_shape = content::dts::make_shape(*shape_stream.second);

          auto local_detail_levels = real_shape->get_detail_levels();

          for (auto i = 0u; i < local_detail_levels.size(); ++i)
          {
            auto new_file_name = shape_info.filename.stem().string() + "-" + local_detail_levels[i] + ".obj";

            std::filesystem::create_directory(export_path);
            std::ofstream output(export_path / new_file_name, std::ios::trunc);
            auto renderer = content::obj_renderer{ output };

            std::vector<std::size_t> details{ i };

            auto local_sequences = real_shape->get_sequences(details);
            real_shape->render_shape(renderer, details, local_sequences);
          }
        });
      }

      ImGui::End();

      ImGui::Begin("Details and Nodes");

      if (ImGui::CollapsingHeader("Detail Levels", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
      {
        for (auto i = 0u; i < detail_levels.size(); ++i)
        {
          auto selected_item = std::find_if(std::begin(detail_level_indexes), std::end(detail_level_indexes), [i](auto value) {
            return value == i;
          });

          bool is_selected = selected_item != std::end(detail_level_indexes);

          if (ImGui::Checkbox(detail_levels[i].c_str(), &is_selected))
          {
            if (is_selected)
            {
              detail_level_indexes.emplace_back(i);
              sequences = shape->get_sequences(detail_level_indexes);
            }
            else if (selected_item != std::end(detail_level_indexes))
            {
              detail_level_indexes.erase(selected_item);
              sequences = shape->get_sequences(detail_level_indexes);
            }
          }
        }
      }

      if (ImGui::CollapsingHeader("Nodes", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
      {
        for (auto index : detail_level_indexes)
        {
          render_tree_view(detail_levels[index], root_visible, visible_nodes, visible_objects);
        }
      }

      ImGui::End();
    }

    if (!sequences.empty())
    {
      ImGui::Begin("Sequences");

      if (ImGui::CollapsingHeader("Sequences", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
      {
        for (auto it = sequences.begin(); it != sequences.end(); it++)
        {
          auto& sequence = *it;

          if (ImGui::Checkbox(sequence.name.c_str(), &sequence.enabled))
          {
            auto enabled_count = std::count_if(sequence.sub_sequences.begin(), sequence.sub_sequences.end(), [](auto& sub_sequence) {
              return sub_sequence.enabled;
            });

            if (enabled_count == sequence.sub_sequences.size() || enabled_count == 0)
            {
              for (auto& sub_sequence : sequence.sub_sequences)
              {
                sub_sequence.enabled = sequence.enabled;
              }
            }
          }
        }
      }

      if (ImGui::CollapsingHeader("Sub Sequences", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
      {
        for (auto& sequence : sequences)
        {
          ImGui::LabelText("", "%s", sequence.name.c_str());
          for (auto& sub_sequence : sequence.sub_sequences)
          {
            ImGui::Checkbox((sequence.name + "/" + sub_sequence.node_name).c_str(), &sub_sequence.enabled);

            ImGui::SliderInt(sequence.enabled ? " " : "", &sub_sequence.frame_index, 0, sub_sequence.num_key_frames - 1);
          }
        }
      }

      ImGui::End();
    }
  }
}// namespace studio::views