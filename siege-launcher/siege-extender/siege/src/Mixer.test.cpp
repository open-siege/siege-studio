#include <catch2/catch.hpp>
#include <sstream>
#include "Mixer.hpp"


UINT WINAPI NumDevsIsTwo()
{
  return 2;
}

TEST_CASE("Wrapped Windows Mixer APIs", "[darkstar.mixer]")
{
  SECTION("DarkMixerGetNumDevs with give the number of devices, plus one") {
        auto Result = DarkMixerGetNumDevs(NumDevsIsTwo);

        REQUIRE(Result == 3);
    }
}
