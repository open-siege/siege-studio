#ifndef ENGINEFUNCTIONS_HPP
#define ENGINEFUNCTIONS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <array>

#include "EngineTypes.hpp"


namespace Engine
{
	using json = nlohmann::json;

	using GetConsoleFunc = GameConsole* (DARKCALL*)();
	using AddConsoleConsumerFunc = void (DARKCALL*) (GameConsole* console, ConsoleConsumer*);


	struct GameFunctions
	{
		GetConsoleFunc GetConsole;
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


	GameFunctions loadFunctions(std::string functionsFileName = "functions.json")
	{
		std::ifstream fileStream(functionsFileName);
		std::string str((std::istreambuf_iterator<char>(fileStream)),
					 std::istreambuf_iterator<char>());
		auto functionData = json::parse(str);
		auto defaultMapping = functionData["default"].get<std::string>();
		auto gameMapping = functionData["mappings"][defaultMapping];

		return {
			(GetConsoleFunc)std::stoul(gameMapping["ConsoleGetConsole"].get<std::string>(), nullptr, 16),
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
