#ifndef DARKSTAR_REGISTRY_HPP
#define DARKSTAR_REGISTRY_HPP

#include <map>
#include <string_view>
#include <array>
#include <utility>
#include <windows.h>

std::array<std::pair<void**, void*>, 8> GetRegistryDetours();


LSTATUS APIENTRY WrappedRegCloseKey(decltype(RegCloseKey) original, std::map<std::string_view, HKEY>& loadedPaths, HKEY hKey);
LSTATUS APIENTRY WrappedRegOpenKeyExA(
  decltype(RegOpenKeyExA) original,
  std::map<std::string_view, HKEY>& loadedPaths,
  HKEY hKey,
  LPCSTR lpSubKey,
  DWORD ulOptions,
  REGSAM samDesired,
  PHKEY phkResult);

#endif //DARKSTAR_REGISTRY_HPP
