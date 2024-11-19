#ifndef SIEGE_DEFAULT_VIEW_HPP
#define SIEGE_DEFAULT_VIEW_HPP

#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/shell.hpp>
#include <siege/platform/extension_module.hpp>
#include <utility>
#include <future>
#include <execution>
#include <fstream>

namespace siege::views
{
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

  struct engine_info
  {
    std::wstring_view engine_name;
    std::optional<std::wstring_view> engine_id;
    std::optional<std::wstring_view> variant_of_engine_id;
  };

  constexpr static auto engines = std::array<engine_info, 26>{ {
    { L"id Tech 3.0 (Raven Branch)"sv, L"id_tech-3.0-raven"sv, L"id_tech-3.0"sv },
    { L"id Tech 2.5 (Raven branch)"sv, L"id_tech-2.5-raven"sv, L"id_tech-2.5"sv },
    { L"id Tech 3.0 (with Ubertools) (Ritual branch)"sv, L"id_tech-3.0-ritual"sv, L"id_tech-3.0"sv },
    { L"id Tech 2.5 (Ritual branch)"sv, L"id_tech-2.5-ritual"sv, L"id_tech-2.5"sv },
    { L"id Tech 3.0"sv, L"id_tech-3.0"sv, L"id_tech-2.5"sv },
    { L"id Tech 2.5"sv, L"id_tech-2.5"sv, L"id_tech-2.0"sv },
    { L"id Tech 2.0"sv, L"id_tech-2.0"sv, std::nullopt },
    { L"isiMotor 2.0"sv, L"isi-2.0"sv },
    { L"Torque"sv, L"torque"sv, L"darkstar"sv },
    { L"Darkstar"sv, L"darkstar"sv, L"3space-3.0"sv },
    { L"3Space 3.0"sv, L"3space-3.0"sv, L"3space-2.5"sv },
    { L"3Space 2.5"sv, L"3space-2.5"sv, L"3space-2.0"sv },
    { L"3Space 2.0"sv, L"3space-2.0"sv, L"3space-1.5"sv },
    { L"3Space 1.5"sv, L"3space-1.5"sv, L"3space-1.0"sv },
    { L"3Space 1.0"sv, L"3space-1.0"sv, std::nullopt },
    { L"Dynamix Game Development System"sv, L"dgds"sv, std::nullopt },
    { L"isiMotor 1.5"sv, L"isi-1.0"sv },
    { L"isiMotor 1.0"sv, L"isi-1.0"sv },
    { L"iW Engine 3.0"sv, L"iw-3.0"sv, L"iw-2.0"sv },
    { L"iW Engine 2.0"sv, L"iw-2.0"sv, std::nullopt },
    { L"iW Engine 1.0"sv, L"id_tech-3.0-iw"sv, L"id_tech-3.0"sv },
    { L"Vampire"sv, L"vampire"sv, std::nullopt },
    { L"id Tech 1.0"sv, L"id_tech-1.0"sv, L"id_tech-raven"sv },
    { L"Shadowcaster engine"sv, L"id_tech-0.5-raven"sv, L"id_tech-0.5"sv },
    { L"Wolfenstein 3D engine"sv, L"id_tech-0.5"sv, std::nullopt },
    { L"Unknown engine"sv, L"unknown"sv, std::nullopt },
  } };

  struct game_info
  {
    std::wstring_view game_name;
    std::wstring_view engine_id;
    std::optional<std::wstring_view> preferered_extension;
    std::array<std::wstring_view, 8> supported_file_formats;
  };

  constexpr static auto games = std::array<game_info, 130>{ {
    { L"Red Baron"sv, L"3space"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rmf"sv } } },
    { L"A-10 Tank Killer 1.5"sv, L"3space-1.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rmf"sv } } },
    { L"Nova 9: The Return Gir Draxon"sv, L"3space-1.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rmf"sv } } },
    { L"Aces of the Pacific"sv, L"3space-1.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".dyn"sv } } },
    { L"Aces Over Europe"sv, L"3space-1.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".dyn"sv } } },
    { L"Betrayal at Krondor"sv, L"3space-1.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rmf"sv } } },
    { L"Aces of the Deep"sv, L"3space-2.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dyn"sv, L".dts"sv } } },
    { L"Metaltech: Battledrome"sv, L"3space-2.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv } } },
    { L"Metaltech: Earthsiege"sv, L"3space-2.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv } } },
    { L"Command: Aces of the Deep"sv, L"3space-2.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dyn"sv, L".dts"sv } } },
    { L"Earthsiege 2"sv, L"3space-2.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv } } },
    { L"Silent Thunder: A-10 Tank Killer 2"sv, L"3space-2.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dt2"sv, L".dml"sv, L".pal"sv, L".ppl" } } },
    { L"Red Baron 2"sv, L"3space-2.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Pro Pilot '98"sv, L"3space-2.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv } } },
    { L"Red Baron 3D"sv, L"3space-2.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv } } },
    { L"Pro Pilot '99"sv, L"3space-2.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv } } },
    { L"Kid Pilot"sv, L"3space-2.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv } } },
    { L"Curse You! Red Baron"sv, L"3space-2.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Front Page Sports: Ski Racing"sv, L"darkstar"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".tbv"sv, L".dts"sv, L".dml"sv, L".pal"sv, L".ppl"sv, L".bmp"sv, L".pba"sv } } },
    { L"King's Quest: Mask of Eternity"sv, L"darkstar"sv, L"siege-extension-mask-of-eternity"sv, std::array<std::wstring_view, 8>{ { L".dml"sv, L".dml"sv, L".pal"sv, L".ppl", L".bmp", L".pba" } } },
    { L"Driver's Education '98"sv, L"3space-3.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".pal"sv, L".ppl", L".bmp", L".pba" } } },
    { L"Starsiege"sv, L"darkstar"sv, L"siege-extension-starsiege"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".pal"sv, L".ppl", L".bmp", L".pba", L".cs" } } },
    { L"Starsiege: Tribes"sv, L"darkstar"sv, L"siege-extension-tribes"sv, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".pal"sv, L".ppl", L".bmp", L".pba", L".cs" } } },
    { L"Field & Stream: Trophy Bass 3D"sv, L"3space-3.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Field & Stream: Trophy Bass 4"sv, L"3space-3.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Driver's Education '99"sv, L"3space-3.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Rise of the Dragon"sv, L"dgds"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"Heart of China"sv, L"dgds"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"The Adventures of Willy Beamish"sv, L"dgds"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rmf" } } },
    { L"Quarky & Quaysoo's Turbo Science"sv, L"dgds"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rmf" } } },
    { L"The Incredible Machine"sv, L"dgds"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rmf" } } },
    { L"The Even More Incredible Machine"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rmf" } } },
    { L"Sid & Al's Incredible Toons"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"The Incredible Machine 2"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"The Incredible Machine 3"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"3-D Ultra Pinball"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"Trophy Bass"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"3-D Ultra Pinball: Creep Night"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"Hunter Hunted"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"Front page Sports: Trophy Bass 2"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"MissionForce: CyberStorm"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rbx" } } },
    { L"Outpost 2: Divided Destiny"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".clm"sv } } },
    { L"3-D Ultra Minigolf"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"3-D Ultra Pinball: The Lost Continent"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"3-D Ultra NASCAR Pinball"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"Cyberstorm 2: Corporate Wars"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"3-D Ultra MiniGolf Deluxe"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".rbx"sv } } },
    { L"3-D Ultra Radio Control Racers"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv, L".dts"sv, L".dml"sv, L".cs"sv } } },
    { L"3-D Ultra Cool Pool"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"3-D Ultra Lionel Train Town"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"3-D Ultra Pinball: Thrillride"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"3-D Ultra Lionel Train Town Deluxe"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"Maximum Pool"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"Return of the Incredible Machine: Contraptions"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"The Incredible Machine: Even More Contraptions"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".tbv" } } },
    { L"Minigolf Maniacs"sv, L"unknown"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vol"sv } } },
    { L"Field & Stream: Trophy Hunting 4"sv, L"torque"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".zip"sv } } },
    { L"Field & Stream: Trophy Hunting 5"sv, L"torque"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".zip"sv } } },
    { L"Tribes 2"sv, L"torque"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vl2"sv } } },
    { L"ShadowCaster"sv, L"idtech-0.5-raven"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".dat"sv } } },
    { L"CyClones"sv, L"idtech-0.5-raven"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".dat"sv } } },
    { L"In Pursuit of Greed"sv, L"idtech-0.5-raven"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".dat"sv } } },
    { L"World of Aden: Thunderscape"sv, L"idtech-0.5-raven"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".dat"sv } } },
    { L"Necrodome"sv, L"idtech-0.5-raven"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pod"sv } } },
    { L"Take No Prisoners"sv, L"vampire"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vpk"sv } } },
    { L"MageSlayer"sv, L"vampire"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".vpk"sv } } },
    { L"Quake"sv, L"id_tech-2.0"sv, L"siege-extension-quake", std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Hexen 2"sv, L"id_tech-2.0"sv, L"siege-extension-hexen-2", std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Laser Arena"sv, L"id_tech-2.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"CIA Operative: Solo Missions"sv, L"id_tech-2.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Quake 2"sv, L"id_tech-2.5"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Heretic 2"sv, L"id_tech-2.5-raven"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"SiN"sv, L"id_tech-2.5-ritual"sv, L"siege-extension-sin", std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Kingpin: Life of Crime"sv, L"id_tech-2.5"sv, L"siege-extension-kingpin", std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Soldier of Fortune"sv, L"id_tech-2.5-raven"sv, L"siege-extension-soldier-of-fortune", std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Daikatana"sv, L"id_tech-2.5"sv, L"siege-extension-daikatana", std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Anachronox"sv, L"id_tech-2.5"sv, L"siege-extension-daikatana", std::array<std::wstring_view, 8>{ { L".pak" } } },
    { L"Quake 3 Arena"sv, L"id_tech-3.0"sv, L"siege-extension-quake-3", std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Heavy Metal: F.A.K.K 2"sv, L"id_tech-3.0-ritual"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Star Trek: Voyager - Elite Force"sv, L"id_tech-3.0-raven"sv, L"siege-extension-elite-force-sp", std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Star Trek: Voyager - Elite Force - Holomatch"sv, L"id_tech-3.0-raven"sv, L"siege-extension-elite-force-mp", std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"American McGee's Alice"sv, L"id_tech-3.0-ritual"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Return to Castle Wolfenstein"sv, L"id_tech-3.0"sv, L"siege-extension-return-to-castle-wolf-sp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Return to Castle Wolfenstein (Multiplayer)"sv, L"id_tech-3.0"sv, L"siege-extension-return-to-castle-wolf-mp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Medal of Honor: Allied Assault"sv, L"id_tech-3.0-ritual"sv, L"siege-extension-allied-assault"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Medal of Honor: Allied Assault - Spearhead"sv, L"id_tech-3.0-ritual"sv, L"siege-extension-allied-assault-spearhead"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Medal of Honor: Allied Assault - Breakthrough"sv, L"id_tech-3.0-ritual"sv, L"siege-extension-allied-assault-breakthrough"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Medal of Honor: Pacific Assault"sv, L"id_tech-3.0-ritual"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Star Wars Jedi Knight 2: Jedi Outcast"sv, L"id_tech-3.0-raven"sv, L"siege-extension-jedi-outcast-sp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Star Wars Jedi Knight 2: Jedi Outcast (Multiplayer)"sv, L"id_tech-3.0-raven"sv, L"siege-extension-jedi-outcast-mp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Star Wars Jedi Knight: Jedi Academy"sv, L"id_tech-3.0-raven"sv, L"siege-extension-jedi-academy-sp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Star Wars Jedi Knight: Jedi Academy (Multiplayer)"sv, L"id_tech-3.0-raven"sv, L"siege-extension-jedi-academy-mp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Soldier of Fortune 2: Double Helix"sv, L"id_tech-3.0-raven"sv, L"siege-extension-soldier-of-fortune-2-sp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Soldier of Fortune 2: Double Helix (Multiplayer)"sv, L"id_tech-3.0-raven"sv, L"siege-extension-soldier-of-fortune-2-mp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Wolfenstein: Enemy Territory"sv, L"id_tech-3.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Call of Duty"sv, L"id_tech-3.0-iw"sv, L"siege-extension-call-of-duty-sp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Call of Duty (Multiplayer)"sv, L"id_tech-3.0-iw"sv, L"siege-extension-call-of-duty-mp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Call of Duty: United Offensive"sv, L"id_tech-3.0-iw"sv, L"siege-extension-call-of-duty-uo-sp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Call of Duty: United Offensive (Multiplayer)"sv, L"id_tech-3.0-iw"sv, L"siege-extension-call-of-duty-uo-mp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Call of Duty 2"sv, L"iw-2.0"sv, L"siege-extension-call-of-duty-2-sp"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Call of Duty 2 (Multiplayer)"sv, L"iw-2.0"sv, L"siege-extension-call-of-duty-2-mp"sv, std::array<std::wstring_view, 8>{ { L".zip" } } },
    { L"Call of Duty 4"sv, L"iw-3.0"sv, L"siege-extension-call-of-duty-4-sp"sv, std::array<std::wstring_view, 8>{ { L".zip" } } },
    { L"Call of Duty 4 (Multiplayer)"sv, L"iw-3.0"sv, L"siege-extension-call-of-duty-4-mp"sv, std::array<std::wstring_view, 8>{ { L".zip" } } },
    { L"Iron Grip: Warlord"sv, L"id_tech-3.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Dark Salvation"sv, L"id_tech-3.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Quake Live"sv, L"id_tech-3.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Space Trader: Merchant Marine"sv, L"id_tech-3.0"sv, L"siege-extension-space-trader"sv, std::array<std::wstring_view, 8>{ { L".pk3" } } },
    { L"Zone Raiders"sv, L"isi-1.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"Sports Car GT"sv, L"isi-1.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"F1 2000"sv, L"isi-1.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"F1 Championship Season 2000"sv, L"isi-1.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"F1 2001"sv, L"isi-1.0"sv, std::nullopt, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"F1 2002"sv, L"isi-1.5"sv, L"siege-extension-f1-2002", std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"F1 Career Challenge"sv, L"isi-1.5"sv, L"siege-extension-f1-career-challenge", std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"NASCAR Thunder 2003"sv, L"isi-1.5"sv, L"siege-extension-nascar-thunder-2003", std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"NASCAR Thunder 2004"sv, L"isi-1.5"sv, L"siege-extension-nascar-thunder-2003", std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"NASCAR SimRacing"sv, L"isi-1.5"sv, L"siege-extension-nascar-sim-racing", std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"GTR - FIA GT Racing Game"sv, L"isi-2.0"sv, L"siege-extension-gtr-fia"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"rFactor"sv, L"isi-2.0"sv, L"siege-extension-rfactor"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"GT Legends"sv, L"isi-2.0"sv, L"siege-extension-gt-legends"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"GTR 2 - FIA GT Racing Game"sv, L"isi-2.0"sv, L"siege-extension-gtr-fia-2"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"BMW M3 Challenge"sv, L"isi-2.0"sv, L"siege-extension-bmw-m3"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"RACE - The WTCC Game"sv, L"isi-2.0 "sv, L"siege-extension-race"sv, std::array<std::wstring_view, 8>{ { L".plr " } } },
    { L"RACE 07 - Official WTCC Game"sv, L"isi-2.0"sv, L"siege-extension-race-07"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"Lexus ISF Track Time"sv, L"isi-2.0"sv, L"siege-extension-lexus"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"Volvo - The Game"sv, L"isi-2.0"sv, L"siege-extension-volvo"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"Superleague Formula 2009: The Game"sv, L"isi-2.0"sv, L"siege-extension-superleague"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"Stock Car Extreme"sv, L"isi-2.0"sv, L"siege-extension-stock-car-extreme"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"Copa Petrobras de Marcas"sv, L"isi-2.0"sv, L"siege-extension-marcas"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
    { L"Automobilista"sv, L"isi-2.0"sv, L"siege-extension-automobilista"sv, std::array<std::wstring_view, 8>{ { L".plr" } } },
  } };

  struct default_view final : win32::window_ref
  {
    inline static auto first_time = true;
    std::wofstream log = std::wofstream("siege-studio-log.txt", std::ios::trunc);

    win32::static_control heading;//"Welcome to Siege Studio."
    win32::static_control logo;

    win32::list_view supported_games_by_engine;

    // TODO finish implementing this
    win32::list_view supported_games_by_file_type;

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
      log << L"Creating default view\n";
      win32::window_factory factory(ref());

      heading = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE });

      ::SetWindowTextW(heading, L"Welcome to Siege Studio");

      logo = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | SS_ICON | SS_REALSIZECONTROL });

      ::SendMessageW(logo, STM_SETIMAGE, IMAGE_ICON, (LPARAM)logo_icon.get());

      supported_games_by_engine = *factory.CreateWindowExW<win32::list_view>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | LVS_REPORT });
      supported_games_by_engine.bind_nm_dbl_click([this](auto c, const auto& n) {
        this->supported_games_nm_dbl_click(std::move(c), n);
      });

      supported_games_by_engine.InsertColumn(-1, LVCOLUMNW{
                                                   .pszText = const_cast<wchar_t*>(L""),
                                                 });

      supported_games_by_engine.InsertColumn(-1, LVCOLUMNW{
                                                   .pszText = const_cast<wchar_t*>(L"Detected Path"),
                                                 });

      ListView_SetView(supported_games_by_engine, LV_VIEW_TILE);

      supported_games_by_engine.EnableGroupView(true);

      int id = 1;

      for (const auto& engine : engines)
      {
        supported_games_by_engine.InsertGroup(-1, LVGROUP{
                                                    .pszHeader = const_cast<wchar_t*>(engine.engine_name.data()),
                                                    .iGroupId = id++,
                                                    .state = LVGS_COLLAPSIBLE,
                                                  });
      }

      LVTILEVIEWINFO tileViewInfo{
        .cbSize = sizeof(tileViewInfo),
        .dwMask = LVTVIM_COLUMNS,
        .dwFlags = LVTVIF_AUTOSIZE,
        .cLines = 1
      };

      ListView_SetTileViewInfo(supported_games_by_engine, &tileViewInfo);


      for (const auto& game : games)
      {
        win32::list_view_item game_item(std::wstring(game.game_name));

        auto engine_iter = std::find_if(engines.begin(), engines.end(), [&](auto& engine) {
          return engine.engine_id == game.engine_id;
        });

        if (engine_iter == engines.end())
        {
          continue;
        }

        auto engine_group_id = std::distance(engines.begin(), engine_iter) + 1;

        game_item.iGroupId = engine_group_id;

        if (game.preferered_extension)
        {
          game_item.lParam = (LPARAM)game.preferered_extension->data();
        }

        game_item.sub_items.emplace_back(L"Not detected");
        auto new_item = supported_games_by_engine.InsertRow(game_item);

        UINT columns[1] = { 1 };
        int formats[1] = { LVCFMT_LEFT };
        LVTILEINFO item_info{ .cbSize = sizeof(LVTILEINFO), .iItem = (int)new_item, .cColumns = 1, .puColumns = columns, .piColFmt = formats };

        ListView_SetTileInfo(supported_games_by_engine, &item_info);
      }

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });


      pending_operation = std::async(std::launch::async, [this]() {
        log << L"Detecting games\n";
        std::set<std::wstring> search_roots;

        std::error_code errc{};
        for (auto drive = L'C'; drive <= L'Z'; ++drive)
        {
          log << L"Checking for drive: " << drive << '\n';
          if (std::filesystem::exists(std::wstring(1, drive) + L":\\", errc))
          {
            log << L"Found drive: " << drive << '\n';
            search_roots.emplace(std::wstring(1, drive) + L":");
          }
        }

        std::wstring program_files_path(256, L'\0');
        std::wstring program_files_x86_path(256, L'\0');

        log << L"Finding program files directories" << '\n';
        if (::SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, SHGFP_TYPE_CURRENT, program_files_path.data()) == S_OK)
        {
          program_files_path.resize(program_files_path.find(L'\0'));
          log << L"Found program files: " << program_files_path << '\n';
          search_roots.emplace(std::move(program_files_path));
        }

        if (::SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILESX86, nullptr, SHGFP_TYPE_CURRENT, program_files_x86_path.data()) == S_OK)
        {
          log << L"Found secondary program files: " << program_files_path << '\n';
          program_files_x86_path.resize(program_files_x86_path.find(L'\0'));
          search_roots.emplace(std::move(program_files_x86_path));
        }

        std::set<fs::path> real_search_paths;

        for (const auto& root : search_roots)
        {
          for (const auto& search_path : default_search_paths)
          {
            auto real_search_path = std::wstring(root) + std::wstring(1, L'\\') + std::wstring(search_path);
            log << L"Adding search path " << real_search_path << '\n';
            real_search_paths.insert(std::move(real_search_path));
          }

          for (const auto& game : games)
          {
            auto real_search_path = std::wstring(root) + std::wstring(1, L'\\') + std::wstring(game.game_name);
            log << L"Adding game search path " << real_search_path << '\n';
            real_search_paths.insert(std::move(real_search_path));
          }
        }

        std::filesystem::path app_path = std::filesystem::path(win32::module_ref::current_application().GetModuleFileName()).parent_path();
        auto extensions = siege::platform::game_extension_module::load_modules(app_path);

        std::mutex game_lock;

        std::for_each(std::execution::par_unseq, real_search_paths.begin(), real_search_paths.end(), [&](const auto& real_search_path) {
          std::error_code errc{};
          log << (L"Checking for " + real_search_path.wstring() + L'\n');
          if (fs::exists(real_search_path, errc))
          {
            try
            {
              log << (L"Found " + real_search_path.wstring() + L'\n');
              log << (L"Searching for exes" + '\n');
              for (const fs::directory_entry& dir_entry :
                fs::recursive_directory_iterator(real_search_path))
              {
                if (dir_entry.path().extension() == L".exe" || dir_entry.path().extension() == L".EXE")
                {
                  log << (L"Checking " + dir_entry.path().wstring() + L'\n');
                  for (auto& extension : extensions)
                  {
                    log << (L"Checking " + dir_entry.path().wstring() + L" with " + extension.GetModuleFileName() + L'\n');
                    if (extension.executable_is_supported(dir_entry.path().c_str()) == true)
                    {
                      auto extension_name = fs::path(extension.GetModuleFileName()).stem();

                      auto game_iter = std::find_if(games.begin(), games.end(), [&](const auto& game) {
                        return game.preferered_extension == extension_name;
                      });

                      if (game_iter == games.end())
                      {
                        break;
                      }

                      LVFINDINFOW find_info{
                        .flags = LVFI_PARAM,
                        .lParam = (LPARAM)game_iter->preferered_extension->data()
                      };

                      auto item = ListView_FindItem(this->supported_games_by_engine, -1, &find_info);

                      if (item != -1)
                      {
                        const std::lock_guard<std::mutex> lock(game_lock);
                        std::wstring final_data = dir_entry.path().wstring();
                        ListView_SetItemText(this->supported_games_by_engine, item, 1, final_data.data());
                      }
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

    void supported_games_nm_dbl_click(win32::list_view, const NMITEMACTIVATE& message)
    {
      auto root = this->GetAncestor(GA_ROOT);

      if (root)
      {
        std::array<wchar_t, 256> temp{};

        ListView_GetItemText(supported_games_by_engine, message.iItem, 1, temp.data(), 256);
        if (temp[0] != L'\0')
        {
          fs::path file_path = temp.data();

          if (!fs::exists(file_path))
          {
            return;
          }

          win32::file file_to_read(file_path, GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

          auto mapping = file_to_read.CreateFileMapping(std::nullopt, PAGE_READONLY, {}, L"");

          std::size_t size = (std::size_t)file_to_read.GetFileSizeEx().value_or(LARGE_INTEGER{}).QuadPart;

          auto view = mapping->MapViewOfFile(FILE_MAP_READ, size);

          fs::current_path(file_path.parent_path());

          root->SetPropW(L"FilePath", temp.data());
          root->CopyData(*this, COPYDATASTRUCT{ .cbData = DWORD(size), .lpData = view.get() });
          root->RemovePropW(L"FilePath");
        }
      }
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MAXIMIZED || type == SIZE_RESTORED)
      {
        supported_games_by_engine.SetWindowPos(POINT{});
        supported_games_by_engine.SetWindowPos(client_size);
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(supported_games_by_engine);
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
