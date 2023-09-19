#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/id_tech.hpp"

namespace darkstar = studio::resources::vol::darkstar;

namespace id_tech = studio::configurations::id_tech;

TEST_CASE("Parsing of an id tech config file", "[vol.darkstar]")
{
  SECTION("When data is empty nullopt returned")
  {
    std::stringstream data;
    auto value = id_tech::id_tech_2::load_config(data, 0);

    REQUIRE(value == std::nullopt);
  }

  SECTION("With sample id Tech 2.5 data, values convert correctly")
  {
    std::stringstream raw_config;
    raw_config << "unbindall\n";

    raw_config << "bind MOUSE1 \"+attack\"\n";
    raw_config << "bind MOUSE2 \"+altattack\"\n";
    raw_config << "bind W \"+forward\"\n";
    raw_config << "bind A \"+moveleft\"\n";
    raw_config << "bind S \"+backward\"\n";
    raw_config << "bind D \"+moveright\"\n";

    raw_config << "set log_file_name \"\"\n";
    raw_config << "set bestweap \"safe\"\n";
    raw_config << "set name \"Mohn Jullins\"\n";

    raw_config << "set net_socksPort \"1000\"\n";
    raw_config << "set ai_maxcorpses \"4\"\n";
    raw_config << "set gl_swapinterval \"1\"\n";

    raw_config << "set gl_offsetunits \"-2.0\"\n";
    raw_config << "set ghl_shadow_darkness \".75\"\n";    
    raw_config << "set vid_brightness \"0.600000\"\n";

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

    REQUIRE(value.config_data.size() == 16);

    REQUIRE(value.find_to_string("unbindall") == "");

    REQUIRE(value.find_string("bind/MOUSE1") == "+attack");
    REQUIRE(value.find_string("bind/MOUSE2") == "+altattack");
    REQUIRE(value.find_string("bind/W") == "+forward");
    REQUIRE(value.find_string("bind/A") == "+moveleft");
    REQUIRE(value.find_string("bind/S") == "+backward");
    REQUIRE(value.find_string("bind/D") == "+moveright");

    REQUIRE(value.find_to_string("set/log_file_name") == "");
    REQUIRE(value.find_to_string("set/bestweap") == "safe");
    REQUIRE(value.find_to_string("set/name") == "Mohn Jullins");

    REQUIRE(value.find_uint32_t("set/net_socksPort") == 1000);
    REQUIRE(value.find_uint32_t("set/ai_maxcorpses") == 4);
    REQUIRE(value.find_uint32_t("set/gl_swapinterval") == 1);

    REQUIRE(value.find_float("set/gl_offsetunits") == -2.0);
    REQUIRE(value.find_float("set/ghl_shadow_darkness") == 0.75);
    REQUIRE(value.find_float("set/vid_brightness") == 0.6);
  }

}
