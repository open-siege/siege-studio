#include <fstream>
#include <algorithm>

#include <filesystem>
#include "resources/darkstar_volume.hpp"

auto replace_extension(std::string output_folder)
{
  if (auto index = output_folder.find(".vol"); index != std::string::npos)
  {
    output_folder.replace(index, 4, "");
  }

  if (auto index = output_folder.find(".VOL"); index != std::string::npos)
  {
    output_folder.replace(index, 4, "");
  }

  return output_folder;
}

int main(int, const char** argv)
{
  std::string output_folder(argv[1]);
  auto volume = std::basic_ifstream<std::byte>{ output_folder, std::ios::binary };

  auto archive = studio::resources::vol::darkstar::vol_file_archive();
  auto files = archive.get_content_listing(volume, output_folder);

  output_folder = replace_extension(output_folder);

  for (auto& some_file : files)
  {
    std::visit([&](auto& info) {
           using info_type = std::decay_t<decltype(info)>;

           if constexpr (std::is_same_v<info_type, studio::resources::file_info>)
           {
             auto final_folder = output_folder / std::filesystem::relative(replace_extension(info.folder_path.string()), output_folder);
             std::filesystem::create_directories(final_folder);
             auto filename = final_folder / info.filename;
             auto new_stream = std::basic_ofstream<std::byte>{ filename, std::ios::binary };
             archive.extract_file_contents(volume, info, new_stream);
           }
    }, some_file);
  }
}
