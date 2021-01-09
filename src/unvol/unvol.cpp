#include <fstream>
#include <algorithm>
#include <iostream>

#include <filesystem>
#include "resource/darkstar_volume.hpp"

int main(int, const char** argv)
{
  std::string output_folder(argv[1]);
  auto volume = std::basic_ifstream<std::byte>{ output_folder, std::ios::binary };

  auto files = darkstar::vol::get_file_metadata(volume);

  if (auto index = output_folder.find(".vol"); index != std::string::npos)
  {
    output_folder.replace(index, 4, "");
  }

  if (auto index = output_folder.find(".VOL"); index != std::string::npos)
  {
    output_folder.replace(index, 4, "");
  }

  std::cout << "Creating " << output_folder << " directory\n";
  std::filesystem::create_directory(output_folder);

  for (auto& some_file : files)
  {
    darkstar::vol::extract_files(volume, argv[1], output_folder, some_file);
  }
}