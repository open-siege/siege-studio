#ifndef DARKSTARDTSCONVERTER_OBJ_RENDERER_HPP
#define DARKSTARDTSCONVERTER_OBJ_RENDERER_HPP

#include <string_view>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "dts_renderable_shape.hpp"

struct obj_renderer final : shape_renderer
{
  std::optional<std::string> new_face_str = std::nullopt;
  std::size_t face_count = 0;
  std::ofstream& output;

  obj_renderer(std::ofstream& output)
    : output(output)
  {
    output << std::setprecision(32);
  }

  void update_node(std::string_view) override
  {
  }

  void update_object(std::string_view object_name) override
  {
    output << "o " << object_name << '\n';
  }

  void new_face(std::size_t num_vertices) override
  {
    std::stringstream stream;
    stream << "\tf";

    for (auto i = 1u; i <= num_vertices; ++i)
    {
      stream << " " << face_count + i << "/" << face_count + i;
    }

    stream << '\n';

    new_face_str = stream.str();

    face_count += num_vertices;
  }

  void end_face() override
  {
    if (new_face_str.has_value())
    {
      output << new_face_str.value();
    }
  }

  void emit_vertex(const darkstar::dts::vector3f& vertex) override
  {
    output << "\tv " << vertex.x << ' ' << vertex.y << ' ' << vertex.z << '\n';
  }

  void emit_texture_vertex(const darkstar::dts::mesh::v1::texture_vertex& vertex) override
  {
    output << "\tvt " << vertex.x << ' ' << vertex.y << '\n';
  }
};

#endif//DARKSTARDTSCONVERTER_OBJ_RENDERER_HPP
