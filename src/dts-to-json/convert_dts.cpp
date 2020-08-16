#include <iostream>
#include <iterator>
#include <algorithm>
#include <execution>
#include <bitset>
#include "json_boost.hpp"
#include "complex_serializer.hpp"
#include "shared.hpp"
#include "dts_io.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;

int main(int argc, const char** argv)
{
  const auto files = dts::shared::find_files(
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

      std::basic_ifstream<std::byte> input(file_name, std::ios::binary);

      auto shape = dts::read_shape(file_name, input);

      std::visit([&](const auto& item) {
        nlohmann::ordered_json item_as_json = item;

        auto new_file_name = file_name.string() + ".json";
        {
          std::ofstream item_as_file(new_file_name, std::ios::trunc);
          item_as_file << item_as_json.dump(4);
        }

        std::stringstream msg;
        msg << "Created " << new_file_name << '\n';
        std::cout << msg.str();
      },
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