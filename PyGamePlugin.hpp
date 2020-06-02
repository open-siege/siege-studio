#ifndef PYPLUGIN_HPP
#define PYPLUGIN_HPP

#include <pybind11/embed.h>
#include <string>
#include <vector>

#include "EngineTypes.hpp"


namespace Engine::Python
{
	namespace py = pybind11;
	class PyGamePlugin : public ExtendedGamePlugin {

		std::string _lastString;
		public:

			virtual const char* DARKCALL executeCallback(GameConsole* console,
					std::int32_t callbackId,
					std::int32_t argc,
					const char** argv) override
			{

				  std::vector<std::string> arguments {argv, argv + argc };
				 _lastString = doExecuteCallback(console, callbackId, arguments);

				 return _lastString.c_str();
			}

			virtual DARKCALL ~PyGamePlugin() = default;

			virtual void DARKCALL setManager(GameManager* manager) override
			{
				this->manager = manager;
			}

			virtual void DARKCALL init() override
			{
				doInit();
			}
			virtual void DARKCALL startFrame() override
			{
				doStartFrame();
			}

			virtual void DARKCALL endFrame() override
			{
				doEndFrame();
			}

			virtual std::string doExecuteCallback(void* console, std::int32_t callbackId, std::vector<std::string>& args) override
			{
					PYBIND11_OVERLOAD_PURE(
						std::string, /* Return type */
						ExtendedGamePlugin,      /* Parent class */
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
						ExtendedGamePlugin,      /* Parent class */
						doInit);
			}

			virtual void doStartFrame() override
			{
				 PYBIND11_OVERLOAD_PURE(
						void, /* Return type */
						ExtendedGamePlugin,      /* Parent class */
						doStartFrame);
			}

			virtual void doEndFrame() override
			{
					PYBIND11_OVERLOAD_PURE(
						void, /* Return type */
						ExtendedGamePlugin,      /* Parent class */
						doEndFrame);
			}
	};

    PYBIND11_EMBEDDED_MODULE(darkstar, m) {
		py::class_<Engine::Python::PyGamePlugin>(m, "GamePlugin")
			.def("doExecuteCallback", &ExtendedGamePlugin::doExecuteCallback)
			.def("doInit", &ExtendedGamePlugin::doInit)
			.def("doStartFrame", &ExtendedGamePlugin::doStartFrame)
			.def("doEndFrame", &ExtendedGamePlugin::doEndFrame);
	}
}

#endif