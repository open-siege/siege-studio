#include "game_info.hpp"

namespace siege
{
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

    std::optional<environment_info> environment_for_game(const game_info& info)
    {
        return std::nullopt;
    }   
}