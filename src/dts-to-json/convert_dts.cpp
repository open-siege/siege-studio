#include <iostream>
#include <iterator>
#include <algorithm>
#include <execution>
#include <bitset>
#include "content/json_boost.hpp"
#include "complex_serializer.hpp"
#include "shared.hpp"
#include "dts_io.hpp"
#include "dts_json_formatting.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;

int main(int argc, const char** argv)
{
  const auto files = shared::find_files(
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

        //TODO make this have a flag
        // and reduce the amount of formatting to only what makes sense
        // and what is easy to parse again
        //format_json(item_as_json);

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