#ifndef PYMODULE_HPP
#define PYMODULE_HPP


#include <string>
#include <vector>
#include <iostream>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "PythonTypes.hpp"
#include "GameRuntime.hpp"


namespace Engine::Python
{
	namespace py = pybind11;

	PYBIND11_EMBEDDED_MODULE(darkstar, m) {
		py::class_<PyConsoleConsumer>(m, "PyConsoleConsumer")
			.def(py::init<>())
			.def("doWriteLine", &ExternalConsoleConsumer::doWriteLine);

		py::class_<ExternalConsoleCallback, PyConsoleCallback>(m, "PyConsoleCallback")
			.def(py::init<>())
			.def("doExecuteCallback", &ExternalConsoleCallback::doExecuteCallback);

		py::class_<PyGamePlugin>(m, "PyGamePlugin")
			.def(py::init<>())
			.def("doExecuteCallback", &ExternalGamePlugin::doExecuteCallback)
			.def("doInit", &ExternalGamePlugin::doInit)
			.def("doStartFrame", &ExternalGamePlugin::doStartFrame)
			.def("doEndFrame", &ExternalGamePlugin::doEndFrame);


		  py::class_<GameRuntime::GameConsole>(m, "GameConsole")
			.def("exec", &GameRuntime::GameConsole::exec)
			.def("eval", &GameRuntime::GameConsole::eval)
            .def("echo", &GameRuntime::GameConsole::echo)
			.def("exportVariables", &GameRuntime::GameConsole::exportVariables)
			.def("deleteVariables", &GameRuntime::GameConsole::deleteVariables)
			.def("exportFunctions", &GameRuntime::GameConsole::exportFunctions)
			.def("deleteFunctions", &GameRuntime::GameConsole::deleteFunctions)
			.def("addCommand", &GameRuntime::GameConsole::addCommandExtended)
			.def("removeCommand", &GameRuntime::GameConsole::removeCommand);

		py::class_<GameRuntime::Game>(m, "Game")
			.def("getConsole", &GameRuntime::Game::getConsole)
			.def("addPlugin", &GameRuntime::Game::addPlugin)
			.def("getPlugins", &GameRuntime::Game::getPlugins);

			m.def("currentInstance", &GameRuntime::Game::currentInstance, "Gets the current instance of a game", py::arg("functionsFileName") = "functions.json");

	}
}

#endif