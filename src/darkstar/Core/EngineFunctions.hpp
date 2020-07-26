#ifndef ENGINEFUNCTIONS_HPP
#define ENGINEFUNCTIONS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <array>

#include "Core/EngineTypes.hpp"

namespace Core
{
	using json = nlohmann::json;

	using GetGameRootFunc = GameRoot* (DARKCALL*)();
	using AddGamePluginFunc = void(DARKCALL*)(GameRoot*, GamePlugin*);
	using GetConsoleFunc = GameConsole* (DARKCALL*)();
	using AddConsoleCallbackFuncFunc = void (DARKCALL*)(GameConsole*, int id, const char* name, ConsoleCallbackFunc, int runLevel);
	using AddConsoleCallbackObjectFunc = void (DARKCALL*) (GameConsole*, int id, const char* name, ConsoleCallback*, int runLevel);
	using AddConsoleConsumerFunc = void (DARKCALL*) (GameConsole* console, ConsoleConsumer*);


	struct GameFunctions
	{
		GetGameRootFunc GetGameRoot;
		AddGamePluginFunc AddGamePlugin;

		GetConsoleFunc GetConsole;

		AddConsoleCallbackFuncFunc AddConsoleCallbackFunc;
		AddConsoleCallbackObjectFunc AddConsoleCallback;

		AddConsoleConsumerFunc AddConsoleConsumer;

		ConsoleCallbackFunc ConsoleCls;
		ConsoleCallbackFunc ConsoleSqrt;
		ConsoleCallbackFunc ConsoleFloor;
		ConsoleCallbackFunc ConsoleEcho;
		ConsoleCallbackFunc ConsoleDbEcho;
		ConsoleCallbackFunc ConsoleStrCat;
		ConsoleCallbackFunc ConsoleQuit;
		ConsoleCallbackFunc ConsoleExec;
		ConsoleCallbackFunc ConsoleEval;
		ConsoleCallbackFunc ConsoleExportVariables;
		ConsoleCallbackFunc ConsoleDeleteVariables;
		ConsoleCallbackFunc ConsoleExportFunctions;
		ConsoleCallbackFunc ConsoleDeleteFunctions;
		ConsoleCallbackFunc ConsoleTrace;
		ConsoleCallbackFunc ConsoleDebug;
	};


	inline GameFunctions loadFunctions(std::string functionsFileName = "functions.json")
	{
		std::ifstream fileStream(functionsFileName);
		std::string str((std::istreambuf_iterator<char>(fileStream)),
					 std::istreambuf_iterator<char>());
		auto functionData = json::parse(str);
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
}

#endif
