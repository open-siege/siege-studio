#include <fstream>
#include <array>
#include <memory>
#include <filesystem>
#include <functional>
#include <utility>
#include <iostream>
#include <siege/resource/darkstar_resource.hpp>
#include <siege/resource/three_space_resource.hpp>
#include <siege/resource/trophy_bass_resource.hpp>

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
  namespace vol = siege::resource::vol;
}


template <typename ArchiveType>
siege::platform::resource_reader create_archive()
{
  return ArchiveType();
}

using CheckerType = decltype(&dio::vol::darkstar::vol_resource_reader::stream_is_supported);
using CreatorType = decltype(&create_archive<dio::vol::darkstar::vol_resource_reader>);

constexpr static auto vol_checkers = std::array<std::pair<CheckerType, CreatorType>, 7> {{
    { dio::vol::darkstar::vol_resource_reader::stream_is_supported, create_archive<dio::vol::darkstar::vol_resource_reader> },
    { dio::vol::three_space::vol_resource_reader::stream_is_supported, create_archive<dio::vol::three_space::vol_resource_reader> },
    { dio::vol::three_space::rmf_resource_reader::stream_is_supported, create_archive<dio::vol::three_space::rmf_resource_reader> },
    { dio::vol::three_space::dyn_resource_reader::stream_is_supported, create_archive<dio::vol::three_space::dyn_resource_reader> },
    { dio::vol::trophy_bass::rbx_resource_reader::stream_is_supported, create_archive<dio::vol::trophy_bass::rbx_resource_reader> },
    { dio::vol::trophy_bass::tbv_resource_reader::stream_is_supported, create_archive<dio::vol::trophy_bass::tbv_resource_reader> }
}};

int main(int, const char** argv)
{
  std::string volume_file(argv[1]);
  auto volume_stream = std::ifstream{ volume_file, std::ios::binary };

  std::optional<siege::platform::resource_reader> archive;

  for (const auto [checker, creator] : vol_checkers)
  {
    if (checker(volume_stream))
    {
      archive.emplace(creator());
      break;
    }
  }

  if (!archive)
  {
    std::cerr << "Could not extract " << volume_file << '\n';
    return EXIT_FAILURE;
  }

  std::any cache;
  auto files = archive->get_content_listing(cache, volume_stream, { volume_file, volume_file });

  std::string output_folder = replace_extension(volume_file);

  std::function<void(decltype(files)&)> extract_files = [&](const auto& files){
    for (const auto& some_file : files)
    {
      std::visit([&](const auto& info) {
        using info_type = std::decay_t<decltype(info)>;

        if constexpr (std::is_same_v<info_type, siege::platform::file_info>)
        {
          auto final_folder = output_folder / std::filesystem::relative(replace_extension(info.folder_path.string()), output_folder);
          std::filesystem::create_directories(final_folder);
          auto filename = final_folder / info.filename;
          auto new_stream = std::ofstream{ filename, std::ios::binary };
          archive->extract_file_contents(cache, volume_stream, info, new_stream);
        }

        if constexpr (std::is_same_v<info_type, siege::platform::folder_info>)
        {
          // TODO think about whether get_content_listing should preserve the original position of the stream or not.
          std::ifstream temp_stream{ volume_file, std::ios::binary };
          auto files = archive->get_content_listing(cache, temp_stream, { volume_file, info.full_path });
          extract_files(files);
        }
      }, some_file);
    }
  };

  extract_files(files);
}
