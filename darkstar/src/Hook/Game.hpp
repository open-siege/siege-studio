#ifndef GAME_HPP
#define GAME_HPP

#include <memory>
#include <map>
#include <utility>
#include <Darkstar/Proxy.hpp>
#include "Hook/Plugins/StarsiegePlugins.hpp"

namespace Hook
{
	using GameRoot = Core::GameRoot;
	using GamePlugin = Core::GamePlugin;
	using ExternalGamePlugin = Core::ExternalGamePlugin;

	class GameConsole;

	class Game
	{
		GameRoot* current = nullptr;

	public:
		void init(std::string functionsFileName = "config.json");

		static std::shared_ptr<Game> currentInstance(std::string functionsFileName = "config.json");

		std::shared_ptr<GameConsole> getConsole();

		void addPlugin(ExternalGamePlugin* plugin);

		std::vector<GamePlugin*> getPlugins();

        Plugins::StarsiegePlugins starsiegePlugins();
	};
}

#endif
