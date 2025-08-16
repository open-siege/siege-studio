#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/configuration/unreal.hpp>

namespace unreal = siege::configuration::unreal;

using namespace std::literals;

TEST_CASE("Parsing of an unreal config file, with sample unreal data, values convert correctly", "[cfg.unreal]")
{
  std::stringstream raw_config;
  raw_config << "[Engine.Input]"
             << "\r\n";

  raw_config << "Aliases[0]=(Command=\"Button bFire | Fire\",Alias=Fire)"
             << "\r\n";
  raw_config << "LeftMouse="
             << "\r\n";
  raw_config << "Enter=InventoryActivate"
             << "\r\n";
  raw_config << "1=SwitchWeapon 1"
             << "\r\n";
  raw_config << "Joy1=Fire"
             << "\r\n";
  raw_config << "Joy2=Jump"
             << "\r\n";

  raw_config << "JoyX=Axis astrafe speed=2"
             << "\r\n";
  raw_config << "JoyY=Axis aBaseY speed=2"
             << "\r\n";

  auto value = unreal::unreal_1::load_config(raw_config, raw_config.tellp());

  REQUIRE(value.has_value() == true);

  REQUIRE(value.value().keys().size() == 8);

  REQUIRE(value.value().find({ "Engine.Input", "Aliases[0]" }) == "(Command=\"Button bFire | Fire\",Alias=Fire)");
  REQUIRE(value.value().find({ "Engine.Input", "LeftMouse" }) == "");
  REQUIRE(value.value().find({ "Engine.Input", "Enter" }) == "InventoryActivate");
  REQUIRE(value.value().find({ "Engine.Input", "1" }) == "SwitchWeapon 1");
  REQUIRE(value.value().find({ "Engine.Input", "Joy1" }) == "Fire");
  REQUIRE(value.value().find({ "Engine.Input", "Joy2" }) == "Jump");

  REQUIRE(value.value().find({ "Engine.Input", "JoyX" }) == "Axis astrafe speed=2");
  REQUIRE(value.value().find({ "Engine.Input", "JoyY" }) == "Axis aBaseY speed=2");
}
