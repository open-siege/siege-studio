#include <unordered_map>
#include <set>
#include <variant>
#include <optional>
#include <memory>
#include <functional>
#include <list>
#include <iterator>
#include <algorithm>
#include <utility>
#include <map>
#include <glm/gtx/quaternion.hpp>

#include <siege/content/wtb_renderable_shape.hpp>


namespace siege::content::wtb
{
  class wtb_renderable_shape final : public renderable_shape
  {
  public:
    wtb_renderable_shape(wtb_shape shape)
      : shape(std::move(shape))
    {
    }

    wtb_renderable_shape(bwd::bwd_model shape)
      : shape(std::move(shape))
    {
    }


    std::vector<sequence_info> get_sequences(const std::vector<std::size_t>& detail_level_indexes) const override
    {
      return {};
    }

    std::vector<std::string> get_detail_levels() const override
    {
      return { "Detail level 0" };
    }

    std::vector<material> get_materials() const override
    {
      return {};
    }

    void render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const override
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

  private:
    std::variant<wtb_shape, bwd::bwd_model> shape;
  };

  renderable_shape_value make_renderable_shape(wtb_shape shape)
  {
    return renderable_shape_value(wtb_renderable_shape(std::move(shape)));
  }

  renderable_shape_value make_renderable_shape(bwd::bwd_model shape)
  {
    return renderable_shape_value(wtb_renderable_shape(std::move(shape)));
  }
}// namespace siege::content::wtb
