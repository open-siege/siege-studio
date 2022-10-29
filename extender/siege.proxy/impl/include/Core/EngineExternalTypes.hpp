#ifndef ENGINEEXTENDEDTYPES_HPP
#define ENGINEEXTENDEDTYPES_HPP

#include "Core/EngineTypes.hpp"

namespace Core
{
    using ExternalConsoleCallbackFunc = const char* (APICALL*)(GameConsole*, std::int32_t, std::int32_t, const char**);

	struct ExternalConsoleCallback
	{
		virtual const char* APICALL doExecuteCallback(GameConsole* console,
                                              std::int32_t callbackId,
                                              std::int32_t argc,
                                              const char** argv) = 0;
	};

	struct ExternalConsoleConsumer
	{
		virtual void APICALL doWriteLine(GameConsole*, const char *consoleLine) = 0;
	};

	struct ExternalGamePlugin : ExternalConsoleCallback
	{
		virtual void APICALL doInit() = 0;
		virtual void APICALL doStartFrame() = 0;

		virtual void APICALL doEndFrame() = 0;

        virtual void DARKCALL destroy() = 0;
    	virtual APICALL ~ExternalGamePlugin() = default;
	};
}

#endif
