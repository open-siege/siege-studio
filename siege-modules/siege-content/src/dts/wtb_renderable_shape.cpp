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

    for (auto& face : shape.faces)
    {
      vector3f result{};

      renderer.new_face(face.values.size());

      for (auto iter = face.values.begin(); iter != face.values.end(); ++iter)
      {
        auto index = *iter;
        result.x = (float)shape.vertices[index].x;
        result.y = (float)shape.vertices[index].y;
        result.z = (float)shape.vertices[index].z;
        //result.x = (shape.scale * (float)shape.vertices[index].x) + shape.translation;
        //result.y = (shape.scale * (float)shape.vertices[index].y) + shape.translation;
        //result.z = (shape.scale * (float)shape.vertices[index].z) + shape.translation;
        renderer.emit_vertex(result);
      }

      renderer.end_face();
    }
  }
}// namespace siege::content::wtb
