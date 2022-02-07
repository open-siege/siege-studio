#include <catch2/catch.hpp>
#include <sstream>
#include "Registry.hpp"

LSTATUS APIENTRY MockRegCloseKey(HKEY hKey)
{
    return ERROR_INVALID_FUNCTION;
}

LSTATUS APIENTRY MockRegOpenKeyExA(
        HKEY hKey,
        LPCSTR lpSubKey,
        DWORD,
        REGSAM,
        PHKEY phkResult
)
{
    std::string Temp(lpSubKey);
    std::transform(Temp.begin(), Temp.end(), Temp.begin(),
                   [](char c) { return std::tolower(c); });
    if (hKey == HKEY_LOCAL_MACHINE && Temp == "software")
    {
        *phkResult = reinterpret_cast<HKEY>(10);
        return ERROR_SUCCESS;
    }

    return ERROR_INVALID_FUNCTION;
}

TEST_CASE("Wrapped Windows Registry APIs", "[darkstar.registry]")
{
    TrueRegCloseKey = MockRegCloseKey;
    TrueRegOpenKeyExA = MockRegOpenKeyExA;

    SECTION("WrappedRegOpenKeyExA will load supported virtual keys, one by one") {
        LoadedPaths.clear();
        HKEY Key = nullptr;
        auto Result = WrappedRegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software", 0, 0, &Key);

        REQUIRE(Result == ERROR_SUCCESS);
        REQUIRE(Key == reinterpret_cast<HKEY>(10));
        REQUIRE(LoadedPaths.size() == 1);
        REQUIRE(LoadedPaths.find("software") != LoadedPaths.end());

        HKEY NextKey = nullptr;
        Result = WrappedRegOpenKeyExA(Key, "Dynamix", 0, 0, &NextKey);

        REQUIRE(Result == ERROR_SUCCESS);
        REQUIRE(NextKey == reinterpret_cast<HKEY>(1));
        REQUIRE(LoadedPaths.size() == 2);
        REQUIRE(LoadedPaths.find("dynamix") != LoadedPaths.end());

        Result = WrappedRegOpenKeyExA(NextKey, "Starsiege", 0, 0, &NextKey);

        REQUIRE(Result == ERROR_SUCCESS);
        REQUIRE(NextKey == reinterpret_cast<HKEY>(2));
        REQUIRE(LoadedPaths.size() == 3);
        REQUIRE(LoadedPaths.find("starsiege") != LoadedPaths.end());
    }

    SECTION("WrappedRegOpenKeyExA will defer to the real Windows API when the key is not supported") {
        HKEY NextKey = nullptr;
        auto Result = WrappedRegOpenKeyExA(HKEY_CURRENT_USER, "Software", 0, 0, &NextKey);

        REQUIRE(Result == ERROR_INVALID_FUNCTION);
    }

    SECTION("WrappedRegOpenKeyExA will load supported virtual keys, with a full path") {
        LoadedPaths.clear();
        HKEY Key = nullptr;
        auto Result = WrappedRegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Dynamix\\Starsiege", 0, 0, &Key);

        REQUIRE(Result == ERROR_SUCCESS);
        REQUIRE(Key == reinterpret_cast<HKEY>(2));
        REQUIRE(LoadedPaths.size() == 3);
        REQUIRE(LoadedPaths.find("starsiege") != LoadedPaths.end());
    }

    SECTION("WrappedRegCloseKey will clear virtual key data if loaded") {
        LoadedPaths.clear();
        LoadedPaths.emplace(std::make_pair("TestKey1", reinterpret_cast<HKEY>(50)));
        LoadedPaths.emplace(std::make_pair("TestKey2", reinterpret_cast<HKEY>(60)));

        auto Result = WrappedRegCloseKey(reinterpret_cast<HKEY>(50));

        REQUIRE(Result == ERROR_SUCCESS);
        REQUIRE(LoadedPaths.size() == 1);
        REQUIRE(LoadedPaths.find("TestKey1") == LoadedPaths.end());


        Result = WrappedRegCloseKey(reinterpret_cast<HKEY>(60));

        REQUIRE(Result == ERROR_SUCCESS);
        REQUIRE(LoadedPaths.empty());
        REQUIRE(LoadedPaths.find("TestKey2") == LoadedPaths.end());
    }
}