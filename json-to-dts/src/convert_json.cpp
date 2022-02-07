#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <execution>
#include "content/dts/darkstar.hpp"
#include "content/json_boost.hpp"
#include "content/dts/complex_serializer.hpp"
#include "shared.hpp"

namespace fs = std::filesystem;
namespace dts = studio::content::dts::darkstar;

bool replace(std::string& str, const std::string& from, const std::string& to)
{
  size_t start_pos = str.rfind(from);
  if (start_pos == std::string::npos)
  {
    return false;
  }
  str.replace(start_pos, from.length(), to);
  return true;
}

int main(int argc, const char** argv)
{
  const auto files = studio::shared::find_files(
    std::vector<std::string>(argv + 1, argv + argc),
    ".dts.json",
    ".DTS.json",
    ".dml.json",
    ".DML.json");

  std::for_each(std::execution::par_unseq, files.begin(), files.end(), [](auto&& file_name) {
    try
    {
      {
        std::stringstream msg;
        msg << "Converting " << file_name.string() << '\n';
        std::cout << msg.str();
      }

      std::ifstream test_file(file_name);
      auto fresh_shape_json = nlohmann::json::parse(test_file);
      const auto json_type_name = fresh_shape_json.at("typeName").get<std::string>();

      std::string new_file_name = file_name.string();
      replace(new_file_name, ".json", "");

      if (fs::is_regular_file(new_file_name) && !fs::is_regular_file(new_file_name + ".old"))
      {
        fs::rename(new_file_name, new_file_name + ".old");
      }

      if (json_type_name == dts::material_list::v2::material_list::type_name)
      {
        const dts::material_list_variant fresh_shape = fresh_shape_json;

        std::basic_ofstream<std::byte> stream(new_file_name, std::ios::binary);
        dts::write_material_list(stream, fresh_shape);


        std::stringstream msg;
        msg << "Created " << new_file_name << '\n';
        std::cout << msg.str();
      }
      else if (json_type_name == dts::shape::v2::shape::type_name)
      {
        const dts::shape_variant fresh_shape = fresh_shape_json;

        std::basic_ofstream<std::byte> stream(new_file_name, std::ios::binary);
        dts::write_shape(stream, fresh_shape);
        std::stringstream msg;
        msg << "Created " << new_file_name << '\n';
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