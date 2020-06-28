#ifndef PYPLUGIN_HPP
#define PYPLUGIN_HPP

#include <pybind11/embed.h>
#include <string>
#include <vector>
#include <iostream>

#include "Core/EngineExternalTypes.hpp"

namespace Python
{
	namespace py = pybind11;

	struct PyConsoleConsumer : Core::ExternalConsoleConsumer
	{
			virtual void doWriteLine(void* console, const std::string_view& line) override
			{
					PYBIND11_OVERLOAD_PURE(
						void, /* Return type */
						ExternalConsoleConsumer,      /* Parent class */
						doWriteLine,          /* Name of function in C++ (must match Python name) */
						console,    /* Argument(s) */
						line
					);
			}
    };


	struct PyConsoleCallback : Core::ExternalConsoleCallback
	{
            virtual std::string doExecuteCallback(void* console, std::int32_t callbackId, const std::vector<std::string_view>& args) override
			{
					PYBIND11_OVERLOAD_PURE(
						std::string, /* Return type */
						ExternalConsoleCallback,      /* Parent class */
						doExecuteCallback,          /* Name of function in C++ (must match Python name) */
						console,    /* Argument(s) */
						callbackId,
						args
					);
			}
    };

	struct PyGamePlugin : Core::ExternalGamePlugin
	{
			virtual std::string doExecuteCallback(void* console, std::int32_t callbackId, const std::vector<std::string_view>& args) override
			{
					PYBIND11_OVERLOAD_PURE(
						std::string, /* Return type */
						ExternalGamePlugin,      /* Parent class */
						doExecuteCallback,          /* Name of function in C++ (must match Python name) */
						console,    /* Argument(s) */
						callbackId,
						args
					);
			}

			virtual void doInit() override
			{
				  PYBIND11_OVERLOAD_PURE(
						void, /* Return type */
						ExternalGamePlugin,      /* Parent class */
						doInit);
			}

			virtual void doStartFrame() override
			{
				 PYBIND11_OVERLOAD_PURE(
						void, /* Return type */
						ExternalGamePlugin,      /* Parent class */
						doStartFrame);
			}

			virtual void doEndFrame() override
			{
					PYBIND11_OVERLOAD_PURE(
						void, /* Return type */
						ExternalGamePlugin,      /* Parent class */
						doEndFrame);
			}
	};
}

#endif