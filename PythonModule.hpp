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
		py::class_<Engine::SimGuiConsolePlugin>(m, "SimGuiConsolePlugin")
		        .def("consoleEnable", &Engine::SimGuiConsolePlugin::consoleEnable);

        py::class_<Engine::GfxPlugin>(m, "GfxPlugin");

		py::class_<Engine::TerrainPlugin>(m, "TerrainPlugin");

		py::class_<Engine::InteriorPlugin>(m, "InteriorPlugin");

        py::class_<Engine::SkyPlugin>(m, "SkyPlugin");

        py::class_<Engine::StarsiegePlugins>(m, "StarsiegePlugins")
			.def_property_readonly("guiConsole", &Engine::StarsiegePlugins::guiConsole)
			.def_property_readonly("gfx", &Engine::StarsiegePlugins::gfx)
			.def_property_readonly("terrain", &Engine::StarsiegePlugins::terrain)
			.def_property_readonly("interior", &Engine::StarsiegePlugins::interior)
			.def_property_readonly("sky", &Engine::StarsiegePlugins::sky)
			.def_property_readonly("net", &Engine::StarsiegePlugins::net)
			.def_property_readonly("soundFx", &Engine::StarsiegePlugins::soundFx)
			.def_property_readonly("redbook", &Engine::StarsiegePlugins::redbook)
			.def_property_readonly("movPlay", &Engine::StarsiegePlugins::movPlay)
			.def_property_readonly("input", &Engine::StarsiegePlugins::input)
			.def_property_readonly("gui", &Engine::StarsiegePlugins::gui)
			.def_property_readonly("tool", &Engine::StarsiegePlugins::tool)
			.def_property_readonly("tree", &Engine::StarsiegePlugins::tree)
			.def_property_readonly("mission", &Engine::StarsiegePlugins::mission)
			.def_property_readonly("fearMission", &Engine::StarsiegePlugins::fearMission);

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


		py::class_<GameRuntime::GameConsole, std::shared_ptr<GameRuntime::GameConsole>>(m, "GameConsole")
			.def("exec", &GameRuntime::GameConsole::exec)
			.def("eval", &GameRuntime::GameConsole::eval)
            .def("echo", &GameRuntime::GameConsole::echo)
			.def("exportVariables", &GameRuntime::GameConsole::exportVariables)
			.def("deleteVariables", &GameRuntime::GameConsole::deleteVariables)
			.def("exportFunctions", &GameRuntime::GameConsole::exportFunctions)
			.def("deleteFunctions", &GameRuntime::GameConsole::deleteFunctions)
			.def("addCommand", &GameRuntime::GameConsole::addCommandExtended)
			.def("removeCommand", &GameRuntime::GameConsole::removeCommand);


		py::class_<GameRuntime::Game, std::shared_ptr<GameRuntime::Game>>(m, "Game")
			.def_property_readonly("getConsole", &GameRuntime::Game::getConsole, py::return_value_policy::reference)
			.def_property_readonly_static("currentInstance", &GameRuntime::Game::currentInstance, "Gets the current instance of a game", py::arg("functionsFileName") = "functions.json", py::return_value_policy::reference)
			.def("addPlugin", &GameRuntime::Game::addPlugin)
			.def("getPlugins", &GameRuntime::Game::getPlugins)
			.def_property_readonly("starsiegePlugins", &GameRuntime::Game::starsiegePlugins);

	}
}

#endif