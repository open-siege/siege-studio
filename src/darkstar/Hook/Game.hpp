#ifndef GAME_HPP
#define GAME_HPP

#include <memory>
#include <map>
#include <utility>
#include "Core/EngineFunctions.hpp"
#include "Core/EngineExternalTypes.hpp"
#include "Hook/Plugins/StarsiegePlugins.hpp"

namespace Hook
{
	using GameFunctions = Core::GameFunctions;
	using GameRoot = Core::GameRoot;
	using GamePlugin = Core::GamePlugin;
	using ConsoleConsumer = Core::ConsoleConsumer;
	using ConsoleCallback = Core::ConsoleCallback;
	using ConsoleCallbackFunc = Core::ConsoleCallbackFunc;
	using ExternalConsoleCallback = Core::ExternalConsoleCallback;

	class GameConsole;

	class Game
	{
		GameRoot* current = nullptr;
		GameFunctions functions = { nullptr };

	public:
		void init(std::string functionsFileName = "functions.json");

		static std::shared_ptr<Game> currentInstance(std::string functionsFileName = "functions.json");

		std::shared_ptr<GameConsole> getConsole();

		void addPlugin(GamePlugin* plugin);

		std::vector<GamePlugin*> getPlugins();

        Plugins::StarsiegePlugins starsiegePlugins();
	};
}

#endif
