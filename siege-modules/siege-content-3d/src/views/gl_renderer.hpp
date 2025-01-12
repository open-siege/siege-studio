#ifndef GL_RENDERER_HPP
#define GL_RENDERER_HPP

#include <map>
#include <GL/GL.h>
#include <siege/content/renderable_shape.hpp>

namespace siege::views
{

  struct gl_renderer final : content::shape_renderer
  {
    const std::array<std::uint8_t, 3> max_colour = { 255, 255, 0 };
    std::string_view current_object_name;
    std::uint8_t num_faces = 0;
    std::map<std::optional<std::string>, std::map<std::string, bool>>& visible_nodes;
    std::map<std::string, std::map<std::string, bool>>& visible_objects;
    bool current_object_visible = true;
    bool current_node_visible = true;

    static std::optional<std::string> to_string(std::optional<std::string_view> value)
    {
      if (value.has_value())
      {
        return std::string{ value.value() };
      }

      return std::nullopt;
    }


    gl_renderer(std::map<std::optional<std::string>, std::map<std::string, bool>>& visible_nodes,
      std::map<std::string, std::map<std::string, bool>>& visible_objects) : visible_nodes(visible_nodes), visible_objects(visible_objects)
    {
    }

    void update_node(std::optional<std::string_view> parent_node, std::string_view node_name) override
    {
      auto [iterator, added] = visible_nodes.emplace(to_string(parent_node), std::map<std::string, bool>{});

      auto [new_node_iterator, object_added] = iterator->second.emplace(node_name, true);

      current_node_visible = new_node_iterator->second;
    }

    void update_object(std::optional<std::string_view> parent_node, std::string_view object_name) override
    {
      num_faces = 0;
      current_object_name = object_name;

      if (parent_node.has_value())
      {
        auto [iterator, added] = visible_objects.emplace(parent_node.value(), std::map<std::string, bool>{});

        auto [new_object_iterator, object_added] = iterator->second.emplace(object_name, true);

        current_object_visible = current_node_visible && new_object_iterator->second;
      }
    }

    void new_face(std::size_t size) override
    {
      if (size == 3)
      {
        glBegin(GL_TRIANGLES);
      }
      else
      {
        glBegin(GL_TRIANGLE_FAN);
      }

      if (current_object_visible)
      {
        const auto [red, green, blue] = max_colour;
        glColor4ub(red - num_faces, green - num_faces, std::uint8_t(current_object_name.size()), 255);
        num_faces += 255 / 15;
      }
    }

    void end_face() override
    {
      glEnd();
    }

    void emit_vertex(const content::vector3f& vertex) override
    {
      if (current_object_visible)
      {
        glVertex3f(vertex.x, vertex.y, vertex.z);
      }
    }

    void emit_texture_vertex(const content::texture_vertex&) override
    {
    }
  };

  inline void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
  {
    constexpr GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;

    // fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    fH = tan(fovY / 360 * pi) * zNear;
    fW = fH * aspect;

    ::glFrustum(-fW, fW, -fH, fH, zNear, zFar);
  }
}// namespace siege::views

#endif// DARKSTARDTSCONVERTER_GL_RENDERER_HPP
