#ifndef ENGINEFUNCTIONS_HPP
#define ENGINEFUNCTIONS_HPP

#include <string>
#include <fstream>
#include <streambuf>
#include <array>

#include "Core/EngineTypes.hpp"

namespace Core
{
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
}

#endif
