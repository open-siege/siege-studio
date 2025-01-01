#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/resource/zip_resource.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
namespace fs = std::filesystem;
using namespace std::literals;
extern game_command_line_caps command_line_caps;

// TODO
// Generate cfg file for controller settings
// Copy glide files to the game folder - use to-be-implemented shared list of detected items
HRESULT apply_prelaunch_settings(const wchar_t* exe_path_str, const siege::platform::game_command_line_args* args)
{
  if (exe_path_str == nullptr)
  {
    return E_POINTER;
  }

  if (args == nullptr)
  {
    return E_POINTER;
  }

  std::error_code last_error;

  auto exe_path = fs::path(exe_path_str);

  HKEY current_user = nullptr;
  if (::RegOpenCurrentUser(KEY_WRITE, &current_user) == 0)
  {
    std::wstring compat = L"~ HIGHDPIAWARE";
    HKEY compat_key = nullptr;
    if (::RegOpenKeyExW(current_user, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 0, KEY_WRITE, &compat_key) == 0)
    {
      ::RegSetValueExW(compat_key, exe_path_str, 0, REG_SZ, (BYTE*)compat.data(), compat.size() * 2);
      ::RegCloseKey(compat_key);
    }

    ::RegCloseKey(current_user);
  }

  return S_OK;
}

// TODO
// Specify controller cfg in the command line
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
      if (setting.value == L"0.0.0.0"sv)
      {
        continue;
      }

      string_args.emplace_back(L"+connect");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"map")
    {
      string_args.emplace_back(L"+map");
      string_args.emplace_back(setting.value);
    }
    else
    {
      string_args.emplace_back(L"+set");
      string_args.emplace_back(setting.name);
      string_args.emplace_back(setting.value);
    }
  }

  if (string_args.empty())
  {
    return nullptr;
  }

  static std::vector<const wchar_t*> raw_args;
  raw_args.resize(string_args.size());
  *new_size = (std::uint32_t)string_args.size();

  std::transform(string_args.begin(), string_args.end(), raw_args.begin(), [](const std::wstring& value) {
    return value.c_str();
  });

  return raw_args.data();
}

predefined_string*
  get_predefined_id_tech_3_map_command_line_settings(const wchar_t* base_dir) noexcept
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
        if (dir_entry.path().extension() == L".zip" || dir_entry.path().extension() == L".ZIP"
            || dir_entry.path().extension() == L".pk3" || dir_entry.path().extension() == L".PK3"
            || dir_entry.path().extension() == L".hwp" || dir_entry.path().extension() == L".HWP")
        {
          pak_files.emplace_back(dir_entry.path());
        }
      }

      std::mutex storage_lock;

      std::for_each(std::execution::par_unseq, pak_files.begin(), pak_files.end(), [&storage_lock](auto& dir_entry) {
        std::any cache;
        std::ifstream stream(dir_entry, std::ios::binary);

        siege::resource::zip::zip_resource_reader reader;

        if (!reader.is_supported(stream))
        {
          return;
        }

        auto contents = reader.get_content_listing(cache, stream, { .archive_path = dir_entry, .folder_path = dir_entry / "maps" });

        for (auto& content : contents)
        {
          if (auto* folder_info = std::get_if<siege::platform::resource_reader::folder_info>(&content); folder_info)
          {
            contents.append_range(reader.get_content_listing(cache, stream, { .archive_path = dir_entry, .folder_path = folder_info->full_path }));
          }
        }

        if (storage.capacity() == 0)
        {
          storage.reserve(contents.size());
        }

        const std::lock_guard<std::mutex> lock(storage_lock);

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

            if (temp.string() == "maps")
            {
              temp = "";
            }
            else if (temp.string().starts_with("maps/") || temp.string().starts_with("maps\\"))
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