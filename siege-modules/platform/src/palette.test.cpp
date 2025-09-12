#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/platform/palette.hpp>

TEST_CASE("Microsoft PAL is detected correctly", "[pal.microsoft]")
{
  std::stringstream mem_buffer;
  mem_buffer << "RAFF11111111111";

  REQUIRE(siege::content::pal::is_microsoft_pal(mem_buffer) == false);
  REQUIRE(mem_buffer.tellg() == 0);

  mem_buffer.clear();
  mem_buffer.str("");
  mem_buffer << "RIFF0000PAL ";

  REQUIRE(siege::content::pal::is_microsoft_pal(mem_buffer) == true);
  REQUIRE(mem_buffer.tellg() == 0);
}