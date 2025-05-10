#include <siege/platform/win/window_module.hpp>
#include <siege/resource/zip_resource.hpp>
#include <siege/resource/pak_resource.hpp>
#include <algorithm>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
namespace fs = std::filesystem;
extern game_command_line_caps command_line_caps;
using namespace std::literals;

// TODO
// Generate cfg file for controller settings
// Copy glide files to the game folder - use to-be-implemented shared list of detected items
const wchar_t** format_command_line(const siege::platform::game_command_line_args* args, std::uint32_t* new_size)
{
  if (!args)
  {
    return nullptr;
  }

  if (!new_size)
  {
    return nullptr;
  }

  static std::vector<std::wstring> string_args;
  string_args.clear();

  for (auto& setting : args->string_settings)
  {
    if (!setting.name)
    {
      continue;
    }

    if (!setting.value)
    {
      continue;
    }

    if (!setting.value[0])
    {
      continue;
    }

    if (std::wstring_view(setting.name) == command_line_caps.ip_connect_setting)
    {
      string_args.emplace_back(L"+connect");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"map")
    {
      string_args.emplace_back(L"+map");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"exec")
    {
      string_args.emplace_back(L"+exec");
      string_args.emplace_back(setting.value);
    }
    else
    {
      string_args.emplace_back(L"+set");
      string_args.emplace_back(setting.name);
      string_args.emplace_back(setting.value);
    }
  }

  static std::vector<const wchar_t*> raw_args;
  raw_args.resize(string_args.size());
  *new_size = (std::uint32_t)string_args.size();

  std::transform(string_args.begin(), string_args.end(), raw_args.begin(), [](const std::wstring& value) {
    return value.c_str();
  });

  return raw_args.data();
}

// TODO
// Specify controller cfg in the command line
predefined_string*
  get_predefined_id_tech_2_map_command_line_settings(const wchar_t* base_dir, bool include_zip) noexcept
{
  static std::vector<std::wstring> storage;
  static std::vector<predefined_string> results;

  if (!results.empty())
  {
    return results.data();
  }

  try
  {
    std::error_code errc{};

    if (fs::is_directory(base_dir, errc))
    {
      std::vector<fs::path> pak_files;
      pak_files.reserve(16);

      for (auto const& dir_entry : std::filesystem::directory_iterator{ base_dir })
      {
        if ((include_zip && dir_entry.path().extension() == L".zip" || include_zip && dir_entry.path().extension() == L".ZIP") || (dir_entry.path().extension() == L".dat" || dir_entry.path().extension() == L".DAT") || dir_entry.path().extension() == L".pak" || dir_entry.path().extension() == L".PAK")
        {
          pak_files.emplace_back(dir_entry.path());
        }
      }

      storage.reserve(pak_files.size() * pak_files.size());

      std::for_each(pak_files.begin(), pak_files.end(), [](auto& dir_entry) {
        std::any cache;
        std::ifstream stream(dir_entry, std::ios::binary);

        std::unique_ptr<siege::platform::resource_reader> reader;

        if (siege::resource::pak::pak_resource_reader::is_supported(stream))
        {
          reader = std::make_unique<siege::resource::pak::pak_resource_reader>();
        }
        else if (siege::resource::zip::zip_resource_reader::is_supported(stream))
        {
          reader = std::make_unique<siege::resource::zip::zip_resource_reader>();
        }
        else
        {
          return;
        }

        auto contents = reader->get_content_listing(cache, stream, { .archive_path = dir_entry, .folder_path = dir_entry / "maps" });

        for (auto& content : contents)
        {
          if (auto* folder_info = std::get_if<siege::platform::resource_reader::folder_info>(&content); folder_info)
          {
            contents.append_range(reader->get_content_listing(cache, stream, { .archive_path = dir_entry, .folder_path = folder_info->full_path }));
          }
        }

        if (storage.capacity() == 0)
        {
          storage.reserve(contents.size());
        }

        for (auto& content : contents)
        {
          if (std::get_if<siege::platform::folder_info>(&content) != nullptr)
          {
            continue;
          }
          auto& file_info = std::get<siege::platform::file_info>(content);

          if (file_info.filename.extension() == ".bsp" || file_info.filename.extension() == ".BSP")
          {
            auto temp = fs::relative(file_info.folder_path, file_info.archive_path);

            if (temp.string() == "maps" || temp.string().starts_with("maps/") || temp.string().starts_with("maps\\"))
            {
              temp = temp.string().replace(0, 5, "");
            }

            if (temp.string() == "" || temp.string() == "/" || temp.string() == "\\")
            {
              storage.emplace_back(file_info.filename.stem().wstring());
            }
            else
            {
              std::wstring final_name = (temp / file_info.filename.stem()).wstring();

              while (final_name.contains(std::filesystem::path::preferred_separator))
              {
                final_name = final_name.replace(final_name.find(std::filesystem::path::preferred_separator), 1, std::wstring(L"/"));
              }

              storage.emplace_back(std::move(final_name));
            }
          }
        }
      });
    }

    results.emplace_back(predefined_string{
      .label = L"No map",
      .value = L"" });

    for (auto& string : storage)
    {
      results.emplace_back(predefined_string{
        .label = string.c_str(),
        .value = string.c_str() });
    }

    results.emplace_back(predefined_string{});

    return results.data();
  }
  catch (...)
  {
    return nullptr;
  }
}
}