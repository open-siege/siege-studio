#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include <filesystem>
#include <fstream>
#include "shared.hpp"


extern "C" {
using namespace std::literals;
using game_command_line_caps = siege::platform::game_command_line_caps;
namespace fs = std::filesystem;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"+connect",
  .player_name_setting = L"player_name",
  .string_settings = { { L"+connect", L"player_name" } }
};

// +connect IP:port
// +connect IP:0
// HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers
// Name == full path
// value == WINXPSP3 HIGHDPIAWARE

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "GAME.DIC"sv,
  "[FINAL_DRIVE]"sv,
  "[GEAR_RATIOS]"sv,
  "gtr2ui.mnu"sv,
  "specialfx.tec"sv,
  "GTR_ONLINE_LOBBY"sv,
  "GTR_ONLINE_LOGIN"sv,
  "GTR_ONLINE_CDKEY"sv,
  "GARAGEONLINEPAGE"sv,
  "STEX.GTR"sv,
  "Net.dll"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

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

  auto parent_path = exe_path.parent_path();

  if (!fs::exists(parent_path / "SDNDTG.DYN", last_error))
  {
    std::ofstream temp(parent_path / "SDNDTG.DYN", std::ios::trunc);
  }

  auto player_name = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) {
    return setting.name && std::wstring_view(setting.name) == command_line_caps.player_name_setting && setting.value && setting.value[0] != L'\0';
  });

  if (player_name != args->string_settings.end())
  {
    try
    {
      std::vector<fs::path> paths_to_rename;

      for (auto entry = std::filesystem::recursive_directory_iterator(L"UserData");
           entry != std::filesystem::recursive_directory_iterator();
           ++entry)
      {
        if (entry.depth() == 1 && entry->path().extension() == L".PLR" || entry->path().extension() == L".plr")
        {
          paths_to_rename.push_back(entry->path());

          auto other_file = entry->path();
          other_file = other_file.replace_extension(".gal");

          if (std::filesystem::exists(other_file))
          {
            paths_to_rename.push_back(other_file);
          }

          paths_to_rename.push_back(entry->path().parent_path());
          break;
        }
      }

      for (auto& path : paths_to_rename)
      {
        auto new_path = std::filesystem::path(player_name->value).replace_extension(path.extension());

        std::filesystem::rename(path, path.parent_path() / new_path);
      }


      if (paths_to_rename.empty())
      {
        std::error_code code;
        auto new_folder = std::filesystem::path(L"UserData") / player_name->value;

        if (!std::filesystem::exists(new_folder, code))
        {
          std::filesystem::create_directory(new_folder, code);
        }

        std::ofstream((new_folder / player_name->value).replace_extension(".gal"), std::ios::trunc);
        std::ofstream((new_folder / player_name->value).replace_extension(".PLR"), std::ios::trunc);
      }
    }
    catch (...)
    {
    }
  }

  return S_OK;
}
}