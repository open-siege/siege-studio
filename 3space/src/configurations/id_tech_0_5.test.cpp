#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/id_tech.hpp"

namespace darkstar = studio::resources::vol::darkstar;


// line ends with /r/n
// comment starts with ;
// key starts with word and any number of spaces then the value

namespace id_tech = studio::configurations::id_tech;

TEST_CASE("Parsing of an id tech config file", "[vol.darkstar]")
{
  SECTION("When data is empty nullopt returned")
  {
    std::stringstream data;
    auto value = id_tech::id_tech_2::load_config(data, 0);

    REQUIRE(value == std::nullopt);
  }

  SECTION("With sample data reads each line correctly")
  {
    const auto* test_data = "; 1 - Joystick Enabled\r\n; 0 - Joystick Disabled\nJoystickEnabled    1\r\n;\r\n; 1 - Joypad Enabled\r\n; 0 - Joypad Disabled\r\nJoypadEnabled      0";

    std::stringstream data;
    auto value = id_tech::id_tech_0_5::rott::load_config(data, 0);

    REQUIRE(value.find_uint32_t("JoystickEnabled") == 1);
    REQUIRE(value.find_uint32_t("JoypadEnabled") == 0);
  }

  SECTION("With sample data type is correct")
  {
    const auto* test_data = "; 1 - Joystick Enabled\r\n; 0 - Joystick Disabled\nJoystickEnabled    1\r\n;\r\n; 1 - Joypad Enabled\r\n; 0 - Joypad Disabled\r\nJoypadEnabled      0";

    std::stringstream data;
    auto value = id_tech::id_tech_0_5::rott::load_config(data, 0);

    REQUIRE(value.find_to_string("__ConfigType") == "id_tech_0_5::rott");
  }
}
