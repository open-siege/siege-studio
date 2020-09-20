#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>

#include <wx/wx.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "dts_io.hpp"
#include "dts_render.hpp"
#include "sfml_keys.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;

struct gl_renderer final : shape_renderer
{
  const std::array<std::uint8_t, 3> max_colour = { 255, 255, 0 };
  std::string_view current_object_name;
  std::uint8_t num_faces = 0;
  std::map<std::string, bool>& visible_nodes;
  std::map<std::string, bool>::iterator current_node;

  gl_renderer(std::map<std::string, bool>& visible_nodes) : visible_nodes(visible_nodes)
  {
    current_node = visible_nodes.end();
  }

  void update_node(std::string_view node_name) override
  {
    current_node = visible_nodes.emplace(node_name, true).first;
  }

  void update_object(std::string_view object_name) override
  {
    num_faces = 0;
    current_object_name = object_name;
  }

  void new_face(std::size_t) override
  {
    if (current_node->second)
    {
      const auto [red, green, blue] = max_colour;
      glColor4ub(red - num_faces, green - num_faces, std::uint8_t(current_object_name.size()), 255);
      num_faces += 255 / 15;
    }
  }

  void end_face() override
  {
  }

  void emit_vertex(const darkstar::dts::vector3f& vertex) override
  {
    if (current_node->second)
    {
      glVertex3f(vertex.x, vertex.y, vertex.z);
    }
  }

  void emit_texture_vertex(const darkstar::dts::mesh::v1::texture_vertex&) override
  {
  }
};


void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
  constexpr GLdouble pi = 3.1415926535897932384626433832795;
  GLdouble fW, fH;

  //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
  fH = tan(fovY / 360 * pi) * zNear;
  fW = fH * aspect;

  glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}


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

int main(int argc, const char** argv)
{
  using namespace std::literals;
  std::optional<std::filesystem::path> shape_path;

  dts::shape_variant shape;

  if (argc > 1)
  {
    shape_path = argv[1];
  }

  if (!shape_path.has_value())
  {
    shape_path = get_shape_path();
  }

  if (!shape_path.has_value())
  {
    wxMessageBox("No file has been selected.", "Cannot Continue.", wxICON_ERROR);
    return EXIT_FAILURE;
  }

  try
  {
    std::basic_ifstream<std::byte> input(shape_path.value(), std::ios::binary);
    shape = dts::read_shape(shape_path.value(), input, std::nullopt);
  }
  catch (const std::exception& ex)
  {
    wxMessageBox(ex.what(), "Error Loading Model.", wxICON_ERROR);
    return EXIT_FAILURE;
  }

  sf::ContextSettings context;
  context.depthBits = 24;
  constexpr auto main_title = "3Space Studio - Darkstar DTS Viewer - ";

  sf::RenderWindow window(sf::VideoMode(800, 600, 32), main_title + shape_path.value().filename().string(), sf::Style::Default, context);

  ImGui::SFML::Init(window);

  std::array color = { 0.f, 0.f, 0.f };
  sf::Clock clock;

  glClearDepth(1.f);
  glClearColor(0.3f, 0.3f, 0.3f, 0.f);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  perspectiveGL(90.f, 1.f, 1.f, 300.0f);


  float x_angle = 180;
  float y_angle = 120;
  float z_angle = -30;

  dts::vector3f translation = { 0, 0, -15 };

  std::map<sf::Keyboard::Key, std::function<void()>> callbacks;
  std::map<std::string, bool> visible_nodes;
  auto detail_levels = get_detail_levels(shape);
  int detail_level_index = 0;

  callbacks.emplace(config::get_key_for_name("Left"), [&]() { x_angle--; });
  callbacks.emplace(config::get_key_for_name("Numpad4"), [&]() { x_angle--; });
  callbacks.emplace(config::get_key_for_name("Right"), [&]() { x_angle++; });
  callbacks.emplace(config::get_key_for_name("Numpad6"), [&]() { x_angle++; });
  callbacks.emplace(config::get_key_for_name("Up"), [&]() { y_angle++; });
  callbacks.emplace(config::get_key_for_name("Numpad8"), [&]() { y_angle++; });
  callbacks.emplace(config::get_key_for_name("Down"), [&]() { y_angle--; });
  callbacks.emplace(config::get_key_for_name("Numpad2"), [&]() { y_angle--; });
  callbacks.emplace(config::get_key_for_name("Home"), [&]() { z_angle++; });
  callbacks.emplace(config::get_key_for_name("Numpad7"), [&]() { z_angle++; });
  callbacks.emplace(config::get_key_for_name("PageUp"), [&]() { z_angle--; });
  callbacks.emplace(config::get_key_for_name("Numpad9"), [&]() { z_angle--; });
  callbacks.emplace(config::get_key_for_name("End"), [&]() { translation.x++; });
  callbacks.emplace(config::get_key_for_name("Numpad1"), [&]() { translation.x++; });
  callbacks.emplace(config::get_key_for_name("PageDown"), [&]() { translation.x--; });
  callbacks.emplace(config::get_key_for_name("Numpad3"), [&]() { translation.x--; });

  callbacks.emplace(config::get_key_for_name("Add"), [&]() { translation.z++; });
  callbacks.emplace(config::get_key_for_name("Subtract"), [&]() { translation.z--; });

  callbacks.emplace(config::get_key_for_name("Divide"), [&]() { translation.y++; });
  callbacks.emplace(config::get_key_for_name("Insert"), [&]() { translation.y++; });
  callbacks.emplace(config::get_key_for_name("Numpad0"), [&]() { translation.y++; });

  callbacks.emplace(config::get_key_for_name("Multiply"), [&]() { translation.y--; });
  callbacks.emplace(config::get_key_for_name("Delete"), [&]() { translation.y--; });
  callbacks.emplace(config::get_key_for_name("Period"), [&]() { translation.y--; });

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      ImGui::SFML::ProcessEvent(event);
      if (event.type == sf::Event::KeyPressed && (event.key.code != sf::Keyboard::Escape))
      {
        const auto callback = callbacks.find(event.key.code);

        if (callback != callbacks.end())
        {
          callback->second();
        }
      }

      if (event.type == sf::Event::Closed)
      {
        window.close();
      }

      if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
      {
        window.close();
      }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(translation.x, translation.y, translation.z);

    glRotatef(y_angle, 1.f, 0.f, 0.f);
    glRotatef(x_angle, 0.f, 1.f, 0.f);
    glRotatef(z_angle, 0.f, 0.f, 1.f);

    glBegin(GL_TRIANGLES);
    auto renderer = gl_renderer{ visible_nodes };
    render_dts(shape, renderer, detail_level_index);
    glEnd();

    window.pushGLStates();

    ImGui::SFML::Update(window, clock.restart());

    if (ImGui::Begin("Options"))
    {
      if (ImGui::Button("Open"))
      {
        const auto path = get_shape_path();

        if (path.has_value())
        {
          try
          {
            std::basic_ifstream<std::byte> input(path.value(), std::ios::binary);
            shape = dts::read_shape(path.value(), input, std::nullopt);
            window.setTitle(main_title + path.value().filename().string());
            visible_nodes.clear();
          }
          catch (const std::exception& ex)
          {
            wxMessageBox(ex.what(), "Error Loading Model.", wxICON_ERROR);
          }
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("Exit"))
      {
        window.close();
        break;
      }

      ImGui::End();
    }

    ImGui::Begin("Nodes");

    for (auto& [node_name, is_visible] : visible_nodes)
    {
      ImGui::Checkbox(node_name.c_str(), &is_visible);
    }

    ImGui::End();

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
    }

    ImGui::End();


    ImGui::SFML::Render(window);

    window.popGLStates();
    window.display();
  }

  ImGui::SFML::Shutdown();
  return EXIT_SUCCESS;
}