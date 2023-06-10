#include <iostream>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <execution>
#include <bitset>
#include <fstream>
#include "content/json_boost.hpp"
#include "content/dts/complex_serializer.hpp"
#include "shared.hpp"
#include "content/dts/darkstar.hpp"
#include "content/dts/3space.hpp"
//#include "content/dts/dts_json_formatting.hpp"

namespace fs = std::filesystem;
namespace dts2 = studio::content::dts::three_space;
namespace dts3 = studio::content::dts::darkstar;

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

int main(int argc, const char** argv)
{
  const auto files = studio::shared::find_files(
    std::vector<std::string>(argv + 1, argv + argc),
    ".dts",
    ".DTS",
    ".dml",
    ".DML");

  std::for_each(files.begin(), files.end(), [](auto&& file_name) {
    try
    {
      {
        std::stringstream msg;
        msg << "Converting " << file_name.string() << '\n';
        std::cout << msg.str();
      }

      std::ifstream input(file_name, std::ios::binary);

      if (dts3::is_darkstar_dts(input))
      {
        auto shape = dts3::read_shape(input);

        std::visit([&](const auto& item) {
          nlohmann::ordered_json item_as_json = item;

          //TODO make this have a flag
          // and reduce the amount of formatting to only what makes sense
          // and what is easy to parse again
          //format_json(item_as_json);

          auto new_file_name = file_name.string() + ".json";
          {
            std::ofstream item_as_file(new_file_name, std::ios::trunc);
            item_as_file << std::setw(4) << item_as_json;
          }

          std::stringstream msg;
          msg << "Created " << new_file_name << '\n';
          std::cout << msg.str();
        },
          shape);
      }
      else if (dts2::v1::is_3space_dts(input))
      {
        namespace dts = dts2::v1;
        auto shapes = dts2::v1::read_shapes(input);
        nlohmann::ordered_json item_as_json = shapes;

        auto new_file_name = file_name.string() + ".json";
        {
          std::ofstream item_as_file(new_file_name, std::ios::trunc);
          item_as_file << std::setw(4) << item_as_json;
        }

        //TODO figure out how these DTS file work
//        for (auto& shape : shapes)
//        {
//          if (std::holds_alternative<dts::shape>(shape))
//          {
//            auto real_shape = std::get<dts::shape>(shape);
//
//            std::vector<dts::group_item> groups = studio::shared::transform_variants<dts::group_item>(real_shape.base.parts);
//
//            for (auto& group : groups)
//            {
//              const auto visit_poly = [](dts::poly& real_poly, dts::group& real_group) {
//                std::vector<int> face;
//                face.reserve(real_poly.vertex_count);
//
//                for (auto i = real_poly.vertex_list; i < real_poly.vertex_list + real_poly.vertex_count; ++i)
//                {
//                  face.emplace_back(real_group.indexes[i]);
//                }
//
//                return face;
//              };
//
//              const auto visit_group = [](dts::group& real_group) {
//                std::vector<dts::poly_item> polys = studio::shared::transform_variants<dts::poly_item>(real_group.items);
//
//                std::vector<std::vector<int>> all_faces;
//
//                for (auto& poly : polys)
//                {
//                  all_faces.emplace_back(std::visit(overloaded{
//                               [&](dts::poly& arg) { return visit_poly(arg, real_group); },
//                               [&](dts::solid_poly& arg) { return visit_poly(arg.base, real_group); },
//                               [&](dts::gouraud_poly& arg) { return visit_poly(arg.base.base, real_group); },
//                               [&](dts::shaded_poly& arg) { return visit_poly(arg.base.base, real_group); },
//                               [&](dts::texture_for_poly& arg) { return visit_poly(arg.base.base, real_group); },
//                             },
//                             poly));
//                }
//
//                return all_faces;
//              };
//              std::visit(overloaded{
//                           [](dts::group& arg) { return visit_group(arg); },
//                           [](dts::bsp_group& arg) { return visit_group(arg); }
//                         },
//                group);
//            }
//          }
//        }

        std::stringstream msg;
        msg << file_name << " has "
            << " " << shapes.size() << " shapes\n";
        std::cout << msg.str();
      }
    }
    catch (const std::exception& ex)
    {
      std::stringstream msg;
      msg << file_name << " " << ex.what() << '\n';
      std::cerr << msg.str();
    }
  });

  return 0;
}