#ifndef DARKSTARDTSCONVERTER_RENDERABLE_SHAPE_HPP
#define DARKSTARDTSCONVERTER_RENDERABLE_SHAPE_HPP

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>
#include "3d_structures.hpp"

namespace siege::content
{
  struct shape_renderer
  {
    virtual void update_node(std::optional<std::string_view> parent_node_name, std::string_view node_name) = 0;
    virtual void update_object(std::optional<std::string_view> parent_node_name, std::string_view object_name) = 0;
    virtual void new_face(std::size_t num_vertices) = 0;
    virtual void end_face() = 0;
    virtual void emit_vertex(const vector3f& vertex) = 0;
    virtual void emit_texture_vertex(const texture_vertex& vertex) = 0;

    virtual ~shape_renderer() = default;

    shape_renderer() = default;
    shape_renderer(const shape_renderer&) = delete;
    shape_renderer(shape_renderer&&) = delete;
  };

  struct sub_sequence_info
  {
    std::int32_t node_index;
    std::string node_name;
    std::int32_t frame_index;
    std::int32_t first_key_frame_index;
    std::int32_t num_key_frames;
    float min_position;
    float max_position;
    float position;
    bool enabled;
  };

  struct sequence_info
  {
    std::int32_t index;
    std::string name;
    bool enabled;
    std::vector<sub_sequence_info> sub_sequences;
  };

  inline std::string to_string(const rgb_data& data)
  {
    return "rgba(" + std::to_string(data.red) + ", " + std::to_string(data.green) + ", " + std::to_string(data.blue) + ", " + std::to_string(data.rgb_flags) + ")";
  }

  struct material
  {
    using metadata_type = std::variant<std::int32_t, std::uint32_t, std::int16_t, std::uint16_t, float, rgb_data>;
    std::string filename;
    std::unordered_map<std::string_view, metadata_type> metadata;
    static std::string to_string(const metadata_type& data)
    {
      using std::to_string;

      return std::visit([](const auto& val) {
        return to_string(val);
      }, data);
    }
  };

  struct renderable_shape
  {
    virtual std::vector<sequence_info> get_sequences(const std::vector<std::size_t>& detail_level_indexes) const = 0;
    virtual std::vector<std::string> get_detail_levels() const = 0;
    virtual std::vector<material> get_materials() const = 0;

    virtual void render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const = 0;

    virtual ~renderable_shape() = default;
  };
}

#endif//DARKSTARDTSCONVERTER_RENDERABLE_SHAPE_HPP
