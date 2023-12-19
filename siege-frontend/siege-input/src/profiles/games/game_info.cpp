#include <array>
#include <algorithm>
#include <vector>
#include <string_view>
#include <cstdlib>
#include <deque>
#include <iostream>
#include "game_info.hpp"

namespace siege
{
    using namespace std::literals;
    std::vector<game_info> get_id_tech_games();
    std::vector<game_info> get_krass_games();
    std::vector<game_info> get_serious_games();
    std::vector<game_info> get_unreal_games();
    std::vector<game_info> get_dosbox_games();
    std::vector<game_info> get_star_wars_games();

    std::vector<game_info> get_supported_games()
    {
        auto id_tech_games = get_id_tech_games();
        auto krass_games = get_krass_games();
        auto serious_games = get_serious_games();
        auto unreal_games = get_unreal_games();
        auto dosbox_games = get_dosbox_games();
        auto star_wars_games = get_star_wars_games();

        std::vector<game_info> all_games;

        all_games.reserve(id_tech_games.size() + 
            krass_games.size() + 
            serious_games.size() +
            unreal_games.size() +
            dosbox_games.size() +
            star_wars_games.size());

        auto move_games = [&](auto& games) {
            std::transform(games.begin(), games.end(), std::back_inserter(all_games), [](auto& game) {
            return std::move(game);
         });
        };

        move_games(id_tech_games);
        move_games(krass_games);
        move_games(serious_games);
        move_games(unreal_games);
        move_games(dosbox_games);
        move_games(star_wars_games);       

        return all_games;
    }

    joystick_info add_unreal_input_metadata(joystick_info);
    joystick_info add_quake_1_input_metadata(joystick_info);
    joystick_info add_quake_2_input_metadata(joystick_info);
    joystick_info add_quake_3_input_metadata(joystick_info);

    joystick_info add_quake_1_default_actions(joystick_info);
    joystick_info add_quake_2_default_actions(joystick_info);
    joystick_info add_quake_3_default_actions(joystick_info);
    joystick_info add_hexen_2_default_actions(joystick_info);
    joystick_info add_kingpin_default_actions(joystick_info);

    joystick_info add_battlemetal_default_actions(joystick_info);
    joystick_info add_heretic_2_default_actions(joystick_info);
    joystick_info add_sin_default_actions(joystick_info);
    joystick_info add_kingpin_default_actions(joystick_info);
    joystick_info add_daikatana_default_actions(joystick_info);
    joystick_info add_action_quake_2_default_actions(joystick_info);
    joystick_info add_soldier_of_fortune_default_actions(joystick_info);
    joystick_info add_half_life_default_actions(joystick_info);
    joystick_info add_iron_grip_default_actions(joystick_info);
    joystick_info add_elite_force_default_actions(joystick_info);
    joystick_info add_elite_force_2_default_actions(joystick_info);
    joystick_info add_jedi_outcast_default_actions(joystick_info);
    joystick_info add_jedi_academy_default_actions(joystick_info);
    joystick_info add_castle_wolfenstein_default_actions(joystick_info);
    joystick_info add_soldier_of_fortune_2_default_actions(joystick_info);
    joystick_info add_medal_of_honor_default_actions(joystick_info);
    joystick_info add_call_of_duty_default_action(joystick_info);
    joystick_info add_space_trader_default_actions(joystick_info);

    using config_pair = std::pair<joystick_info(&)(joystick_info), std::string_view>;


    std::string_view get_config_dir(const game_info& info)
    {
        constexpr static auto config_dirs = std::array<config_pair, 9> {{
            config_pair(add_unreal_input_metadata, "System"sv),
            config_pair(add_quake_1_input_metadata, "base"sv),
            config_pair(add_quake_2_input_metadata, "base"sv),
            config_pair(add_quake_3_input_metadata, "base"sv),
            config_pair(add_quake_1_default_actions, "Id1"sv),
            config_pair(add_quake_2_default_actions, "baseq2"sv),
            config_pair(add_quake_3_default_actions, "baseq3"sv),
            config_pair(add_hexen_2_default_actions, "data1"sv),
            config_pair(add_kingpin_default_actions, "main"sv),
            config_pair(add_daikatana_default_actions, "data"sv),
            config_pair(add_elite_force_default_actions, "BaseEF"sv),
            config_pair(add_medal_of_honor_default_actions, "main"sv),
            config_pair(add_jedi_outcast_default_actions, "GameData/base"sv),
            config_pair(add_jedi_academy_default_actions, "GameData/base"sv),
            config_pair(add_space_trader_default_actions, "st"sv),
            config_pair(add_call_of_duty_default_action, "Main"sv),
            config_pair(add_castle_wolfenstein_default_actions, "Main"sv),
            config_pair(add_battlemetal_default_actions, "metaldata"sv),
            config_pair(add_action_quake_2_default_actions, "baseaq"sv),
             //quake qw
            //quake rogue
            //quake hipnotic
            //quake 2 rogue
            //quake 2 xatrix
            //quake 3 missionpack
            //sin 2015
            //moh mainta
            //moh maintt
            //call of duty uo
            //quake live baseq3
            //Half life valve
            //half life gearbox
            //half life bshift
            //half life czero
            //half life czeror
            //cry of fear cryoffear
            //action action
            //anachronox anoxdata
        }};

        auto config_dir = std::find_if(config_dirs.begin(), config_dirs.end(), [&](auto& name) {
            return name.first == &info.add_default_actions;
        });

        if (config_dir == config_dirs.end())
        {
            config_dir = std::find_if(config_dirs.begin(), config_dirs.end(), [&](auto& name) {
                return name.first == &info.add_input_metadata;
            });
        }

        if (config_dir != config_dirs.end())
        {
            return config_dir->second;
        }

        return "";
    }

    std::string_view get_exe_dir(const game_info& info)
    {
        constexpr static auto exe_dirs = std::array<config_pair, 1> {{
            config_pair(add_unreal_input_metadata, "System"sv), 
        }};

        auto exe_dir = std::find_if(exe_dirs.begin(), exe_dirs.end(), [&](auto& name) {
            return name.first == &info.add_input_metadata;
        });

        if (exe_dir != exe_dirs.end())
        {
            return exe_dir->second;
        }

        return "";
    }

    environment_info_hint environment_for_game(const game_info& info)
    {
        constexpr static auto alt_names = std::array<std::array<std::string_view, 2>, 2> {{
            {"Quake II"sv, "Quake 2"sv},
            {"Unreal"sv, "Unreal Gold"sv},
        }};

        environment_info_hint env;
        env.working_dir_hints[0] = info.english_name;

        auto alt_name = std::find_if(alt_names.begin(), alt_names.end(), [&](auto name) {
            return name[0] == info.english_name;
        });

        if (alt_name != alt_names.end())
        {
            env.working_dir_hints[1] = alt_name->back();
        }

        env.config_dir = get_config_dir(info);
        env.exe_dir = get_exe_dir(info);
        
        return env;
    } 

    std::vector<std::filesystem::path> get_common_search_paths()
    {
        constexpr static auto variables = std::array<const char*, 3>
        {
            "ProgramFiles",
            "ProgramFiles(x86)",
            "SystemDrive"
        };

        constexpr static auto folders = std::array<std::string_view, 4> {{
            "GOG Galaxy/Games"sv,
            "Steam/steamapps/common"sv,
            "GOG Games"sv,
            "ZOOM PLATFORM"sv,
        }};

        std::deque<std::filesystem::path> results;

        for (auto& variable : variables)
        {
            if (auto* env = std::getenv(variable))
            {
                auto env_path = std::filesystem::path(env);

                for (auto& folder : folders)
                {
                    if (std::filesystem::exists(env_path / folder))
                    {
                        results.emplace_back(env_path / folder);
                    }
                }
            }
        }

        if (auto* home = std::getenv("HOME"))
        {
            auto home_path = std::filesystem::path(home);

            std::deque<std::filesystem::path> drive_root_paths;

            if (std::filesystem::exists(home_path / ".cxoffice"))
            {
                for (auto entry = std::filesystem::recursive_directory_iterator(home_path / ".cxoffice"); 
                        entry != std::filesystem::recursive_directory_iterator();
                        ++entry)
                {
                    if (entry.depth() == 1 && entry->path().filename() == "drive_c")
                    {
                        drive_root_paths.emplace_back(entry->path());
                    }
                }
            }

            if (std::filesystem::exists(home_path / ".wine" / "drive_c"))
            {
                drive_root_paths.emplace_back(home_path / ".wine" / "drive_c");
            }

            std::for_each(drive_root_paths.begin(), drive_root_paths.end(), [](auto& path) {
                std::cout << "Root path: " << path << std::endl;
            });

            for (auto& root : drive_root_paths)
            {
                for (auto folder : folders)
                {
                    if (std::filesystem::exists(root / folder))
                    {
                        results.emplace_back(root / folder);
                    }

                    if (std::filesystem::exists(root / "Program Files" / folder))
                    {
                        results.emplace_back(root / "Program Files" / folder);
                    }

                    if (std::filesystem::exists(root / "Program Files (x86)" / folder))
                    {
                        results.emplace_back(root / "Program Files (x86)" / folder);
                    }
                }

                results.emplace_back(root);
            }

            if (std::filesystem::exists(home_path / "Games" / "Heroic"))
            {
                results.emplace_back(home_path / "Games" / "Heroic");
            }
        }

        std::for_each(results.begin(), results.end(), [](auto& path) {
            std::cout << "Search path: " << path << std::endl;
        });

       return {results.begin(), results.end()}; 
    }

    std::vector<environment_info_hint> find_installed_game_hints(const std::vector<std::filesystem::path>& search_paths, environment_info_hint info)
    {
        std::deque<environment_info_hint> results;

        for (auto& path : search_paths)
        {
            for (auto hint : info.working_dir_hints)
            {
                if (hint.empty())
                {
                    continue;
                }
                if (std::filesystem::exists(path / hint))
                {
                    auto& temp = results.emplace_back(info);
                    std::cout << "Game path is " << path << " " << hint << std::endl;
                    temp.working_dir.emplace(path / hint);
                    break;
                }
            }
        }

        return {results.begin(), results.end()}; 
    } 

    std::vector<environment_info> verity_game_hints(const std::vector<environment_info_hint>& games)
    {
        std::deque<environment_info> results;

        for (auto& game : games)
        {
            if (!game.working_dir.has_value())
            {
                continue;
            }

            bool has_exe_dir = false;

            for (auto& file : std::filesystem::directory_iterator(game.working_dir.value() / game.exe_dir))    
            {
                if (file.path().extension() == ".exe" || 
                    file.path().extension() == ".EXE" ||
                    file.path().extension() == ".bat" || 
                    file.path().extension() == ".BAT")
                {
                    has_exe_dir = true;
                    break;
                }
            }

            bool has_config_dir = std::filesystem::exists(game.working_dir.value() / game.config_dir);

            if (has_exe_dir && has_config_dir)
            {
                results.emplace_back(environment_info { game.working_dir.value(), game.working_dir.value() / game.config_dir, game.working_dir.value() / game.exe_dir });
            }
        }
        return {results.begin(), results.end()}; 
    }
}