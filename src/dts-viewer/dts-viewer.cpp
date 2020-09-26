#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>

#include <wx/wx.h>
#include <wx/aui/aui.h>

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "dts_io.hpp"
#include "dts_render.hpp"
#include "gl_renderer.hpp"
#include "sfml_keys.hpp"
#include "utility.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;

struct shape_instance
{
  dts::shape_variant shape;

  dts::vector3f translation;
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

void setup_opengl()
{
  glClearDepth(1.f);
  glClearColor(0.3f, 0.3f, 0.3f, 0.f);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  perspectiveGL(90.f, 1.f, 1.f, 300.0f);
}

auto apply_configuration(std::map<std::string, std::function<void(shape_instance&)>>& actions)
{
  //TODO read this from config one day
  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(shape_instance&)>>> callbacks;
  callbacks.emplace(config::get_key_for_name("Left"), std::ref(actions.at("pan_left")));
  callbacks.emplace(config::get_key_for_name("Numpad4"), std::ref(actions.at("pan_left")));
  callbacks.emplace(config::get_key_for_name("Right"), std::ref(actions.at("pan_right")));
  callbacks.emplace(config::get_key_for_name("Numpad6"), std::ref(actions.at("pan_right")));
  callbacks.emplace(config::get_key_for_name("Up"), std::ref(actions.at("pan_up")));
  callbacks.emplace(config::get_key_for_name("Numpad8"), std::ref(actions.at("pan_up")));
  callbacks.emplace(config::get_key_for_name("Down"), std::ref(actions.at("pan_down")));
  callbacks.emplace(config::get_key_for_name("Numpad2"), std::ref(actions.at("pan_down")));
  callbacks.emplace(config::get_key_for_name("Home"), std::ref(actions.at("increase_x_rotation")));
  callbacks.emplace(config::get_key_for_name("Numpad7"), std::ref(actions.at("increase_x_rotation")));
  callbacks.emplace(config::get_key_for_name("PageUp"), std::ref(actions.at("decrease_x_rotation")));
  callbacks.emplace(config::get_key_for_name("Numpad9"), std::ref(actions.at("decrease_x_rotation")));
  callbacks.emplace(config::get_key_for_name("End"), std::ref(actions.at("increase_z_rotation")));
  callbacks.emplace(config::get_key_for_name("Numpad1"), std::ref(actions.at("increase_z_rotation")));
  callbacks.emplace(config::get_key_for_name("PageDown"), std::ref(actions.at("decrease_z_rotation")));
  callbacks.emplace(config::get_key_for_name("Numpad3"), std::ref(actions.at("decrease_z_rotation")));

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

  auto instance_iterator = shape_instances.emplace(shape_path, shape_instance{ get_shape(shape_path), { 0, 0, -15 }, { 180, 120, -30 } });
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

  actions.emplace("pan_left", [](auto& instance) { instance.translation.x++; });
  actions.emplace("pan_right", [](auto& instance) { instance.translation.x--; });

  actions.emplace("pan_up", [](auto& instance) { instance.translation.y++; });
  actions.emplace("pan_down", [](auto& instance) { instance.translation.y--; });


  std::map<std::optional<std::string>, std::map<std::string, bool>> visible_nodes;
  std::map<std::string, std::map<std::string, bool>> visible_objects;
  auto detail_levels = get_detail_levels(instance->second.shape);
  int detail_level_index = 0;
  bool root_visible = true;

  auto callbacks = apply_configuration(actions);

  sf::Clock clock;

  setup_opengl();

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

    glRotatef(rotation.y, 1.f, 0.f, 0.f);
    glRotatef(rotation.x, 0.f, 1.f, 0.f);
    glRotatef(rotation.z, 0.f, 0.f, 1.f);

    glBegin(GL_TRIANGLES);
    auto renderer = gl_renderer{ visible_nodes, visible_objects };
    render_dts(shape, renderer, detail_level_index);
    glEnd();


    window->pushGLStates();

    ImGui::SFML::Update(*window, clock.restart());

    if (!detail_levels.empty())
    {
      ImGui::Begin("Nodes");

      render_tree_view(detail_levels[detail_level_index], root_visible, visible_nodes, visible_objects);

      ImGui::End();
    }

    ImGui::Begin("Detail Levels");

    if (ImGui::ListBox(
          "", &detail_level_index, [](void* data, int idx, const char** out_text) {
            *out_text = reinterpret_cast<std::string*>(data)[idx].c_str();
            return true;
          },
          detail_levels.data(),
          detail_levels.size()))
    {
      visible_nodes.clear();
      visible_objects.clear();
    }

    ImGui::End();


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

wxMenuBar* createMenuBar()
{
  auto* menuFile = new wxMenu();
  menuFile->Append(wxID_OPEN);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);


  auto* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  auto* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");

  return menuBar;
}

template<typename Callback>
wxButton* bindEvent(wxButton* button, Callback&& callback)
{
  button->Bind(wxEVT_BUTTON, std::forward<Callback>(callback));
  return button;
}


void create_render_view(wxWindow* panel, std::optional<std::filesystem::path> path)
{
  auto* graphics = new wxControl(panel, -1, wxDefaultPosition, wxSize{ 800, 500 }, 0);

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

  graphics->Bind(wxEVT_IDLE, [=](auto& event) {
    graphics->Refresh();
  });
  graphics->Bind(wxEVT_PAINT, renderer_main(path, window, graphics, gui_context));

  graphics->Bind(wxEVT_DESTROY, [=](auto& event) {
    delete window;
    if (gui_context != primary_gui_context)
    {
      ImGui::DestroyContext(gui_context);
    }
  });
}

int main(int argc, char** argv)
{
  wxApp::SetInitializerFunction(createApp);
  wxEntryStart(argc, argv);
  auto* app = wxApp::GetInstance();
  app->CallOnInit();

  auto* frame = new wxFrame(nullptr, wxID_ANY, "3Space Studio");

  frame->SetMenuBar(createMenuBar());

  frame->CreateStatusBar();
  frame->SetStatusText("Welcome to C++ Durban!");

  frame->Bind(
    wxEVT_MENU, [](auto& event) {
      wxMessageBox("This is a wxWidgets Hello World example",
        "About Hello wxWidgets",
        wxOK | wxICON_INFORMATION);
    },
    wxID_ABOUT);

  wxAuiNotebook* auiNotebook = new wxAuiNotebook(frame, wxID_ANY);

  wxPanel* panel = new wxPanel(auiNotebook, wxID_ANY);
  auto path = get_shape_path(argc, argv);
  create_render_view(panel, path);
  auiNotebook->AddPage(panel, path.has_value() ? path.value().filename().string() :"Tab 1");


  frame->Bind(
    wxEVT_MENU, [&](auto& event) {
      const auto new_path = get_shape_path();

      if (new_path.has_value())
      {
        wxPanel* panel = new wxPanel(auiNotebook, wxID_ANY);
        create_render_view(panel, new_path);
        auiNotebook->AddPage(panel, new_path.value().filename().string());
        auiNotebook->ChangeSelection(auiNotebook->GetPageCount() - 1);
      }
    },
    wxID_OPEN);

  frame->Bind(
    wxEVT_MENU, [frame](auto& event) {
      frame->Close(true);
    },
    wxID_EXIT);

  frame->SetSize(800, 600);
  frame->Show(true);

  app->OnRun();

  ImGui::SFML::Shutdown();

  return 0;
}