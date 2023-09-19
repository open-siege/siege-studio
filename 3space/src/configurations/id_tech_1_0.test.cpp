#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/id_tech.hpp"

namespace darkstar = studio::resources::vol::darkstar;

/*


key_left		75
key_up		72
key_down		80
key_strafeleft		51
key_straferight		52
key_fire		29
key_use		57
key_strafe		56
key_speed		54
use_mouse		1
mouseb_fire		0
mouseb_strafe		1
mouseb_forward		2
use_joystick		0
joyb_fire		0
joyb_strafe		1
joyb_use		3
joyb_speed		2
*/

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
    const auto* test_data = "key_left\t\t75\r\nkey_up\t\t72";

    std::stringstream data;
    auto value = id_tech::id_tech_1::load_config(data, 0);

    REQUIRE(value.find_uint32_t("key_left") == 75);
    REQUIRE(value.find_uint32_t("key_up") == 72);
  }

  SECTION("With sample data type is correct")
  {
    const auto* test_data = "key_left\t\t75\r\nkey_up\t\t72";

    std::stringstream data;
    auto value = id_tech::id_tech_1::load_config(data, 0);

    REQUIRE(value.find_to_string("__ConfigType") == "id_tech_1_0");
  }

}
