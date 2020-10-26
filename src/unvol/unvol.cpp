#include "archives/darkstar_volume.hpp"
#include <fstream>
#include <iostream>

int main(int, const char** argv)
{
  auto file = std::basic_ifstream<std::byte>{argv[1], std::ios::binary};

  auto files = darkstar::get_file_metadata(file);

  for (auto& some_file : files)
  {
    std::cout << some_file.filename << " " << some_file.offset << " " << some_file.size << '\n';
  }
}