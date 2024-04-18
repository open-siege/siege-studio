#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/build.hpp"

namespace build = siege::configuration::build;

using namespace std::literals;

TEST_CASE("Parsing of an build config file", "[cfg.build]")
{
  SECTION("With sample build data, values convert correctly")
  {
    std::stringstream raw_config;
    raw_config << "[Controls]" << "\r\n";

    raw_config << "JoystickButton0 = \"Fire\"" << "\r\n";
    raw_config << "MouseAiming = 0" << "\r\n";
    raw_config << "JoystickAnalogAxes0 = \"analog_turning\"" << "\r\n";

    auto value = build::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.has_value() == true);

    REQUIRE(value.value().keys().size() == 3);

    REQUIRE(value.value().find({"Controls", "JoystickButton0"}) == "Fire");
    REQUIRE(value.value().find({"Controls", "MouseAiming"}) == "0");
    REQUIRE(value.value().find({"Controls", "JoystickAnalogAxes0"}) == "analog_turning");
  }
}
