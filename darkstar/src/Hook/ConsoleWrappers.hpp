#ifndef GAMERUNTIMEEXTERNALTYPES_HPP
#define GAMERUNTIMEEXTERNALTYPES_HPP

#include <memory>
#include <map>
#include <utility>
#include "Core/EngineFunctions.hpp"
#include "Core/EngineExternalTypes.hpp"

namespace Hook
{
	using GameFunctions = Core::GameFunctions;
	using GameRoot = Core::GameRoot;
	using GamePlugin = Core::GamePlugin;
	using ConsoleConsumer = Core::ConsoleConsumer;
	using ConsoleCallback = Core::ConsoleCallback;
	using ConsoleCallbackFunc = Core::ConsoleCallbackFunc;
	using ExternalConsoleCallback = Core::ExternalConsoleCallback;

	template<typename TCallback>
	struct ConsoleCallbackWrapper : ConsoleCallback
	{
		TCallback* _internalCallback;
		std::string _lastResult;

		ConsoleCallbackWrapper(TCallback* callback) : _internalCallback(callback) {}

		virtual const char* DARKCALL executeCallback(Core::GameConsole* console,
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
