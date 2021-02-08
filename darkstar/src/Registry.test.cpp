#include <catch2/catch.hpp>
#include <sstream>
#include "Registry.hpp"

LSTATUS APIENTRY MockRegCloseKey(HKEY hKey)
{
    return ERROR_INVALID_FUNCTION;
}

TEST_CASE("Windows Registry APIs can be wrapped", "[darkstar.registry]")
{
    TrueRegCloseKey = MockRegCloseKey;
    SECTION("WrappedRegCloseKey will clear virtual key data if loaded") {
        LoadedPaths.emplace(std::make_pair("TestKey1", reinterpret_cast<HKEY>(50)));
        LoadedPaths.emplace(std::make_pair("TestKey2", reinterpret_cast<HKEY>(60)));

        auto Result = WrappedRegCloseKey(reinterpret_cast<HKEY>(50));

        REQUIRE(Result == ERROR_SUCCESS);
        REQUIRE(LoadedPaths.size() == 1);
        REQUIRE(LoadedPaths.find("TestKey1") == LoadedPaths.end());


        Result = WrappedRegCloseKey(reinterpret_cast<HKEY>(60));

        REQUIRE(Result == ERROR_SUCCESS);
        REQUIRE(LoadedPaths.size() == 0);
        REQUIRE(LoadedPaths.find("TestKey2") == LoadedPaths.end());
    }
}