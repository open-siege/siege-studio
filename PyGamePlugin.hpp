#ifndef PYPLUGIN_HPP
#define PYPLUGIN_HPP

#include <pybind11/embed.h>
#include <string>
#include <vector>
#include <iostream>

#include "EngineTypes.hpp"
#include "GameRuntime.hpp"


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
				  std::cout << "execute callback called";
				  std::vector<std::string> arguments {argv, argv + argc };
				 _lastString = doExecuteCallback(console, callbackId, arguments);

				 return _lastString.c_str();
			}

			virtual DARKCALL ~PyGamePlugin() = default;

			virtual void DARKCALL setManager(GameManager* manager) override
			{
				std::cout << "setManager called";
				this->manager = manager;
			}

			virtual void DARKCALL init() override
			{
				std::cout << "init called";
				doInit();
			}
			virtual void DARKCALL startFrame() override
			{
				std::cout << "startFrame called";
				doStartFrame();
			}

			virtual void DARKCALL endFrame() override
			{
				std::cout << "endFrame called";
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
			.def(py::init<>())
			.def("doExecuteCallback", &ExtendedGamePlugin::doExecuteCallback)
			.def("doInit", &ExtendedGamePlugin::doInit)
			.def("doStartFrame", &ExtendedGamePlugin::doStartFrame)
			.def("doEndFrame", &ExtendedGamePlugin::doEndFrame);


		  py::class_<GameRuntime::GameConsole>(m, "GameConsole")
			.def("exec", &GameRuntime::GameConsole::exec)
			.def("eval", &GameRuntime::GameConsole::eval)
			.def("exportVariables", &GameRuntime::GameConsole::exportVariables)
			.def("deleteVariables", &GameRuntime::GameConsole::deleteVariables)
			.def("exportFunctions", &GameRuntime::GameConsole::exportFunctions)
			.def("deleteFunctions", &GameRuntime::GameConsole::deleteFunctions);

		py::class_<GameRuntime::Game>(m, "Game")
			.def("getConsole", &GameRuntime::Game::getConsole)
			.def("addPlugin", &GameRuntime::Game::addPlugin)
			.def("getPlugins", &GameRuntime::Game::getPlugins);

			m.def("currentInstance", &GameRuntime::Game::currentInstance, "Gets the current instance of a game", py::arg("functionsFileName") = "functions.json");

	}
}

#endif