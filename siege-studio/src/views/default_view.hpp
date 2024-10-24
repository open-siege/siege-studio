#ifndef SIEGE_DEFAULT_VIEW_HPP
#define SIEGE_DEFAULT_VIEW_HPP

#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/extension_module.hpp>
#include <utility>
#include <future>
#include <execution>

namespace siege::views
{
  using namespace std::literals;
  struct engine_info
  {
    std::wstring_view engine_name;
    std::optional<std::wstring_view> variant_of_engine_id;
    std::optional<std::wstring_view> variant_version;
  };

  using namespace std::literals;
  namespace fs = std::filesystem;

  constexpr static auto default_search_paths = std::array<std::wstring_view, 10>{ {
    L"Games"sv,
    L"GOG Games"sv,
    L"Steam\\steamapps\\common"sv,
    L"GOG Galaxy\\Games"sv,
    L"SteamLibrary"sv,
    L"ZOOM PLATFORM"sv,
    // developer/publisher specific
    L"Sierra"sv,
    L"Dynamix"sv,
    L"EA Games"sv,
    L"Activision"sv,
  } };

  constexpr static auto engines = std::array<engine_info, 18>{ {
    { L"3Space 1.0"sv, L"3space"sv, L"1.0"sv },
    { L"3Space 1.5"sv, L"3space-1.0"sv, L"1.5"sv },
    { L"3Space 2.0"sv, L"3space"sv, L"2.0"sv },
    { L"3Space 2.5"sv, L"3space-2.0"sv, L"2.5"sv },
    { L"3Space 3.0"sv, L"3space"sv, L"3.0"sv },
    { L"Darkstar"sv, L"3space-3.0"sv, L"3.5"sv },
    { L"Torque"sv, L"3space"sv, L"4.0"sv },
    { L"Wolfenstein 3D engine"sv, L"idTech"sv, L"0.5"sv },
    { L"Shadowcaster engine"sv, L"idTech"sv, L"0.5"sv },
    { L"id Tech 1.0"sv, L"idTech"sv, L"1.0"sv },
    { L"Quake engine"sv, L"idTech"sv, L"2.0"sv },
    { L"Quake 2 engine"sv, L"idTech-2.0"sv, L"2.5"sv },
    { L"Quake 2 engine (Raven branch)"sv, L"idTech-2.5"sv, L"2.5-raven"sv },
    { L"Quake 2 engine (Ritual branch)"sv, L"idTech-2.5"sv, L"2.5-ritual"sv },
    { L"id Tech 3.0"sv, L"idTech-2.5"sv, L"3.0"sv },
    { L"id Tech 3.0 (with Ubertools) (Ritual branch)"sv, L"idTech-3.0"sv, L"3.0-ritual"sv },
    { L"id Tech 3.0 (Raven Branch)"sv, L"idTech-3.0"sv, L"3.0-raven"sv },
    { L"iW Engine 1.0"sv, L"idTech-3.0"sv, L"3.0-iw"sv },
  } };

  struct game_info
  {
    std::wstring_view game_name;// Starsiege/Earthsiege 2
    std::wstring_view engine_id;// darkstar/3space2.5
    std::optional<std::wstring_view> preferered_extension;// siege-extension-starsiege
    std::array<std::wstring_view, 8> supported_file_formats;// vol, cs, dts, dml,
  };

  constexpr static auto games = std::array<game_info, 107>{ {
    { L"A-10 Tank Killer 1.5"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Nova 9: The Return Gir Draxon"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Aces of the Pacific"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Aces Over Europe"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Betrayal at Krondor"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Aces of the Deep"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Metaltech: Battledrome"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Metaltech: Earthsiege"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Command: Aces of the Deep"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Earthsiege 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Silent Thunder: A-10 Tank Killer 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Red Baron 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Pro Pilot '98"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Red Baron 3D"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Pro Pilot '99"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Kid Pilot"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Curse You! Red Baron"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Front Page Sports: Ski Racing"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"King's Quest: Mask of Eternity"sv, L"darkstar"sv, L"siege-extension-mask-of-eternity"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Driver's Education '98"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Starsiege: Tribes"sv, L"darkstar"sv, L"siege-extension-tribes"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Field & Stream: Trophy Bass 3D"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Field & Stream: Trophy Bass 4"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Driver's Education '99"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Starsiege"sv, L"darkstar"sv, L"siege-extension-starsiege"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Rise of the Dragon"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Heart of China"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"The Adventures of Willy Beamish"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Quarky & Quaysoo's Turbo Science"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"The Incredible Machine"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"The Even More Incredible Machine"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Sid & Al's Incredible Toons"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"The Incredible Machine 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"The Incredible Machine 3"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Pinball"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Trophy Bass"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Pinball: Creep Night"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Hunter Hunted"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Front page Sports: Trophy Bass 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"MissionForce: CyberStorm"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Outpost 2: Divided Destiny"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Minigolf"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Pinball: The Lost Continent"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra NASCAR Pinball"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Cyberstorm 2: Corporate Wars"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra MiniGolf Deluxe"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Radio Control Racers"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Cool Pool"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Lionel Train Town"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Pinball: Thrillride"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Lionel Train Town Deluxe"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Maximum Pool"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Return of the Incredible Machine: Contraptions"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"The Incredible Machine: Even More Contraptions"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Minigolf Maniacs"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Red Baron"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Field & Stream: Trophy Hunting 4"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Field & Stream: Trophy Hunting 5"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Tribes 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"ShadowCaster"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"CyClones"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"In Pursuit of Greed"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"World of Aden: Thunderscape"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Necrodome"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Take No Prisoners"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"MageSlayer"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Quake"sv, L"darkstar"sv, L"siege-extension-quake", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Hexen 2"sv, L"darkstar"sv, L"siege-extension-hexen-2", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Laser Arena"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"CIA Operative: Solo Missions"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Quake 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Heretic 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"SiN"sv, L"darkstar"sv, L"siege-extension-sin", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Kingpin: Life of Crim"sv, L"darkstar"sv, L"siege-extension-kingpin", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Soldier of Fortune"sv, L"darkstar"sv, L"siege-extension-soldier-of-fortune", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Daikatana"sv, L"darkstar"sv, L"siege-extension-daikatana", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Anachronox"sv, L"darkstar"sv, L"siege-extension-daikatana", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Quake 3 Arena"sv, L"darkstar"sv, L"siege-extension-quake-3", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Heavy Metal: F.A.K.K 2"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Star Trek: Voyager - Elite Force"sv, L"darkstar"sv, L"siege-extension-elite-force-sp", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Star Trek: Voyager - Elite Force - Holomatch"sv, L"darkstar"sv, L"siege-extension-elite-force-mp", std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"American McGee's Alice"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Return to Castle Wolfenstein"sv, L"darkstar"sv, L"siege-extension-return-to-castle-wolf-sp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Return to Castle Wolfenstein (Multiplayer)"sv, L"darkstar"sv, L"siege-extension-return-to-castle-wolf-mp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Medal of Honor: Allied Assault"sv, L"darkstar"sv, L"siege-extension-allied-assault"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Medal of Honor: Allied Assault - Spearhead"sv, L"darkstar"sv, L"siege-extension-allied-assault-spearhead"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Medal of Honor: Allied Assault - Breakthrough"sv, L"darkstar"sv, L"siege-extension-allied-assault-breakthrough"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Medal of Honor: Pacific Assault"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Star Wars Jedi Knight 2: Jedi Outcast"sv, L"darkstar"sv, L"siege-extension-jedi-outcast-sp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Star Wars Jedi Knight 2: Jedi Outcast (Multiplayer)"sv, L"darkstar"sv, L"siege-extension-jedi-outcast-mp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Star Wars Jedi Knight: Jedi Academy"sv, L"darkstar"sv, L"siege-extension-jedi-academy-sp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Star Wars Jedi Knight: Jedi Academy (Multiplayer)"sv, L"darkstar"sv, L"siege-extension-jedi-academy-mp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Soldier of Fortune 2: Double Helix"sv, L"darkstar"sv, L"siege-extension-soldier-of-fortune-2-sp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Soldier of Fortune 2: Double Helix (Multiplayer)"sv, L"darkstar"sv, L"siege-extension-soldier-of-fortune-2-mp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Wolfenstein: Enemy Territory"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Call of Duty"sv, L"darkstar"sv, L"siege-extension-call-of-duty-sp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Call of Duty (Multiplayer)"sv, L"darkstar"sv, L"siege-extension-call-of-duty-mp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Call of Duty: United Offensive"sv, L"darkstar"sv, L"siege-extension-call-of-duty-uo-sp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Call of Duty: United Offensive (Multiplayer)"sv, L"darkstar"sv, L"siege-extension-call-of-duty-uo-mp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Call of Duty 2"sv, L"darkstar"sv, L"siege-extension-call-of-duty-2-sp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Call of Duty 2 (Multiplayer)"sv, L"darkstar"sv, L"siege-extension-call-of-duty-2-mp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Call of Duty 4"sv, L"darkstar"sv, L"siege-extension-call-of-duty-4-sp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Call of Duty 4 (Multiplayer)"sv, L"darkstar"sv, L"siege-extension-call-of-duty-4-mp"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Iron Grip: Warlord"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Dark Salvation"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Quake Live"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Space Trader: Merchant Marine"sv, L"darkstar"sv, L"siege-extension-space-trader"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
  } };

  struct default_view final : win32::window_ref
  {
    inline static auto first_time = true;

    win32::static_control heading;//"Welcome to Siege Studio."
    win32::static_control logo;

    win32::gdi::icon logo_icon;

    std::future<void> pending_operation;
    std::map<fs::path, game_info> detected_games;
    //      std::string url = "https://github.com/open-siege/open-siege/wiki/" + extension;
    //"This particular file is not yet supported by Siege Studio.\nThough, you can still read about it on our wiki.\nClick the link below to find out more."
    default_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
    {
      logo_icon.reset((HICON)::LoadImageW(params.hInstance, L"AppIcon", IMAGE_ICON, 0, 0, 0));
    }

    auto wm_create()
    {
      win32::window_factory factory(ref());

      heading = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE });

      ::SetWindowTextW(heading, L"Welcome to Siege Studio");

      logo = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | SS_ICON | SS_REALSIZECONTROL });

      ::SendMessageW(logo, STM_SETIMAGE, IMAGE_ICON, (LPARAM)logo_icon.get());

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      pending_operation = std::async(std::launch::async, []() {
        std::set<std::wstring> search_roots;

        for (auto drive = L'C'; drive <= L'Z'; ++drive)
        {
          if (std::filesystem::exists(std::wstring(1, drive) + L":\\"))
          {
            search_roots.emplace(std::wstring(1, drive) + L":");
          }
        }

        std::wstring program_files_path(256, L'\0');
        std::wstring program_files_x86_path(256, L'\0');

        if (::SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, SHGFP_TYPE_CURRENT, program_files_path.data()) == S_OK)
        {
          program_files_path.resize(program_files_path.find(L'\0'));
          search_roots.emplace(std::move(program_files_path));
        }

        if (::SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILESX86, nullptr, SHGFP_TYPE_CURRENT, program_files_x86_path.data()) == S_OK)
        {
          program_files_x86_path.resize(program_files_x86_path.find(L'\0'));
          search_roots.emplace(std::move(program_files_x86_path));
        }

        std::set<fs::path> real_search_paths;

        for (const auto& root : search_roots)
        {
          for (const auto& search_path : default_search_paths)
          {
            auto real_search_path = std::wstring(root) + std::wstring(1, L'\\') + std::wstring(search_path);
            real_search_paths.insert(std::move(real_search_path));
          }
        }

        std::filesystem::path app_path = std::filesystem::path(win32::module_ref::current_application().GetModuleFileName()).parent_path();
        auto extensions = siege::platform::game_extension_module::load_modules(app_path);

        std::mutex game_lock;


        std::for_each(std::execution::par_unseq, real_search_paths.begin(), real_search_paths.end(), [&](const auto& real_search_path) {
          std::error_code errc{};
          if (fs::exists(real_search_path, errc))
          {
            try
            {
              for (const fs::directory_entry& dir_entry :
                fs::recursive_directory_iterator(real_search_path))
              {
                if (dir_entry.path().extension() == L".exe" || dir_entry.path().extension() == L".EXE")
                {
                  for (auto& extension : extensions)
                  {
                    if (extension.executable_is_supported(dir_entry.path().c_str()) == true)
                    {
                      OutputDebugStringW(dir_entry.path().c_str());
                      OutputDebugStringW(L"\n");
                    }
                  }
                }
              }
            }
            catch (...)
            {
            }
          }
        });
      });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MAXIMIZED || type == SIZE_RESTORED)
      {
        auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 2 };
        auto bottom_size = SIZE{ .cx = client_size.cy / 2, .cy = client_size.cy / 2 };

        heading.SetWindowPos(POINT{});
        heading.SetWindowPos(top_size);

        logo.SetWindowPos(POINT{ .x = (client_size.cx - bottom_size.cx) / 2, .y = (client_size.cy - top_size.cy) / 2 });
        logo.SetWindowPos(bottom_size);
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(heading);
        win32::apply_theme(logo);
        win32::apply_theme(*this);

        return 0;
      }

      return std::nullopt;
    }

    auto wm_command()
    {
      // wxLaunchDefaultBrowser(url);
    }
  };
}// namespace siege::views

#endif