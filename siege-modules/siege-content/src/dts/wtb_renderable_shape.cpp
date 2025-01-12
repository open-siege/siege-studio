#include <unordered_map>
#include <set>
#include <variant>
#include <optional>
#include <memory>
#include <functional>
#include <list>
#include <glm/gtx/quaternion.hpp>

#include "wtb_renderable_shape.hpp"


namespace siege::content::wtb
{

  std::vector<sequence_info> wtb_renderable_shape::get_sequences(const std::vector<std::size_t>& detail_level_indexes) const
  {
    return {};
  }

  std::vector<std::string> wtb_renderable_shape::get_detail_levels() const
  {
    return { "Detail level 0" };
  }

  std::vector<material> wtb_renderable_shape::get_materials() const
  {
    return {};
  }

  void wtb_renderable_shape::render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const
  {

    auto render_shape = [&](const wtb_shape& wtb, std::optional<bwd::vec_3s> translation) {
      for (auto& face : wtb.faces)
      {
        vector3f result{};

        renderer.new_face(face.values.size());

        if (translation)
        {
          for (auto index : face.values)
          {
            result.x = (float)wtb.vertices[index].x + (float)translation->x;
            result.y = (float)wtb.vertices[index].y + (float)translation->x;
            result.z = (float)wtb.vertices[index].z + (float)translation->x;
            renderer.emit_vertex(result);
          }
        }
        else
        {
          for (auto index : face.values)
          {
            result.x = (float)wtb.vertices[index].x;
            result.y = (float)wtb.vertices[index].y;
            result.z = (float)wtb.vertices[index].z;
            renderer.emit_vertex(result);
          }
        }

        renderer.end_face();
      }
    };

    if (auto* wtb = std::get_if<wtb_shape>(&shape); wtb)
    {
      render_shape(*wtb, std::nullopt);
    }
    else if (auto* bwd = std::get_if<bwd::bwd_model>(&shape); bwd)
    {
      if (!bwd->lod_objects.empty())
      {
        auto& shapes = bwd->lod_objects.begin()->second;

        for (auto& shape : shapes)
        {
          if (shape.shape)
          {
            render_shape(*shape.shape, std::nullopt);
          }
        }
      }
    }
  }
}// namespace siege::content::wtb
