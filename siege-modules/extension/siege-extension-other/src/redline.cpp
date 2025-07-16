#include <siege/platform/extension_module.hpp>
#include <siege/extension/shared.hpp>
#include <detours.h>

extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .string_settings = { { L"connect", L"name" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
};

constexpr static std::array<std::string_view, 13> verification_strings = { {
  "connect"sv,
  "name"sv,
  "team"sv,
  "skin"sv,
  "game"sv,
  "host"sv,
  "maxplayers"sv,
  "hostname"sv,
  "console"sv,
  "lobby"sv,
  "redline.cfg"sv,
  "RED6"sv,
  "Redline"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

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

    string_args.emplace_back(std::wstring(L"+") + setting.name);
    string_args.emplace_back(setting.value);
  }

  static std::vector<const wchar_t*> raw_args;
  raw_args.resize(string_args.size());
  *new_size = (std::uint32_t)string_args.size();

  std::transform(string_args.begin(), string_args.end(), raw_args.begin(), [](const std::wstring& value) {
    return value.c_str();
  });

  return raw_args.data();
}


static auto* TrueCoCreateInstance = ::CoCreateInstance;

HRESULT WINAPI WrappedCoCreateInstance(const IID& class_id, IUnknown* outer, DWORD context, const IID& requested, void** out)
{
  static auto wsock = []() -> std::optional<win32::module> {
    try
    {
      std::wstring buffer;
      buffer.resize(::GetDllDirectoryW(0, nullptr));
      buffer.resize(::GetDllDirectoryW((DWORD)buffer.size(), buffer.data()));

      if (!buffer.empty())
      {
        return win32::module(std::filesystem::path(buffer) / "ws2_32.dll");
      }

      return win32::module("ws2_32.dll");
    }
    catch (...)
    {
      return std::nullopt;
    }
  }();

  return TrueCoCreateInstance(class_id, outer, context, requested, out);
}

static std::array<std::pair<void**, void*>, 1> detour_functions{ {
  { &(void*&)TrueCoCreateInstance, WrappedCoCreateInstance },
} };

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
  if constexpr (sizeof(void*) != sizeof(std::uint32_t))
  {
    return TRUE;
  }

  if (DetourIsHelperProcess())
  {
    return TRUE;
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

    auto value = app_module.GetProcAddress<std::uint32_t*>("DisableSiegeExtensionModule");

    if (value && *value == -1)
    {
      return TRUE;
    }
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      try
      {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourAttach(func.first, func.second); });

        DetourTransactionCommit();
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
    }
  }

  return TRUE;
}
}