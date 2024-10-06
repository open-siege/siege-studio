#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/content/pal/palette.hpp>

TEST_CASE("Phoenix PAL is detected correctly", "[pal.darkstar]")
{
  std::stringstream mem_buffer;
  mem_buffer << "RAFF11111111111";

  REQUIRE(siege::content::pal::is_phoenix_pal(mem_buffer) == false);
  REQUIRE(mem_buffer.tellg() == 0);

  mem_buffer.clear();
  mem_buffer.str("");
  mem_buffer << "PL98";

  REQUIRE(siege::content::pal::is_phoenix_pal(mem_buffer) == true);
  REQUIRE(mem_buffer.tellg() == 0);
}
