#include <memory>
#include <map>
#include <string_view>
#include <set>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <Darkstar/Proxy.hpp>
#include "Hook/Game.hpp"
#include "Hook/GameConsole.hpp"

namespace Hook
{
    static std::set<std::string> callbackNames;
    static std::map<std::pair<std::string_view, std::int32_t>, Core::ExternalConsoleCallbackFunc> callbackFuncs;
    static std::map<std::pair<std::string_view, std::int32_t>, Core::ExternalConsoleCallback*> callbackObjects;
    static std::vector<Core::ExternalConsoleConsumer*> consumers(10, nullptr);
    static std::vector<Core::ExternalGamePlugin*> plugins(10, nullptr);

    template<typename ResultType>
    void APICALL AddCallback(std::map<std::pair<std::string_view, std::int32_t>, ResultType>& callbacks, const char* name, std::int32_t id, ResultType func)
    {
        auto result = callbackNames.emplace(name);
        callbacks.emplace(std::make_pair(std::string_view(*result.first), id), func);
    }

    template<typename ResultType>
    ResultType APICALL FindCallback(const std::map<std::pair<std::string_view, std::int32_t>, ResultType>& callbacks, const char* name, std::int32_t id)
    {
        auto result = callbacks.find(std::make_pair(std::string_view(name), id));

        if (result != callbacks.end())
        {
            return result->second;
        }

        return nullptr;
    }

    void APICALL AddCallbackFunc(const char* name, std::int32_t id, Core::ExternalConsoleCallbackFunc func)
    {
        AddCallback(callbackFuncs, name, id, func);
    }

    Core::ExternalConsoleCallbackFunc APICALL FindCallbackFunc(const char* name, std::int32_t id)
    {
        return FindCallback(callbackFuncs, name, id);
    }

    void APICALL AddCallbackObject(const char* name, std::int32_t id, Core::ExternalConsoleCallback* func)
    {
        AddCallback(callbackObjects, name, id, func);
    }

    Core::ExternalConsoleCallback* APICALL FindCallbackObject(const char* name, std::int32_t id)
    {
        return FindCallback(callbackObjects, name, id);
    }

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
                AddCallbackFunc,
                FindCallbackFunc,
                AddCallbackObject,
                FindCallbackObject,
                consumers.data(),
                0,
                consumers.size(),
                plugins.data(),
                0,
                plugins.size(),
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
