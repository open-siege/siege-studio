#include <catch2/catch_test_macros.hpp>
import std;
import siege.configuration.ini;

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
  std::stringstream raw_config(
    "[DefaultPlayer]\nName=Player\nTeam=255\n"
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

TEST_CASE("Ini save round-trips Unreal-style aliases and preserves comments")
{
  std::string source =
    "; player prefs\r\n"
    "[DefaultPlayer]\r\n"
    "Name=Player\r\n"
    "\r\n"
    "[Engine.Input]\r\n"
    "Aliases[0]=(Command=\"Button bFire | Fire\",Alias=Fire)\r\n"
    "LeftMouse=Fire\r\n"
    "Joy1=Fire\r\n";

  std::stringstream raw_config(source);
  auto value = ini::load_config(raw_config, source.size());
  REQUIRE(value.has_value());

  value->emplace({ "Engine.Input"sv, "Aliases[0]"sv },
    siege::configuration::key_type{ "(Command=\"Button bFire | Fire\",Alias=Fire)"sv });
  value->emplace({ "Engine.Input"sv, "Joy2"sv }, siege::configuration::key_type{ "Jump"sv });
  value->emplace({ "DefaultPlayer"sv, "Name"sv }, siege::configuration::key_type{ "SiegePlayer"sv });

  std::stringstream saved;
  value->save(saved);

  auto reloaded = ini::load_config(saved, saved.str().size());
  REQUIRE(reloaded.has_value());
  REQUIRE(reloaded->find({ "DefaultPlayer"sv, "Name"sv }) == "SiegePlayer");
  REQUIRE(reloaded->find({ "Engine.Input"sv, "Aliases[0]"sv }) == "(Command=\"Button bFire | Fire\",Alias=Fire)");
  REQUIRE(reloaded->find({ "Engine.Input"sv, "LeftMouse"sv }) == "Fire");
  REQUIRE(reloaded->find({ "Engine.Input"sv, "Joy1"sv }) == "Fire");
  REQUIRE(reloaded->find({ "Engine.Input"sv, "Joy2"sv }) == "Jump");
  REQUIRE(saved.str().contains("SiegePlayer"));
  REQUIRE(saved.str().contains("Joy2=Jump"));
}
