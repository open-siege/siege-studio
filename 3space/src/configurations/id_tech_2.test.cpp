#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/id_tech.hpp"

namespace id_tech = studio::configurations::id_tech;

using namespace std::literals;

TEST_CASE("Parsing of an id tech config file", "[vol.darkstar]")
{
  SECTION("When data is empty nullopt returned")
  {
    std::stringstream data;
    auto value = id_tech::id_tech_2::load_config(data, 0);

    REQUIRE(value == std::nullopt);
  }


  SECTION("With sample id Tech 2.5 line, with single key and value, values convert correctly")
  {
    std::stringstream raw_config("keya value");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find("keya"sv) == "value");
  }

  SECTION("With sample id Tech 2.5 line, with single quoted key and value, values convert correctly")
  {
    std::stringstream raw_config("\"keya\" value");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find("keya"sv) == "value");
  }

  SECTION("With sample id Tech 2.5 line, with single key and quoted value, values convert correctly")
  {
    std::stringstream raw_config("keya \"value\"");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find("keya"sv) == "value");
  }

  SECTION("With sample id Tech 2.5 line, with single quoted key and quoted value,  values convert correctly")
  {
    std::stringstream raw_config("\"keya\" \"value\"");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find("keya"sv) == "value");
  }

  SECTION("With sample id Tech 2.5 data, with multi part quoted key and value, values convert correctly")
  {
    std::stringstream raw_config("\"multi key\" value");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find("multi key"sv) == "value");
  }

  SECTION("With sample id Tech 2.5 data, with multi part quoted key and multi part quoted value, values convert correctly")
  {
    std::stringstream raw_config("\"multi key\" \"multi value\"");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find("multi key"sv) == "multi value");
  }

  SECTION("With sample id Tech 2.5 line, with single key and value, values convert correctly")
  {
    std::stringstream raw_config("keya keyb value");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find({"keya", "keyb"}) == "value");
  }

  SECTION("With sample id Tech 2.5 line, with single quoted key and value, values convert correctly")
  {
    std::stringstream raw_config("\"keya\" keyb value");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find({"keya", "keyb"}) == "value");
  }

  SECTION("With sample id Tech 2.5 line, with single quoted key and value, values convert correctly")
  {
    std::stringstream raw_config("keya \"keyb\" value");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find({"keya", "keyb"}) == "value");
  }

  SECTION("With sample id Tech 2.5 line, with single key and quoted value, values convert correctly")
  {
    std::stringstream raw_config("keya keyb \"value\"");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find({"keya", "keyb"}) == "value");
  }

  SECTION("With sample id Tech 2.5 line, with single key and quoted value, values convert correctly")
  {
    std::stringstream raw_config("\"keya\" \"keyb\" \"value\"");

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.value().find({"keya", "keyb"}) == "value");
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

    auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.has_value() == true);

    REQUIRE(value.value().find("unbindall"sv) == "");

    REQUIRE(value.value().find({"bind", "MOUSE1"}) == "+attack");
    REQUIRE(value.value().find({"bind", "MOUSE2"}) == "+altattack");
    REQUIRE(value.value().find({"bind", "W"}) == "+forward");
    REQUIRE(value.value().find({"bind", "A"}) == "+moveleft");
    REQUIRE(value.value().find({"bind", "S"}) == "+backward");
    REQUIRE(value.value().find({"bind", "D"}) == "+moveright");

    REQUIRE(value.value().find({"set", "log_file_name"}) == "");
    REQUIRE(value.value().find({"set", "bestweap"}) == "safe");
    REQUIRE(value.value().find({"set", "name"}) == "Mohn Jullins");

    REQUIRE(value.value().find({"set", "net_socksPort"}) == "1000");
    REQUIRE(value.value().find({"set", "ai_maxcorpses"}) == "4");
    REQUIRE(value.value().find({"set", "gl_swapinterval"}) == "1");

    REQUIRE(value.value().find({"set", "gl_offsetunits"}) == "-2.0");
    REQUIRE(value.value().find({"set", "ghl_shadow_darkness"}) == "0.75");
    REQUIRE(value.value().find({"set", "vid_brightness"}) == "0.6");
  }

}
