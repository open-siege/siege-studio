#ifndef ENGINEFUNCTIONS_HPP
#define ENGINEFUNCTIONS_HPP

#include <cstdint>

#include "Core/EngineTypes.hpp"
#include "Core/EngineExternalTypes.hpp"

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
        void (APICALL *AddCallbackFunc)(const char* name, std::int32_t id, ExternalConsoleCallbackFunc);
        ExternalConsoleCallbackFunc (APICALL *FindCallbackFunc)(const char* name, std::int32_t id);

        void (APICALL *AddCallbackObject)(const char* name, std::int32_t id, ExternalConsoleCallback*);
        ExternalConsoleCallback* (APICALL *FindCallbackObject)(const char* name, std::int32_t id);

        ExternalConsoleConsumer** consumers;
        std::size_t consumersSize;
        std::size_t consumersCapacity;

        ExternalGamePlugin** plugins;
        std::size_t pluginsSize;
        std::size_t pluginsCapacity;

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
