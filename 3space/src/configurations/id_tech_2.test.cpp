#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/id_tech.hpp"

namespace darkstar = studio::resources::vol::darkstar;

namespace id_tech = studio::configurations::id_tech;

// C++ style in line comment - //
// bind then space then value then second value
// value always has quotes
// key then space then value
// set then space then key the value
// seta then space then key then value

TEST_CASE("Parsing of an id tech config file", "[vol.darkstar]")
{
  SECTION("When data is empty nullopt returned")
  {
    std::stringstream data;
    auto value = id_tech::id_tech_2::load_config(data, 0);

    REQUIRE(value == std::nullopt);
  }

  SECTION("With sample data reads each line correctly - Quake 1")
  {
    const auto* test_data = "bind \"PAUSE\" \"pause\"\r\nauxlook \"1\"";

    std::stringstream data;
    auto value = id_tech::id_tech_1::load_config(data, 0);

    REQUIRE(value.find_to_string("bind/Pause") == "pause");
    REQUIRE(value.find_to_string("auxlook") == 1);
  }

  SECTION("With sample data reads each line correctly - Quake 2")
  {
    const auto* test_data = "bind \"PAUSE\" \"pause\"\r\nset in_joystick \"0\"";

    std::stringstream data;
    auto value = id_tech::id_tech_1::load_config(data, 0);

    REQUIRE(value.find_to_string("bind/Pause") == "pause");
    REQUIRE(value.find_to_string("set/in_joystick") == 0);
  }

  SECTION("With sample data reads each line correctly - Quake 3")
  {
    const auto* test_data = "bind \"PAUSE\" \"pause\"\r\nseta com_hunkMegs \"56\"";

    std::stringstream data;
    auto value = id_tech::id_tech_1::load_config(data, 0);

    REQUIRE(value.find_to_string("bind/Pause") == "pause");
    REQUIRE(value.find_to_string("seta/com_hunkMegs") == 56);
  }

  SECTION("With sample data type is correct")
  {
    const auto* test_data = "key_left\t\t75\r\nkey_up\t\t72";

    std::stringstream data;
    auto value = id_tech::id_tech_1::load_config(data, 0);

    REQUIRE(value.find_to_string("__ConfigType") == "id_tech_2");
  }

}
