#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/id_tech.hpp"

namespace darkstar = studio::resources::vol::darkstar;

TEST_CASE("Parsing of an id tech config file", "[vol.darkstar]")
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
