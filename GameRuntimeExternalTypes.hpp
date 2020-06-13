#ifndef GAMERUNTIMEEXTERNALTYPES_HPP
#define GAMERUNTIMEEXTERNALTYPES_HPP

#include <memory>
#include <map>
#include <utility>
#include "EngineFunctions.hpp"
#include "EngineExternalTypes.hpp"
#include "PythonTypes.hpp"


namespace GameRuntime
{
	using GameFunctions = Engine::GameFunctions;
	using GameRoot = Engine::GameRoot;
	using GamePlugin = Engine::GamePlugin;
	using ConsoleConsumer = Engine::ConsoleConsumer;
	using ConsoleCallback = Engine::ConsoleCallback;
	using ConsoleCallbackFunc = Engine::ConsoleCallbackFunc;
	using ExternalConsoleCallback = Engine::ExternalConsoleCallback;
	using PyConsoleCallback = Engine::Python::PyConsoleCallback;

	template<typename TCallback>
	struct ConsoleCallbackWrapper : ConsoleCallback
	{
		TCallback* _internalCallback;
		std::string _lastResult;

		ConsoleCallbackWrapper(TCallback* callback) : _internalCallback(callback) {}

		virtual const char* DARKCALL executeCallback(Engine::GameConsole* console,
			std::int32_t callbackId,
			std::int32_t argc,
			const char** argv)
		{
			try
			{
				std::vector<std::string_view> arguments(argv, argv + argc);
				_lastResult = _internalCallback->doExecuteCallback(console, callbackId, arguments);

			}
			catch (const std::exception & ex)
			{
				std::ofstream file{ "darkstar-hook-errors.log", std::ios_base::app };
				file << ex.what() << std::endl;
				return "False";
			}

			return _lastResult.c_str();
		}
	};

	template<typename TConsumer>
	struct ConsoleConsumerWrapper : ConsoleConsumer
	{

	};

	template<typename TPlugin>
	struct GamePluginWrapper : ConsoleCallbackWrapper<TPlugin>
	{

	};
}

#endif
