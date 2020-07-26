#include <memory>
#include <map>
#include <utility>
#include "Core/EngineFunctions.hpp"
#include "Core/EngineExternalTypes.hpp"
#include "Hook/Game.hpp"
#include "Hook/GameConsole.hpp"

namespace Hook
{
	void Game::init(std::string functionsFileName)
	{
		if (functions.GetConsole == nullptr)
		{
			functions = Core::loadFunctions(functionsFileName);
		}

		current = functions.GetGameRoot();
	}

	std::shared_ptr<Game> Game::currentInstance(std::string functionsFileName)
	{
		static std::shared_ptr<Game> instance = std::make_shared<Game>();

		if (instance->functions.GetConsole == nullptr)
		{
			instance->init(functionsFileName);
		}
		return instance;
	}

	std::shared_ptr<GameConsole> Game::getConsole()
	{
		static std::shared_ptr<GameConsole> console = std::make_shared<GameConsole>(functions);
		return console;
	}

	void Game::addPlugin(GamePlugin* plugin)
	{
		functions.AddGamePlugin(current, plugin);
	}

	std::vector<GamePlugin*> Game::getPlugins()
	{
		std::vector<GamePlugin*> result;
		auto rawArray = (Core::DynamicArray<GamePlugin*>*)((std::uint8_t*)current + 264);

		result.reserve(rawArray->capacity);

		for (decltype(rawArray->size) i = 0; i < rawArray->size; i++)
		{
			result.push_back(rawArray->data[i]);
		}
		return result;
	}

	Plugins::StarsiegePlugins Game::starsiegePlugins()
	{
		return Plugins::StarsiegePlugins{getPlugins()};
    }
}
