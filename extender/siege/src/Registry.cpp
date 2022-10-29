#include <map>
#include <array>
#include <string_view>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <windows.h>
#include "Registry.hpp"

constexpr static std::array<std::string_view, 3> SettingsPath = {
  "software",
  "dynamix",
  "starsiege"
};

static std::map<std::string_view, HKEY> LoadedPaths{};

static auto* TrueRegCreateKeyExA = RegCreateKeyExA;
static auto* TrueRegDeleteKeyA = RegDeleteKeyA;
static auto* TrueRegEnumKeyExA = RegEnumKeyExA;
static auto* TrueRegEnumValueA = RegEnumValueA;
static auto* TrueRegQueryValueExA = RegQueryValueExA;
static auto* TrueRegSetValueExA = RegSetValueExA;

LSTATUS APIENTRY WrappedRegCloseKey(decltype(RegCloseKey) original, std::map<std::string_view, HKEY>& loadedPaths, HKEY hKey)
{
  for (auto& [Name, Key] : loadedPaths)
  {
    if (Key == hKey)
    {
      loadedPaths.erase(Name);
      return ERROR_SUCCESS;
    }
  }
  return original(hKey);
}

LSTATUS APIENTRY WrappedRegCreateKeyExA(
  HKEY hKey,
  LPCSTR lpSubKey,
  DWORD Reserved,
  LPSTR lpClass,
  DWORD dwOptions,
  REGSAM samDesired,
  const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  PHKEY phkResult,
  LPDWORD lpdwDisposition)
{
  return TrueRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

LSTATUS APIENTRY WrappedRegDeleteKeyA(
  HKEY hKey,
  LPCSTR lpSubKey)
{
  return TrueRegDeleteKeyA(hKey, lpSubKey);
}

LSTATUS APIENTRY WrappedRegEnumKeyExA(
  HKEY hKey,
  DWORD dwIndex,
  LPSTR lpName,
  LPDWORD lpcchName,
  LPDWORD lpReserved,
  LPSTR lpClass,
  LPDWORD lpcchClass,
  PFILETIME lpftLastWriteTime)
{
  return TrueRegEnumKeyExA(hKey, dwIndex, lpName, lpcchName, lpReserved, lpClass, lpcchClass, lpftLastWriteTime);
}

LSTATUS APIENTRY WrappedRegEnumValueA(HKEY hKey,
  DWORD dwIndex,
  LPSTR lpValueName,
  LPDWORD lpcchValueName,
  LPDWORD lpReserved,
  LPDWORD lpType,
  LPBYTE lpData,
  LPDWORD lpcbData)
{
  return TrueRegEnumValueA(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
}

LSTATUS APIENTRY WrappedRegOpenKeyExA(
  decltype(RegOpenKeyExA) original,
  std::map<std::string_view, HKEY>& loadedPaths,
  HKEY hKey,
  LPCSTR lpSubKey,
  DWORD ulOptions,
  REGSAM samDesired,
  PHKEY phkResult)
{
  std::string Temp(lpSubKey);

  if (Temp.back() == '\\')
  {
    Temp.pop_back();
  }

  std::transform(Temp.begin(), Temp.end(), Temp.begin(), [](char c) { return std::tolower(c); });

  if (Temp.find('\\', 0) != std::string::npos)
  {
    static std::string MergedResult;

    if (MergedResult.empty())
    {
      std::stringstream TempStream;

      for (auto& Path : SettingsPath)
      {
        TempStream << Path;

        if (Path != SettingsPath.back())
        {
          TempStream << '\\';
        }
      }

      MergedResult = TempStream.str();
    }

    if (Temp == MergedResult)
    {
      HKEY Key = HKEY_LOCAL_MACHINE;
      LSTATUS Result;
      for (auto& Path : SettingsPath)
      {
        Result = WrappedRegOpenKeyExA(original, loadedPaths, Key, std::string(Path).c_str(), 0, 0, &Key);

        if (Result != ERROR_SUCCESS)
        {
          break;
        }
      }

      *phkResult = Key;
      return Result;
    }
  }

  if (hKey == HKEY_LOCAL_MACHINE && Temp == SettingsPath[0])
  {
    auto alreadyOpened = LoadedPaths.find(SettingsPath[0]);

    if (alreadyOpened != LoadedPaths.end())
    {
      *phkResult = alreadyOpened->second;
      return ERROR_SUCCESS;
    }

    auto result = original(hKey, lpSubKey, ulOptions, samDesired, phkResult);

    if (result == ERROR_SUCCESS)
    {
      LoadedPaths.emplace(std::make_pair(SettingsPath[0], *phkResult));
    }
    return result;
  }

  for (auto& [Name, Key] : LoadedPaths)
  {
    for (auto i = 0; i < 2; ++i)
    {
      if (Key == hKey && Name == SettingsPath[i] && Temp == SettingsPath[i + 1])
      {
        auto New = LoadedPaths.emplace(std::make_pair(SettingsPath[i + 1], reinterpret_cast<HKEY>(i + 1)));

        *phkResult = New.first->second;
        return ERROR_SUCCESS;
      }
    }
  }

  return original(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

LSTATUS APIENTRY WrappedRegQueryValueExA(
  HKEY hKey,
  LPCSTR lpValueName,
  LPDWORD lpReserved,
  LPDWORD lpType,
  LPBYTE lpData,
  LPDWORD lpcbData)
{
  std::string Temp(lpValueName);
  std::transform(Temp.begin(), Temp.end(), Temp.begin(), [](char c) { return std::tolower(c); });

  for (auto& [Name, Key] : LoadedPaths)
  {
    if (Name == SettingsPath.back() && Key == hKey && lpData && lpcbData)
    {
      if (Temp == "path")
      {
        auto CurrentDir = std::filesystem::current_path().string();
        auto Size = *lpcbData;
        Size = Size > CurrentDir.size() ? Size : CurrentDir.size();
        std::copy(CurrentDir.data(), CurrentDir.data() + Size, lpData);
        return ERROR_SUCCESS;
      }
      else if (Temp == "installtype")
      {
        constexpr static std::string_view Type = "Full";
        std::copy(Type.data(), Type.data() + Type.size(), lpData);

        return ERROR_SUCCESS;
      }
    }
  }

  return TrueRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

LSTATUS APIENTRY WrappedRegSetValueExA(
  HKEY hKey,
  LPCSTR lpValueName,
  DWORD Reserved,
  DWORD dwType,
  const BYTE* lpData,
  DWORD cbData)
{
  return TrueRegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}


static auto* TrueRegCloseKey = RegCloseKey;
LSTATUS APIENTRY FinalRegCloseKey(HKEY hKey)
{
  return WrappedRegCloseKey(TrueRegCloseKey, LoadedPaths, hKey);
}
//static_assert(std::is_same_v<TrueRegCloseKey, FinalRegCloseKey>());

static auto* TrueRegOpenKeyExA = RegOpenKeyExA;
LSTATUS APIENTRY FinalRegOpenKeyExA(
  HKEY hKey,
  LPCSTR lpSubKey,
  DWORD ulOptions,
  REGSAM samDesired,
  PHKEY phkResult)
{
  return WrappedRegOpenKeyExA(TrueRegOpenKeyExA, LoadedPaths, hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

//static_assert(std::is_same_v<TrueRegOpenKeyExA, FinalRegOpenKeyExA>());

std::array<std::pair<void**, void*>, 8> GetRegistryDetours()
{
  return std::array<std::pair<void**, void*>, 8>{ {
    { &(void*&)TrueRegCloseKey, FinalRegCloseKey },
    { &(void*&)TrueRegCreateKeyExA, WrappedRegCreateKeyExA },
    { &(void*&)TrueRegDeleteKeyA, WrappedRegDeleteKeyA },
    { &(void*&)TrueRegEnumKeyExA, WrappedRegEnumKeyExA },
    { &(void*&)TrueRegEnumValueA, WrappedRegEnumValueA },
    { &(void*&)TrueRegOpenKeyExA, FinalRegOpenKeyExA },
    { &(void*&)TrueRegQueryValueExA, WrappedRegQueryValueExA },
    { &(void*&)TrueRegSetValueExA, WrappedRegSetValueExA } } };
}
