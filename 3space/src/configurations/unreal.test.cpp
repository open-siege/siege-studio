#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/id_tech.hpp"

namespace darkstar = studio::resources::vol::darkstar;

/*
[DefaultPlayer]
Name=Player
Class=UPak.UPakFemaleOne
Skin=Female1Skins.Gina
Team=255

[Engine.Input]
Aliases[0]=(Command="Button bFire | Fire",Alias=Fire)
Aliases[1]=(Command="Button bAltFire | AltFire",Alias=AltFire)
Aliases[2]=(Command="Axis aBaseY  Speed=+300.0",Alias=MoveForward)
Aliases[3]=(Command="Axis aBaseY  Speed=-300.0",Alias=MoveBackward)
Aliases[4]=(Command="Axis aBaseX Speed=-150.0",Alias=TurnLeft)
Aliases[5]=(Command="Axis aBaseX  Speed=+150.0",Alias=TurnRight)
Aliases[6]=(Command="Axis aStrafe Speed=-300.0",Alias=StrafeLeft)
Aliases[7]=(Command="Axis aStrafe Speed=+300.0",Alias=StrafeRight)
Aliases[8]=(Command="Jump | Axis aUp Speed=+300.0",Alias=Jump)
Aliases[9]=(Command="Button bDuck | Axis aUp Speed=-300.0",Alias=Duck)
Aliases[10]=(Command="Button bLook",Alias=Look)
Aliases[11]=(Command="Toggle bLook",Alias=LookToggle)
Aliases[12]=(Command="ActivateItem",Alias=InventoryActivate)
Aliases[13]=(Command="NextItem",Alias=InventoryNext)
Aliases[14]=(Command="PrevItem",Alias=InventoryPrevious)
Aliases[15]=(Command="Axis aLookUp Speed=+100.0",Alias=LookUp)
Aliases[16]=(Command="Axis aLookUp Speed=-100.0",Alias=LookDown)
Aliases[17]=(Command="Button bSnapLevel",Alias=CenterView)
Aliases[18]=(Command="Button bRun",Alias=Walking)
Aliases[19]=(Command="Button bStrafe",Alias=Strafe)
Aliases[20]=(Command="NextWeapon",Alias=NextWeapon)
Aliases[21]=(Command="ActivateTranslator",Alias=ActivateTranslator)
Aliases[22]=(Command="ActivateHint",Alias=ActivateHint)
Aliases[23]=(Command="Button bFreeLook",Alias=FreeLook)
Aliases[24]=(Command="ViewClass Pawn",Alias=ViewTeam)
Aliases[25]=(Command="",Alias=None)
Aliases[26]=(Command="",Alias=None)
Aliases[27]=(Command="",Alias=None)
Aliases[28]=(Command="",Alias=None)
Aliases[29]=(Command="",Alias=None)
Aliases[30]=(Command="",Alias=None)
Aliases[31]=(Command="",Alias=None)
Aliases[32]=(Command="",Alias=None)
Aliases[33]=(Command="",Alias=None)
Aliases[34]=(Command="",Alias=None)
Aliases[35]=(Command="",Alias=None)
Aliases[36]=(Command="",Alias=None)
Aliases[37]=(Command="",Alias=None)
Aliases[38]=(Command="",Alias=None)
Aliases[39]=(Command="",Alias=None)
LeftMouse=Fire
RightMouse=Jump
MiddleMouse=AltFire
Tab=Type
Enter=InventoryActivate
Shift=Walking
Ctrl=Duck
*/

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
    const auto* test_data = "[Engine.Input]\r\nLeftMouse=Fire\r\nRightMouse=Jump";

    std::stringstream data;
    auto value = id_tech::id_tech_1::load_config(data, 0);

    REQUIRE(value.find_to_string("Engine.Input/LeftMouse") == "pause");
    REQUIRE(value.find_to_string("Engine.Input/RightMouse") == 1);
  }

  SECTION("With sample data type is correct")
  {
    const auto* test_data = "key_left\t\t75\r\nkey_up\t\t72";

    std::stringstream data;
    auto value = id_tech::id_tech_1::load_config(data, 0);

    REQUIRE(value.find_to_string("__ConfigType") == "id_tech_2");
  }
}
