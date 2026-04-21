#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/configuration/ini.hpp>

namespace ini = siege::configuration::common::ini;

using namespace std::literals;

TEST_CASE("When data is empty for ini nullopt returned")
{
  std::stringstream data;
  auto value = ini::load_config(data, 0);

  REQUIRE(value == std::nullopt);
}

TEST_CASE("With sample ini lines, with single group, key and value, values convert correctly")
{
  std::stringstream raw_config("[DefaultPlayer]\nName=Player");

  auto value = ini::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "DefaultPlayer"sv, "Name"sv }) == "Player");
}

TEST_CASE("With sample ini lines, with trailing end line, values convert correctly")
{
  std::stringstream raw_config("[DefaultPlayer]\nName=Player\n");

  auto value = ini::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "DefaultPlayer"sv, "Name"sv }) == "Player");
}

TEST_CASE("With sample ini lines, with multiple groups and values, values convert correctly")
{
  std::stringstream raw_config("[DefaultPlayer]\nName=Player\nTeam=255\n"
	  "[Engine.Input]\n"
	  "Aliases[0]=(Command=\"Button bFire | Fire\",Alias=Fire)\n"
	  "LeftMouse=Fire\nRightMouse=AltFire\nCtrl=Duck");

  auto value = ini::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "DefaultPlayer"sv, "Name"sv }) == "Player");
  REQUIRE(value.value().find({ "DefaultPlayer"sv, "Team"sv }) == "255");
  REQUIRE(value.value().find({ "Engine.Input"sv, "Aliases[0]"sv }) == "(Command=\"Button bFire | Fire\",Alias=Fire)");
  REQUIRE(value.value().find({ "Engine.Input"sv, "LeftMouse"sv }) == "Fire");
  REQUIRE(value.value().find({ "Engine.Input"sv, "RightMouse"sv }) == "AltFire");
  REQUIRE(value.value().find({ "Engine.Input"sv, "Ctrl"sv }) == "Duck");
}
