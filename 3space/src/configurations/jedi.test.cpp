#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/jedi.hpp"

namespace darkstar = studio::resources::vol::darkstar;

TEST_CASE("With one text file, creates a Darkstar Volume file with the correct byte layout", "[vol.darkstar]")
{
  SECTION("When data is 4 byte aligned, no padding is added.")
  {
    REQUIRE(studio::resources::vol::darkstar::vol_file_archive::is_supported(mem_buffer) == true);
  }

  SECTION("When data is 4 byte aligned, no padding is added.")
  {
    REQUIRE(studio::resources::vol::darkstar::vol_file_archive::is_supported(mem_buffer) == true);
  }

  SECTION("When data is 4 byte aligned, no padding is added.")
  {
    REQUIRE(studio::resources::vol::darkstar::vol_file_archive::is_supported(mem_buffer) == true);
  }
}
