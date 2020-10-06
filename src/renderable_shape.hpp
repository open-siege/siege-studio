#ifndef DARKSTARDTSCONVERTER_RENDERABLE_SHAPE_HPP
#define DARKSTARDTSCONVERTER_RENDERABLE_SHAPE_HPP

#include <string>
#include <vector>
#include <optional>
#include "dts_structures.hpp"

struct shape_renderer
{
  virtual void update_node(std::optional<std::string_view> parent_node_name, std::string_view node_name) = 0;
  virtual void update_object(std::optional<std::string_view> parent_node_name, std::string_view object_name) = 0;
  virtual void new_face(std::size_t num_vertices) = 0;
  virtual void end_face() = 0;
  virtual void emit_vertex(const darkstar::dts::vector3f& vertex) = 0;
  virtual void emit_texture_vertex(const darkstar::dts::mesh::v1::texture_vertex& vertex) = 0;

  virtual ~shape_renderer() = default;

  shape_renderer() = default;
  shape_renderer(const shape_renderer&) = delete;
  shape_renderer(shape_renderer&&) = delete;
};

struct sequence_info
{
  std::string name;
  float position;
};

struct renderable_shape
{
  virtual std::vector<std::string> get_detail_levels() const = 0;

  virtual void render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes/*, const std::vector<sequence_info>& sequences*/) const = 0;

  virtual ~renderable_shape() = default;
};



#endif//DARKSTARDTSCONVERTER_RENDERABLE_SHAPE_HPP
