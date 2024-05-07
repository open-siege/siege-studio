#include <iostream>
#include <iterator>
#include <execution>
#include <algorithm>
#include <bitset>
#include <utility>
#include <unordered_map>
#include <siege/content/json_boost.hpp>
#include <siege/content/dts/complex_serializer.hpp>
#include <siege/platform/shared.hpp>
#include <siege/content/dts/darkstar.hpp>
#include <siege/content/dts/dts_renderable_shape.hpp>
#include <siege/content/obj_renderer.hpp>

namespace fs = std::filesystem;
namespace dts = siege::content::dts::darkstar;

// helper type for the visitor #4
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
  const auto files = siege::platform::find_files(
    std::vector<std::string>(argv + 1, argv + argc),
    ".dts",
    ".DTS",
    ".dml",
    ".DML");

  std::for_each(std::execution::par_unseq, files.begin(), files.end(), [](auto&& file_name) {
    try
    {
      {
        std::stringstream msg;
        msg << "Converting " << file_name.string() << '\n';
        std::cout << msg.str();
      }

      std::ifstream input(file_name, std::ios::binary);

      auto shape = dts::read_shape(input);

      std::visit(overloaded{
                   [&](const dts::shape_variant& core_shape)
                   {
                     std::visit([&](const auto& main_shape) {
                       for (auto i = 0u; i < main_shape.details.size(); ++i)
                       {
                         const auto& detail_level = main_shape.details[i];
                         const auto root_node = main_shape.nodes[detail_level.root_node_index];
                         const std::string root_node_name = main_shape.names[root_node.name_index].data();
                         std::ofstream output(file_name.string() + "." + root_node_name + ".obj", std::ios::trunc);
                         auto renderer = siege::content::obj_renderer{output};

                         siege::content::dts::darkstar::dts_renderable_shape instance{core_shape};
                         std::vector<std::size_t> details{i};
                         auto sequences = instance.get_sequences(details);
                         instance.render_shape(renderer, details, sequences);
                       }
                     },
                       core_shape);
                   },
                   [&](const dts::material_list_variant&) {
                     //TODO generate a MTL file
                   } },
        shape);
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