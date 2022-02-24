#ifndef DARKSTAR_REGISTRY_HPP
#define DARKSTAR_REGISTRY_HPP

#include <map>
#include <string_view>
#include <array>
#include <utility>
#include <windows.h>

std::array<std::pair<void**, void*>, 8> GetRegistryDetours();

static auto* TrueRegCloseKey = RegCloseKey;
static auto* TrueRegOpenKeyExA = RegOpenKeyExA;
static inline std::map<std::string_view, HKEY> LoadedPaths;

LSTATUS APIENTRY WrappedRegCloseKey(HKEY hKey);
LSTATUS APIENTRY WrappedRegOpenKeyExA(
  HKEY hKey,
  LPCSTR lpSubKey,
  DWORD ulOptions,
  REGSAM samDesired,
  PHKEY phkResult);

#endif //DARKSTAR_REGISTRY_HPP
