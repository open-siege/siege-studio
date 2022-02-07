#ifndef DARKSTAR_REGISTRY_HPP
#define DARKSTAR_REGISTRY_HPP

#include <map>
#include <array>
#include <string_view>
#include <algorithm>
#include <filesystem>
#include <windows.h>

constexpr static std::array<std::string_view, 3> SettingsPath = {
        "software",
        "dynamix",
        "starsiege"};

static auto *TrueRegCloseKey = RegCloseKey;
static auto *TrueRegCreateKeyExA = RegCreateKeyExA;
static auto *TrueRegDeleteKeyA = RegDeleteKeyA;
static auto *TrueRegEnumKeyExA = RegEnumKeyExA;
static auto *TrueRegEnumValueA = RegEnumValueA;
static auto *TrueRegOpenKeyExA = RegOpenKeyExA;
static auto *TrueRegQueryValueExA = RegQueryValueExA;
static auto *TrueRegSetValueExA = RegSetValueExA;

static std::map<std::string_view, HKEY> LoadedPaths;

LSTATUS APIENTRY WrappedRegCloseKey(HKEY hKey)
{
    for (auto&[Name, Key] : LoadedPaths)
    {
        if (Key == hKey) {
            LoadedPaths.erase(Name);
            return ERROR_SUCCESS;
        }
    }
    return TrueRegCloseKey(hKey);
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
        LPDWORD lpdwDisposition
) {
    return TrueRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes,
                               phkResult, lpdwDisposition);
}

LSTATUS APIENTRY WrappedRegDeleteKeyA(
        HKEY hKey,
        LPCSTR lpSubKey
) {
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
        PFILETIME lpftLastWriteTime
) {
    return TrueRegEnumKeyExA(hKey, dwIndex, lpName, lpcchName, lpReserved, lpClass, lpcchClass, lpftLastWriteTime);
}

LSTATUS APIENTRY WrappedRegEnumValueA(HKEY hKey,
                                      DWORD dwIndex,
                                      LPSTR lpValueName,
                                      LPDWORD lpcchValueName,
                                      LPDWORD lpReserved,
                                      LPDWORD lpType,
                                      LPBYTE lpData,
                                      LPDWORD lpcbData
) {
    return TrueRegEnumValueA(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
}

LSTATUS APIENTRY WrappedRegOpenKeyExA(
        HKEY hKey,
        LPCSTR lpSubKey,
        DWORD ulOptions,
        REGSAM samDesired,
        PHKEY phkResult
) {
    std::string Temp(lpSubKey);

    if (Temp.back() == '\\') {
        Temp.pop_back();
    }

    std::transform(Temp.begin(), Temp.end(), Temp.begin(),
                   [](char c) { return std::tolower(c); });

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
                Result = WrappedRegOpenKeyExA(Key, std::string(Path).c_str(), 0, 0, &Key);

                if (Result != ERROR_SUCCESS)
                {
                    break;
                }
            }

            *phkResult = Key;
            return Result;
        }
    }

    if (hKey == HKEY_LOCAL_MACHINE && Temp == SettingsPath[0]) {
        auto alreadyOpened = LoadedPaths.find(SettingsPath[0]);

        if (alreadyOpened != LoadedPaths.end()) {
            *phkResult = alreadyOpened->second;
            return ERROR_SUCCESS;
        }

        auto result = TrueRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);

        if (result == ERROR_SUCCESS) {
            LoadedPaths.emplace(std::make_pair(SettingsPath[0], *phkResult));
        }
        return result;
    }

    for (auto&[Name, Key] : LoadedPaths) {
        for (auto i = 0; i < 2; ++i) {
            if (Key == hKey && Name == SettingsPath[i] && Temp == SettingsPath[i + 1]) {
                auto New = LoadedPaths.emplace(std::make_pair(SettingsPath[i + 1], reinterpret_cast<HKEY>(i + 1)));

                *phkResult = New.first->second;
                return ERROR_SUCCESS;
            }
        }
    }

    return TrueRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

LSTATUS APIENTRY WrappedRegQueryValueExA(
        HKEY hKey,
        LPCSTR lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData
) {
    std::string Temp(lpValueName);
    std::transform(Temp.begin(), Temp.end(), Temp.begin(),
                   [](char c) { return std::tolower(c); });

    for (auto&[Name, Key] : LoadedPaths) {
        if (Name == SettingsPath.back() && Key == hKey && lpData && lpcbData) {
            if (Temp == "path") {
                auto CurrentDir = std::filesystem::current_path().string();
                auto Size = *lpcbData;
                Size = Size > CurrentDir.size() ? Size : CurrentDir.size();
                std::copy(CurrentDir.data(), CurrentDir.data() + Size, lpData);
                return ERROR_SUCCESS;
            } else if (Temp == "installtype") {
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
        const BYTE *lpData,
        DWORD cbData
) {
    return TrueRegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

#endif //DARKSTAR_REGISTRY_HPP
