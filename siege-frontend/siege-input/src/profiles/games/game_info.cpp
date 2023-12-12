#include <array>
#include <algorithm>
#include <vector>
#include <string_view>
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
             //quake qw
            //quake rogue
            //quake hipnotic
            //quake 2 rogue
            //quake 2 xatrix
            //quake 3 missionpack
            //sin 2015
            //daikatana data
            //kingpin main
            //elite force BaseEF
            //moh main
            //moh mainta
            //moh maintt
            //jedi outcast GameData base
            //jedi academy GameData base
            //space trader st
            //call of duty Main
            //call of duty uo
            //quake live baseq3
            //Half life valve
            //half life gearbox
            //half life bshift
            //half life czero
            //half life czeror
            //rtcw Main
            //cry of fear cryoffear
            //battlemetal metaldata
            //action baseaq
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

    std::optional<environment_info> environment_for_game(const game_info& info)
    {
        constexpr static auto alt_names = std::array<std::array<std::string_view, 2>, 1> {{
            {"Quake II"sv, "Quake 2"sv},
        }};

        environment_info env{};
        env.root_dir_hints[0] = info.english_name;

        auto alt_name = std::find_if(alt_names.begin(), alt_names.end(), [&](auto name) {
            return name[0] == info.english_name;
        });

        if (alt_name != alt_names.end())
        {
            env.root_dir_hints[1] = alt_name->back();
        }

        env.config_dir = get_config_dir(info);
        env.exe_dir = get_exe_dir(info);
        
        return std::nullopt;
    }   
}