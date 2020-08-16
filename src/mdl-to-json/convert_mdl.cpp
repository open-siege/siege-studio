#include <iostream>
#include <iterator>
#include <algorithm>
#include <execution>
#include <bitset>
#include "json_boost.hpp"
#include "shared.hpp"
#include "binary_io.hpp"
#include "mdl_structures.hpp"

namespace fs = std::filesystem;
namespace mdl = idtech2::mdl;
namespace io = binary::io;

int main(int argc, const char** argv)
{
  const auto files = shared::find_files(
    std::vector<std::string>(argv + 1, argv + argc),
    ".mdl",
    ".MDL");

  std::for_each(std::execution::par_unseq, files.begin(), files.end(), [](auto&& file_name) {
    try
    {
      {
        std::stringstream msg;
        msg << "Converting " << file_name.string() << '\n';
        std::cout << msg.str();
      }

      std::basic_ifstream<std::byte> input(file_name, std::ios::binary);

      auto header = io::read<mdl::file_header>(input);

      if (header.file_tag != mdl::mdl_tag)
      {
        throw std::invalid_argument("The file presented does not appear to be a valid MDL file.");
      }

      if (header.version != 6)
      {
        throw std::invalid_argument("Only version 6 MDL files are supported.");
      }

      auto data_header = io::read<mdl::data_header>(input);

      for (auto i = 0; i < data_header.num_skins; ++i)
      {
        auto skin = io::read<mdl::skin>(input);

        if (skin.group != 0 && skin.group != 1)
        {
          throw std::invalid_argument("Skin not parsed correctly.");
        }

        auto raw_data = io::read_vector<std::byte>(input,
          static_cast<std::size_t>(data_header.skin_width * data_header.skin_height));

        std::cout << "Read " << raw_data.size() << " bytes for texture\n";
      }

      auto texture_verts = io::read_vector<mdl::texture_vertex>(input, data_header.num_vertices);

      auto faces = io::read_vector<mdl::face>(input, data_header.num_triangles);

      std::vector<std::variant<mdl::simple_frame, mdl::group_frame>> frames;
      frames.reserve(data_header.num_frames);

      for (auto i = 0; i < data_header.num_frames; ++i)
      {
        int frame_type = io::read<mdl::frame_type>(input);

        if (frame_type == mdl::simple_frame::frame_type)
        {
          frames.emplace_back(io::read<mdl::simple_frame>(input));
        }
        else
        {
          frames.emplace_back(io::read<mdl::group_frame>(input));
        }
      }

      std::cout << "File has read " << input.tellg() << " out of "
                << std::filesystem::file_size(file_name) << " bytes\n";
      std::cout << "File has " << frames.size() << " frames\n";
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