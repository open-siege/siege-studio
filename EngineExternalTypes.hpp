#ifndef ENGINEEXTENDEDTYPES_HPP
#define ENGINEEXTENDEDTYPES_HPP

#include "EngineTypes.hpp"

#include <vector>
#include <string>

namespace Engine
{
	struct ExternalConsoleCallback
	{
		virtual std::string doExecuteCallback(void* console, std::int32_t callbackId, std::vector<std::string>& args) = 0;
		virtual ~ExternalConsoleCallback() = default;
	};

	struct ExternalConsoleConsumer
	{
		virtual void doWriteLine(void* console, const std::string& line) = 0;
		virtual ~ExternalConsoleConsumer() = default;
	};

	struct ExternalGamePlugin
	{
		virtual std::string doExecuteCallback(void* console, std::int32_t callbackId, std::vector<std::string>& args) = 0;

		virtual void doInit() = 0;
		virtual void doStartFrame() = 0;

		virtual void doEndFrame() = 0;

    	virtual ~ExternalGamePlugin() = default;
	};

	struct ConsoleCallbackWrapper : ConsoleCallback
	{
		 ExternalConsoleCallback* _internalCallback;
		 std::string _lastResult;

		 ConsoleCallbackWrapper(ExternalConsoleCallback* callback) : _internalCallback(callback)
		 {

         }

		  virtual const char* DARKCALL executeCallback(GameConsole* console,
				std::int32_t callbackId,
				std::int32_t argc,
				const char** argv)
				{
					try
					{
						std::vector<std::string> arguments;
						_lastResult = _internalCallback->doExecuteCallback(console, callbackId, arguments);

					}
					catch(const std::exception& ex)
					{
							std::ofstream file{"darkstar-hook-errors.log", std::ios_base::app};
							file << ex.what() << std::endl;
                            return "False";
					}

					return _lastResult.c_str();
				}
	};
}

#endif
