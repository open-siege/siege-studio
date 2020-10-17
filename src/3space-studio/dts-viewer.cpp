#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <deque>
#include <memory>

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <glm/gtx/quaternion.hpp>

#include "dts_io.hpp"
#include "dts_renderable_shape.hpp"
#include "gl_renderer.hpp"
#include "sfml_keys.hpp"
#include "utility.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;

struct shape_instance
{
  std::unique_ptr<renderable_shape> shape;

  glm::vec3 translation;
  dts::vector3f rotation;
};

std::optional<std::filesystem::path> get_shape_path()
{
  constexpr static auto filetypes = "Darkstar DTS files|*.dts;*.DTS";
  auto dialog = std::make_unique<wxFileDialog>(nullptr, "Open a Darkstar DTS File", "", "", filetypes, wxFD_OPEN, wxDefaultPosition);

  if (dialog->ShowModal() == wxID_OK)
  {
    const auto buffer = dialog->GetPath().ToAscii();
    return std::string_view{ buffer.data(), buffer.length() };
  }

  return std::nullopt;
}

std::optional<std::filesystem::path> get_workspace_path()
{
  auto dialog = std::make_unique<wxDirDialog>(nullptr, "Open a folder to use as a workspace");

  if (dialog->ShowModal() == wxID_OK)
  {
    const auto buffer = dialog->GetPath().ToAscii();
    return std::string_view{ buffer.data(), buffer.length() };
  }

  return std::nullopt;
}

std::optional<std::filesystem::path> get_shape_path(int argc, char** argv)
{
  if (argc > 1)
  {
    return argv[1];
  }

  return get_shape_path();
}

dts::shape_variant get_shape(std::optional<std::filesystem::path> shape_path)
{
  if (!shape_path.has_value())
  {
    return dts::shape_variant{};
  }

  try
  {
    std::basic_ifstream<std::byte> input(shape_path.value(), std::ios::binary);
    return dts::read_shape(shape_path.value(), input, std::nullopt);
  }
  catch (const std::exception& ex)
  {
    wxMessageBox(ex.what(), "Error Loading Model.", wxICON_ERROR);
    return dts::shape_variant{};
  }
}

void setup_opengl(wxControl* parent)
{
  auto [width, height] = parent->GetClientSize();

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

auto apply_configuration(std::map<std::string, std::function<void(shape_instance&)>>& actions)
{
  //TODO read this from config one day
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(shape_instance&)>>> callbacks;
  callbacks.emplace(config::get_key_for_name("Numpad4"), std::ref(actions.at("pan_left")));
  callbacks.emplace(config::get_key_for_name("Numpad6"), std::ref(actions.at("pan_right")));
  callbacks.emplace(config::get_key_for_name("Numpad8"), std::ref(actions.at("pan_up")));
  callbacks.emplace(config::get_key_for_name("Numpad2"), std::ref(actions.at("pan_down")));
  callbacks.emplace(config::get_key_for_name("Numpad1"), std::ref(actions.at("increase_x_rotation")));
  callbacks.emplace(config::get_key_for_name("Numpad3"), std::ref(actions.at("decrease_x_rotation")));
  callbacks.emplace(config::get_key_for_name("Numpad7"), std::ref(actions.at("increase_z_rotation")));
  callbacks.emplace(config::get_key_for_name("Numpad9"), std::ref(actions.at("decrease_z_rotation")));

  callbacks.emplace(config::get_key_for_name("Add"), std::ref(actions.at("zoom_in")));
  callbacks.emplace(config::get_key_for_name("Subtract"), std::ref(actions.at("zoom_out")));

  callbacks.emplace(config::get_key_for_name("Divide"), std::ref(actions.at("increase_y_rotation")));
  callbacks.emplace(config::get_key_for_name("Insert"), std::ref(actions.at("increase_y_rotation")));
  callbacks.emplace(config::get_key_for_name("Numpad0"), std::ref(actions.at("increase_y_rotation")));

  callbacks.emplace(config::get_key_for_name("Multiply"), std::ref(actions.at("decrease_y_rotation")));
  callbacks.emplace(config::get_key_for_name("Delete"), std::ref(actions.at("decrease_y_rotation")));
  callbacks.emplace(config::get_key_for_name("Period"), std::ref(actions.at("decrease_y_rotation")));

  return callbacks;
}


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

auto renderer_main(std::optional<std::filesystem::path> shape_path, sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext)
{
  static std::map<std::optional<std::filesystem::path>, shape_instance> shape_instances;

  auto instance_iterator = shape_instances.emplace(shape_path, shape_instance{ std::make_unique<dts_renderable_shape>(get_shape(shape_path)), { 0, 0, -20 }, { 115, 180, -35 } });
  auto& instance = instance_iterator.first;

  static std::map<std::string, std::function<void(shape_instance&)>> actions;

  actions.emplace("increase_x_rotation", [](auto& instance) { instance.rotation.x++; });
  actions.emplace("decrease_x_rotation", [](auto& instance) { instance.rotation.x--; });

  actions.emplace("increase_y_rotation", [](auto& instance) { instance.rotation.y++; });
  actions.emplace("decrease_y_rotation", [](auto& instance) { instance.rotation.y--; });
  actions.emplace("increase_z_rotation", [](auto& instance) { instance.rotation.z++; });
  actions.emplace("decrease_z_rotation", [](auto& instance) { instance.rotation.z--; });

  actions.emplace("zoom_in", [](auto& instance) { instance.translation.z++; });
  actions.emplace("zoom_out", [](auto& instance) { instance.translation.z--; });

  actions.emplace("pan_left", [](auto& instance) { instance.translation.x--; });
  actions.emplace("pan_right", [](auto& instance) { instance.translation.x++; });

  actions.emplace("pan_up", [](auto& instance) { instance.translation.y++; });
  actions.emplace("pan_down", [](auto& instance) { instance.translation.y--; });


  std::map<std::optional<std::string>, std::map<std::string, bool>> visible_nodes;
  std::map<std::string, std::map<std::string, bool>> visible_objects;
  auto detail_levels = instance->second.shape->get_detail_levels();
  std::vector<std::size_t> detail_level_indexes = { 0 };
  auto sequences = instance->second.shape->get_sequences(detail_level_indexes);

  bool root_visible = true;

  auto qRot1 = glm::quat(1.f, 0.f, 0.f, 0.f);

  auto callbacks = apply_configuration(actions);

  sf::Clock clock;

  setup_opengl(parent);

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
          callback->second(instance->second);
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

    auto& [shape, translation, rotation] = instance->second;

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

    window->pushGLStates();

    ImGui::SFML::Update(*window, clock.restart());

    if (!detail_levels.empty())
    {
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
          ImGui::LabelText("", sequence.name.c_str());
          for (auto& sub_sequence : sequence.sub_sequences)
          {
            ImGui::Checkbox((sequence.name + "/" + sub_sequence.node_name).c_str(), &sub_sequence.enabled);

            ImGui::SliderInt(sequence.enabled ? " " : "", &sub_sequence.frame_index, 0, sub_sequence.num_key_frames - 1);
          }
        }
      }

      ImGui::End();
    }


    ImGui::SFML::Render(*window);
    window->popGLStates();
    window->display();
  };
}

wxAppConsole* createApp()
{
  wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
    "Hello wxWidgets");
  return new wxApp();
}

constexpr auto event_open_in_new_tab = 1;
constexpr auto event_open_folder_as_workspace = 2;

wxMenuBar* create_menu_bar()
{
  auto* menuFile = new wxMenu();
  menuFile->Append(wxID_OPEN);
  menuFile->Append(event_open_in_new_tab, "Open in New Tab...");
  menuFile->Append(event_open_folder_as_workspace, "Open Folder as Workspace");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  auto* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  auto* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");

  return menuBar;
}

void create_render_view(wxWindow* panel, std::optional<std::filesystem::path> path)
{
  auto* graphics = new wxControl(panel, -1, wxDefaultPosition, wxDefaultSize, 0);

  panel->GetSizer()->Add(graphics, 1, wxEXPAND | wxALL, 5);

  sf::ContextSettings context;
  context.depthBits = 24;
  auto* window = new sf::RenderWindow(get_handle(graphics), context);
  static bool is_init = false;
  static ImGuiContext* primary_gui_context;

  ImGuiContext* gui_context;

  if (!is_init)
  {
    ImGui::SFML::Init(*window);

    primary_gui_context = gui_context = ImGui::GetCurrentContext();
    is_init = true;
  }
  else
  {
    gui_context = ImGui::CreateContext(ImGui::GetIO().Fonts);
  }

  graphics->Bind(wxEVT_ERASE_BACKGROUND, [](auto& event) {});

  graphics->Bind(wxEVT_SIZE, [=](auto& event) {
    setup_opengl(graphics);
  });

  graphics->Bind(wxEVT_IDLE, [=](auto& event) {
    graphics->Refresh();
  });
  graphics->Bind(wxEVT_PAINT, renderer_main(path, window, graphics, gui_context));

  graphics->Bind(wxEVT_DESTROY, [=](auto& event) {
    delete window;
    if (gui_context != primary_gui_context)
    {
      ImGui::DestroyContext(gui_context);
      ImGui::SetCurrentContext(primary_gui_context);
    }
  });
}

auto get_path_from_tree_item(wxTreeCtrl* tree_view, wxTreeItemId item, const fs::path& search_path)
{
  std::deque<std::filesystem::path> path_fragments;

  auto parent_item = tree_view->GetItemParent(item);

  do
  {
    if (parent_item != tree_view->GetRootItem())
    {
      path_fragments.emplace_front(tree_view->GetItemText(parent_item).ToAscii().data());
      parent_item = tree_view->GetItemParent(parent_item);
    }
  } while (parent_item != tree_view->GetRootItem());

  path_fragments.emplace_back(std::filesystem::path{ tree_view->GetItemText(item).ToAscii().data() });

  auto new_path = search_path;

  for (auto& path : path_fragments)
  {
    new_path = new_path / path;
  }

  return new_path;
}

void populate_tree_view(wxTreeCtrl* tree_view, fs::path search_path, std::optional<wxTreeItemId> parent = std::nullopt)
{
  using namespace std::literals;
  constexpr std::array extensions = { ".dts"sv, ".DTS"sv };

  if (!fs::is_directory(search_path))
  {
    return;
  }

  bool is_root = false;
  if (!parent.has_value())
  {
    tree_view->DeleteAllItems();
    is_root = true;
  }

  parent = parent.has_value() ? parent : tree_view->AddRoot(search_path.string());

  for (auto& item : fs::directory_iterator(search_path))
  {
    if (item.is_directory())
    {
      auto new_parent = tree_view->AppendItem(parent.value(), item.path().stem().string());

      if (is_root)
      {
        populate_tree_view(tree_view, item, new_parent);
      }
    }
  }

  for (auto& item : fs::directory_iterator(search_path))
  {
    if (!item.is_directory())
    {
      auto filename = item.path().filename().string();
      if (std::any_of(extensions.begin(), extensions.end(), [&filename](const auto& ext) {
            return ends_with(filename, ext);
          }))
      {
        tree_view->AppendItem(parent.value(), filename);
      }
    }
  }

  if (is_root)
  {
    tree_view->Expand(parent.value());
  }
}

int main(int argc, char** argv)
{
  auto search_path = fs::current_path();

  wxApp::SetInitializerFunction(createApp);
  wxEntryStart(argc, argv);
  auto* app = wxApp::GetInstance();
  app->CallOnInit();

  auto* frame = new wxFrame(nullptr, wxID_ANY, "3Space Studio");

  frame->Bind(
    wxEVT_MENU, [](auto& event) {
      wxMessageBox("This is a tool to explore files using the 3Space or Darkstar engines. Currently only Starsiege, Starsiege Tribes, Trophy Bass 3D and Front Page Sports: Ski Racing are supported.",
        "About 3Space Studio",
        wxOK | wxICON_INFORMATION);
    },
    wxID_ABOUT);

  auto* sizer = new wxBoxSizer(wxHORIZONTAL);

  auto* tree_view = new wxTreeCtrl(frame);
  sizer->Add(tree_view, 20, wxEXPAND, 0);

  populate_tree_view(tree_view, search_path);

  wxAuiNotebook* notebook = new wxAuiNotebook(frame, wxID_ANY);
  auto num_elements = notebook->GetPageCount();

  sizer->Add(notebook, 80, wxEXPAND, 0);

  auto add_element_from_file = [notebook, &num_elements](auto& new_path, bool replace_selection = false) {
    if (fs::is_directory(new_path))
    {
      return;
    }

    wxPanel* panel = new wxPanel(notebook, wxID_ANY);
    panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    create_render_view(panel, new_path);

    if (replace_selection)
    {
      auto selection = notebook->GetSelection();
      notebook->InsertPage(selection, panel, new_path.filename().string());
      num_elements = notebook->GetPageCount();

      if (num_elements > 2)
      {
        notebook->DeletePage(selection + 1);
      }

      notebook->ChangeSelection(selection);
    }
    else
    {
      notebook->InsertPage(notebook->GetPageCount() - 1, panel, new_path.filename().string());
      num_elements = notebook->GetPageCount();
      notebook->ChangeSelection(notebook->GetPageCount() - 2);
    }
  };

  auto add_new_element = [notebook, &num_elements]() {
    wxPanel* panel = new wxPanel(notebook, wxID_ANY);
    panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    create_render_view(panel, std::nullopt);
    notebook->InsertPage(notebook->GetPageCount() - 1, panel, "New Tab");
    notebook->ChangeSelection(notebook->GetPageCount() - 2);
    num_elements = notebook->GetPageCount();
  };

  tree_view->Bind(wxEVT_TREE_ITEM_EXPANDING, [tree_view, &search_path](wxTreeEvent& event) {
    auto item = event.GetItem();

    if (item == tree_view->GetRootItem())
    {
      return;
    }

    if (tree_view->HasChildren(item))
    {
      wxTreeItemIdValue cookie = nullptr;

      auto child = tree_view->GetFirstChild(item, cookie);

      if (cookie && !tree_view->HasChildren(child))
      {
        populate_tree_view(tree_view, get_path_from_tree_item(tree_view, child, search_path), child);
      }

      do {
        child = tree_view->GetNextChild(item, cookie);

        if (cookie && !tree_view->HasChildren(child))
        {
          populate_tree_view(tree_view, get_path_from_tree_item(tree_view, child, search_path), child);
        }
      } while (cookie != nullptr);
    }
  });

  tree_view->Bind(wxEVT_TREE_ITEM_ACTIVATED, [tree_view, &search_path, &add_element_from_file](wxTreeEvent& event) {
    auto item = event.GetItem();

    if (item == tree_view->GetRootItem())
    {
      return;
    }


    auto item_path = get_path_from_tree_item(tree_view, item, search_path);

    add_element_from_file(item_path, true);
  });

  wxPanel* panel = new wxPanel(notebook, wxID_ANY);
  panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
  create_render_view(panel, std::nullopt);
  notebook->AddPage(panel, "New Tab");

  panel = new wxPanel(notebook, wxID_ANY);
  panel->SetName("+");
  notebook->AddPage(panel, "+");

  notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED,
    [notebook, &add_new_element](wxAuiNotebookEvent& event) {
      auto* tab = notebook->GetPage(event.GetSelection());

      if (tab->GetName() == "+")
      {
        if (notebook->GetPageCount() == 1)
        {
          add_new_element();
        }
        else
        {
          notebook->ChangeSelection(event.GetSelection() - 1);
        }
      }
    });

  notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING,
    [notebook, &num_elements, &add_new_element](wxAuiNotebookEvent& event) mutable {
      auto* tab = notebook->GetPage(event.GetSelection());

      if (tab->GetName() == "+")
      {
        if (num_elements > notebook->GetPageCount())
        {
          num_elements = notebook->GetPageCount();
          return;
        }

        add_new_element();
      }
      else
      {
        event.Skip();
      }
    });


  frame->Bind(
    wxEVT_MENU, [&](auto& event) {
      const auto new_path = get_shape_path();

      if (new_path.has_value())
      {
        add_element_from_file(new_path.value(), true);

        search_path = new_path.value().parent_path();
        populate_tree_view(tree_view, search_path);
      }
    },
    wxID_OPEN);

  frame->Bind(
    wxEVT_MENU, [&](auto& event) {
      const auto new_path = get_shape_path();

      if (new_path.has_value())
      {
        add_element_from_file(new_path.value());
      }
    },
    event_open_in_new_tab);

  frame->Bind(
    wxEVT_MENU, [&](auto& event) {
      const auto new_path = get_workspace_path();

      if (new_path.has_value())
      {
        search_path = new_path.value();
        populate_tree_view(tree_view, search_path);
      }
    },
    event_open_folder_as_workspace);

  frame->Bind(
    wxEVT_MENU, [frame](auto& event) {
      frame->Close(true);
    },
    wxID_EXIT);

  sizer->SetSizeHints(frame);
  frame->SetSizer(sizer);

  frame->SetMenuBar(create_menu_bar());
  frame->CreateStatusBar();
  frame->SetStatusText("3Space Studio");
  frame->SetSize(800, 600);
  frame->Show(true);

  app->OnRun();

  ImGui::SFML::Shutdown();

  return 0;
}