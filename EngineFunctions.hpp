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
	using ConsoleEvalFunc = const char* (DARKCALL*) (GameConsole* console, int functionId, int argc, const char** argv);


	struct GameFunctions
	{
		GetConsoleFunc GetConsole;
		AddConsoleConsumerFunc AddConsoleConsumer;
		ConsoleEvalFunc ConsoleEval;
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
			(ConsoleEvalFunc)std::stoul(gameMapping["ConsoleEval"].get<std::string>(), nullptr, 16),
		};
	}
}

#endif
