#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <filesystem>
#include "archives/darkstar_volume.hpp"

int main(int, const char** argv)
{
  std::string filename(argv[1]);
  auto file = std::basic_ifstream<std::byte>{ filename, std::ios::binary };

  auto files = darkstar::vol::get_file_metadata(file);

  if (auto index = filename.find(".vol"); index != std::string::npos)
  {
    filename.replace(index, 4, "");
  }

  if (auto index = filename.find(".VOL"); index != std::string::npos)
  {
    filename.replace(index, 4, "");
  }

  std::cout << "Creating " << filename << " directory\n";
  std::filesystem::create_directory(filename);


  for (auto& some_file : files)
  {
    auto new_path = std::filesystem::path(filename) / some_file.filename;

    if (some_file.compression_type == darkstar::vol::compression_type::none)
    {
      std::cout << "Extracting " << some_file.filename << " to " << new_path << '\n';
      auto new_file = std::basic_ofstream<std::byte> { new_path, std::ios::binary };

      file.seekg(some_file.offset + sizeof(darkstar::vol::volume_header), std::ios::beg);
      std::copy_n(std::istreambuf_iterator<std::byte>(file),
                  some_file.size,
                  std::ostreambuf_iterator<std::byte>(new_file));
    }
    else
    {
      std::stringstream command;

      command << "extract.exe" << ' ' << argv[1] << ' ' << some_file.filename << ' ' << new_path;

      std::cout << "Executing " << command.str() << '\n';
      std::system(command.str().c_str());

      if (std::filesystem::file_size(new_path) > some_file.size)
      {
        auto old_path = new_path.string() + ".old";
        std::filesystem::rename(new_path, old_path);

        {
          auto old_file = std::basic_ifstream<std::byte>{ old_path, std::ios::binary };

          auto new_file = std::basic_ofstream<std::byte>{ new_path, std::ios::binary };

          std::copy_n(std::istreambuf_iterator<std::byte>(old_file),
                      some_file.size,
                      std::ostreambuf_iterator<std::byte>(new_file));
        }

        std::filesystem::remove(old_path);
      }

    }
  }
}