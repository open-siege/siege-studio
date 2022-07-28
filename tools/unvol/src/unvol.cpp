#include <fstream>
#include <array>
#include <memory>
#include <filesystem>
#include <functional>
#include <utility>
#include <iostream>
#include "content/mis/mission.hpp"
#include "resources/darkstar_volume.hpp"
#include "resources/three_space_volume.hpp"
#include "resources/trophy_bass_volume.hpp"

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

namespace dio
{
  namespace mis = studio::resources::mis;
  namespace vol = studio::resources::vol;
}


template <typename ArchiveType>
auto create_archive()
{
  return std::unique_ptr<studio::resources::archive_plugin>(new ArchiveType());
}

using CheckerType = decltype(&dio::vol::darkstar::vol_file_archive::is_supported);
using CreatorType = decltype(&create_archive<dio::vol::darkstar::vol_file_archive>);

constexpr static auto vol_checkers = std::array<std::pair<CheckerType, CreatorType>, 7> {{
    { dio::vol::darkstar::vol_file_archive::is_supported, create_archive<dio::vol::darkstar::vol_file_archive> },
    { dio::mis::darkstar::mis_file_archive::is_supported, create_archive<dio::mis::darkstar::mis_file_archive> },
    { dio::vol::three_space::vol_file_archive::is_supported, create_archive<dio::vol::three_space::vol_file_archive> },
    { dio::vol::three_space::rmf_file_archive::is_supported, create_archive<dio::vol::three_space::rmf_file_archive> },
    { dio::vol::three_space::dyn_file_archive::is_supported, create_archive<dio::vol::three_space::dyn_file_archive> },
    { dio::vol::trophy_bass::rbx_file_archive::is_supported, create_archive<dio::vol::trophy_bass::rbx_file_archive> },
    { dio::vol::trophy_bass::tbv_file_archive::is_supported, create_archive<dio::vol::trophy_bass::tbv_file_archive> }
}};

int main(int, const char** argv)
{
  std::string volume_file(argv[1]);
  auto volume_stream = std::basic_ifstream<std::byte>{ volume_file, std::ios::binary };

  std::unique_ptr<studio::resources::archive_plugin> archive;

  for (const auto [checker, creator] : vol_checkers)
  {
    if (checker(volume_stream))
    {
      archive = creator();
      break;
    }
  }

  if (!archive)
  {
    std::cerr << "Could not extract " << volume_file << '\n';
    return EXIT_FAILURE;
  }

  auto files = archive->get_content_listing(volume_stream, { volume_file, volume_file });

  std::string output_folder = replace_extension(volume_file);

  studio::resources::batch_storage storage;

  std::function<void(decltype(files)&)> extract_files = [&](const auto& files){
    for (const auto& some_file : files)
    {
      std::visit([&](const auto& info) {
        using info_type = std::decay_t<decltype(info)>;

        if constexpr (std::is_same_v<info_type, studio::resources::file_info>)
        {
          auto final_folder = output_folder / std::filesystem::relative(replace_extension(info.folder_path.string()), output_folder);
          std::filesystem::create_directories(final_folder);
          auto filename = final_folder / info.filename;
          auto new_stream = std::basic_ofstream<std::byte>{ filename, std::ios::binary };
          archive->extract_file_contents(volume_stream, info, new_stream, storage);
        }

        if constexpr (std::is_same_v<info_type, studio::resources::folder_info>)
        {
          // TODO think about whether get_content_listing should preserve the original position of the stream or not.
          std::basic_ifstream<std::byte> temp_stream{ volume_file, std::ios::binary };
          auto files = archive->get_content_listing(temp_stream, { volume_file, info.full_path });
          extract_files(files);
        }
      }, some_file);
    }
  };

  extract_files(files);
}
