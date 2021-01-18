#include <memory>
#include <map>
#include <utility>
#include <nlohmann/json.hpp>
#include <Darkstar/Proxy.hpp>
#include "Hook/Game.hpp"
#include "Hook/GameConsole.hpp"

namespace Hook
{
    Core::GameFunctions loadFunctions(std::string functionsFileName = "config.json")
    {
        using namespace Core;
        std::ifstream fileStream(functionsFileName);
        std::string str((std::istreambuf_iterator<char>(fileStream)),
                        std::istreambuf_iterator<char>());
        auto functionData = nlohmann::json::parse(str);
        auto defaultMapping = functionData["default"].get<std::string>();
        auto gameMapping = functionData["mappings"][defaultMapping];

        return {
                (GetGameRootFunc)std::stoul(gameMapping["GetGameRoot"].get<std::string>(), nullptr, 16),
                (AddGamePluginFunc)std::stoul(gameMapping["AddGamePlugin"].get<std::string>(), nullptr, 16),
                (GetConsoleFunc)std::stoul(gameMapping["ConsoleGetConsole"].get<std::string>(), nullptr, 16),
                (AddConsoleCallbackFuncFunc)std::stoul(gameMapping["ConsoleAddCommandFunc"].get<std::string>(), nullptr, 16),
                (AddConsoleCallbackObjectFunc)std::stoul(gameMapping["ConsoleAddCommandObject"].get<std::string>(), nullptr, 16),
                (AddConsoleConsumerFunc)std::stoul(gameMapping["ConsoleAddConsumer"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleCls"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleSqrt"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleFloor"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleEcho"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleDbEcho"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleStrCat"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleQuit"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleExec"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleEval"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleExportVariables"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleDeleteVariables"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleExportFunctions"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleDeleteFunctions"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleTrace"].get<std::string>(), nullptr, 16),
                (ConsoleCallbackFunc)std::stoul(gameMapping["ConsoleDebug"].get<std::string>(), nullptr, 16)
        };
    }


    void Game::init(std::string configFilename)
	{
        DarkstarSetGameFunctions(loadFunctions(std::move(configFilename)));
		current = DarkstarGetGameRoot();
	}

	std::shared_ptr<Game> Game::currentInstance(std::string configFilename)
	{
		static std::shared_ptr<Game> instance = std::make_shared<Game>();

		if (instance->current == nullptr)
		{
			instance->init(std::move(configFilename));
		}
		return instance;
	}

	std::shared_ptr<GameConsole> Game::getConsole()
	{
		static std::shared_ptr<GameConsole> console = std::make_shared<GameConsole>();
		return console;
	}

	void Game::addPlugin(ExternalGamePlugin* plugin)
	{
		DarkstarAddGamePlugin(current, plugin);
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
