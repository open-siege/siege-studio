#ifndef ENGINEEXTENDEDTYPES_HPP
#define ENGINEEXTENDEDTYPES_HPP

#include "EngineTypes.hpp"

#include <vector>
#include <string>
#include <string_view>

namespace Engine
{
	struct ExternalConsoleCallback
	{
		virtual std::string doExecuteCallback(void* console, std::int32_t callbackId, const std::vector<std::string_view>& args) = 0;
		virtual ~ExternalConsoleCallback() = default;
	};

	struct ExternalConsoleConsumer
	{
		virtual void doWriteLine(void* console, const std::string_view& line) = 0;
		virtual ~ExternalConsoleConsumer() = default;
	};

	struct ExternalGamePlugin : ExternalConsoleCallback
	{
		virtual void doInit() = 0;
		virtual void doStartFrame() = 0;

		virtual void doEndFrame() = 0;

    	virtual ~ExternalGamePlugin() = default;
	};
}

#endif
