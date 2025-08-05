#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>

using game_command_line_caps = siege::platform::game_command_line_caps;
using namespace std::literals;

extern "C" {

extern auto command_line_caps = game_command_line_caps{
  .string_settings = { { L"connect" } },
  .ip_connect_setting = L"connect",
};

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 4> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 4>{ {
                                                                                                              { "cls"sv, std::size_t(0x6fb741) },
                                                                                                              { "trace"sv, std::size_t(0x6fb7af) },
                                                                                                              { "Console::logBufferEnabled"sv, std::size_t(0x6fb7b5) },
                                                                                                              { "Console::logMode"sv, std::size_t(0x6fb7fa) },
                                                                                                            } },
  std::array<std::pair<std::string_view, std::size_t>, 4>{ {
    { "cls"sv, std::size_t(0x657211) },
    { "trace"sv, std::size_t(0x65727f) },
    { "Console::logBufferEnabled"sv, std::size_t(0x657285) },
    { "Console::logMode"sv, std::size_t(0x6572ca) },
  } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 28> function_name_ranges{ { { "File::findFirst"sv, "File::getPath"sv },
  { "setMaterialProperty"sv, "addExportText"sv },
  { "spawnPlayer"sv, "Client::cmdObservePlayer"sv },
  { "AI::Spawn"sv, "Object::getName"sv },
  { "ME::Create"sv, "ME::RebuildCommandMap"sv },
  { "loadObject"sv, "getNextObject"sv },
  { "HTMLOpen"sv, "HTMLOpenAndGoWin"sv },
  { "swapSurfaces"sv, "isGfxDriver"sv },
  { "netStats"sv, "net::kick"sv },
  { "newRedbook"sv, "rbSetPlayMode"sv },
  { "newInputManager"sv, "popActionMap"sv },
  { "simTreeCreate"sv, "simTreeRegScript"sv },
  { "newSfx"sv, "sfxGetMaxBuffers"sv },
  { "newToolWindow"sv, "saveFileAs"sv },
  { "newTerrain"sv, "lightTerrain"sv },
  { "GuiEditMode"sv, "windowsKeyboardDisable"sv },
  { "LS::Create"sv, "LS::parseCommands"sv },
  { "ircConnect"sv, "ircEcho"sv },
  { "BaseRep::getFirst"sv, "Object::getName"sv },
  { "FGTextList::sort"sv, "FGTextList::sort"sv },
  { "CmdInventory::getVisibleSet"sv, "CmdInventory::setFavorites"sv },
  { "FGSkin::set"sv, "FGSkin::cycleArmor"sv },
  { "FGArray::addEntry"sv, "FGArray::getSelectedText"sv },
  { "FGCombo::addEntry"sv, "FGCombo::selectNext"sv },
  { "Server::ResortList"sv, "Server::ResortList"sv },
  { "FGSlider::setDiscretePositions"sv, "FGSlider::setDiscretePositions"sv },
  { "FGMasterList::addEntry"sv, "FGMasterList::getSelected"sv },
  { "cls"sv, "trace"sv } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 13> variable_name_ranges{ { { "Console::ForeRGB"sv, "Console::LastLineTimeout"sv },
  { "$pref::mipcap"sv, "$OpenGL::TNB"sv },
  { "GFXMetrics::EmittedPolys"sv, "GFXMetrics::numPaletteDLs"sv },
  { "pref::sfx2DVolume"sv, "pref::sfx3DVolume"sv },
  { "GuiEdit::GridSnapX"sv, "pref::politeGui"sv },
  { "Console::logBufferEnabled"sv, "Console::logMode"sv },
  { "DNet::ShowStats"sv, "DNet::PacketLoss"sv } } };

std::errc get_function_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(function_name_ranges, length, data, saved);
}

std::errc get_variable_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(variable_name_ranges, length, data, saved);
}


std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
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

    auto& back = string_args.emplace_back(setting.name);
    back.insert(0, 1, L'+');
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

static auto* TrueSetWindowsHookExA = SetWindowsHookExA;
static auto* TrueAllocConsole = AllocConsole;

BOOL WINAPI WrappedAllocConsole()
{
  return TrueAllocConsole();
}

HHOOK WINAPI WrappedSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
  if (dwThreadId == 0)
  {
    dwThreadId = ::GetCurrentThreadId();
  }

  return TrueSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
}

static std::array<std::pair<void**, void*>, 2> detour_functions{ { { &(void*&)TrueSetWindowsHookExA, WrappedSetWindowsHookExA },
  { &(void*&)TrueAllocConsole, WrappedAllocConsole } } };

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
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
      int index = 0;
      try
      {
        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourAttach(func.first, func.second); });

        DetourTransactionCommit();
        //auto host = std::make_unique<siege::extension::DarkstarScriptDispatch>(std::move(functions), std::move(variables), [](std::string_view eval_string) -> std::string_view {
        //  std::array<const char*, 2> args{ "eval", eval_string.data() };

        //  // Luckily this function is static and doesn't need the console instance object nor
        //  // an ID to identify the callback. It doesn't even check for "eval" and skips straight to the second argument.
        //  auto result = ConsoleEval(nullptr, 0, 2, args.data());

        //  if (result == nullptr)
        //  {
        //    return "";
        //  }


        //  return result;
        //});
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());

      std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourDetach(func.first, func.second); });
      DetourTransactionCommit();
    }
  }

  return TRUE;
}
}
