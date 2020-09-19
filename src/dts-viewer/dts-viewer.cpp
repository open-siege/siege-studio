#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <functional>

//#include <toml11/toml.hpp>

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GLU.h>

#include "dts_io.hpp"
#include "dts_render.hpp"
#include "sfml_keys.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;
// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

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
      glColor3ub(red - num_faces, green - num_faces, std::uint8_t(current_object_name.size()));
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


int main(int, const char** argv)
{
  //const auto hello_toml = toml::parse("settings.toml");

  // Create the main window
  sf::RenderWindow window(sf::VideoMode(800, 600, 32), "SFML OpenGL");

  ImGui::SFML::Init(window);

  //sf::Color bgColor;

  std::array color = { 0.f, 0.f, 0.f };
  std::array<char, 255> windowTitle = { "Hello ImGui + SFML" };

  // Create a clock for measuring time elapsed
  sf::Clock Clock;

  //prepare OpenGL surface for HSR
  glClearDepth(1.f);
  glClearColor(0.3f, 0.3f, 0.3f, 0.f);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  //// Setup a perspective projection & Camera position
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90.f, 1.f, 1.f, 300.0f);//fov, aspect, zNear, zFar

  float x_angle = 180;
  float y_angle = 120;
  float z_angle = -30;

  dts::vector3f translation = { 0, 0, -15 };

  std::basic_ifstream<std::byte> input(argv[1], std::ios::binary);

  auto shape = dts::read_shape(argv[1], input);

  std::map<sf::Keyboard::Key, std::function<void()>> callbacks;
  std::map<std::string, bool> visible_nodes;

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

  // Start game loop
  while (window.isOpen())
  {
    // Process events
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

      // Close window : exit
      if (event.type == sf::Event::Closed)
      {
        window.close();
      }

      // Escape key : exit
      if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
      {
        window.close();
      }
    }

    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Apply some transformations for the cube
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(translation.x, translation.y, translation.z);

    glRotatef(y_angle, 1.f, 0.f, 0.f);
    glRotatef(x_angle, 0.f, 1.f, 0.f);
    glRotatef(z_angle, 0.f, 0.f, 1.f);

    std::visit(overloaded{
                 [&](const dts::shape_variant& core_shape) {
                   std::visit([&](const auto& main_shape) {
                     glBegin(GL_TRIANGLES);
                     auto renderer = gl_renderer{visible_nodes};

                     render_dts(main_shape, renderer);

                     glEnd();
                   },
                     core_shape);
                 },
                 [&](const dts::material_list_variant&) {
                   //Do nothing
                 } },
      shape);

    window.pushGLStates();

    ImGui::SFML::Update(window, Clock.restart());

    ImGui::Begin("Nodes");// begin window

    for (auto& [node_name, is_visible] : visible_nodes)
    {
      ImGui::Checkbox(node_name.c_str(), &is_visible);
    }

    ImGui::End();

    ImGui::SFML::Render(window);

    window.popGLStates();
    window.display();
  }

  ImGui::SFML::Shutdown();
  return EXIT_SUCCESS;
}