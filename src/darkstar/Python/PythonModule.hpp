#ifndef PYMODULE_HPP
#define PYMODULE_HPP


#include <string>
#include <vector>
#include <iostream>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "Python/PythonTypes.hpp"
#include "Hook/Game.hpp"
#include "Hook/GameConsole.hpp"

namespace Python
{
	namespace py = pybind11;
	namespace Plugins = Hook::Plugins;
	using ExternalGamePlugin = Core::ExternalGamePlugin;
	using GameConsole = Hook::GameConsole;
	using Game = Hook::Game;


	PYBIND11_EMBEDDED_MODULE(darkstar, m) {
		py::class_<Plugins::SimGuiConsolePlugin>(m, "SimGuiConsolePlugin")
		        .def("consoleEnable", &Plugins::SimGuiConsolePlugin::consoleEnable);

		py::class_<Plugins::GfxPlugin>(m, "GfxPlugin")
				.def("screenshot", &Plugins::GfxPlugin::screenshot);

		py::class_<Plugins::TerrainPlugin>(m, "TerrainPlugin");

		py::class_<Plugins::InteriorPlugin>(m, "InteriorPlugin");

        py::class_<Plugins::SkyPlugin>(m, "SkyPlugin");

        py::class_<Plugins::StarsiegePlugins>(m, "StarsiegePlugins")
			.def_property_readonly("guiConsole", &Plugins::StarsiegePlugins::guiConsole)
			.def_property_readonly("gfx", &Plugins::StarsiegePlugins::gfx)
			.def_property_readonly("terrain", &Plugins::StarsiegePlugins::terrain)
			.def_property_readonly("interior", &Plugins::StarsiegePlugins::interior)
			.def_property_readonly("sky", &Plugins::StarsiegePlugins::sky)
			.def_property_readonly("net", &Plugins::StarsiegePlugins::net)
			.def_property_readonly("soundFx", &Plugins::StarsiegePlugins::soundFx)
			.def_property_readonly("redbook", &Plugins::StarsiegePlugins::redbook)
			.def_property_readonly("movPlay", &Plugins::StarsiegePlugins::movPlay)
			.def_property_readonly("input", &Plugins::StarsiegePlugins::input)
			.def_property_readonly("gui", &Plugins::StarsiegePlugins::gui)
			.def_property_readonly("tool", &Plugins::StarsiegePlugins::tool)
			.def_property_readonly("tree", &Plugins::StarsiegePlugins::tree)
			.def_property_readonly("mission", &Plugins::StarsiegePlugins::mission)
			.def_property_readonly("fearMission", &Plugins::StarsiegePlugins::fearMission);

		py::class_<PyConsoleConsumer>(m, "PyConsoleConsumer")
			.def(py::init<>())
			.def("doWriteLine", &Core::ExternalConsoleConsumer::doWriteLine);

		py::class_<Core::ExternalConsoleCallback, PyConsoleCallback, >(m, "PyConsoleCallback")
			.def(py::init<>())
			.def("doExecuteCallback", &Core::ExternalConsoleCallback::doExecuteCallback);

		py::class_<PyGamePlugin>(m, "PyGamePlugin")
			.def(py::init<>())
			.def("doExecuteCallback", &Core::ExternalGamePlugin::doExecuteCallback)
			.def("doInit", &Core::ExternalGamePlugin::doInit)
			.def("doStartFrame", &Core::ExternalGamePlugin::doStartFrame)
			.def("doEndFrame", &Core::ExternalGamePlugin::doEndFrame);

		py::class_<GameConsole, std::shared_ptr<GameConsole>>(m, "GameConsole")
			.def("exec", &GameConsole::exec)
			.def("eval", &GameConsole::eval)
            .def("echo", &GameConsole::echo)
			.def("exportVariables", &GameConsole::exportVariables)
			.def("deleteVariables", &GameConsole::deleteVariables)
			.def("exportFunctions", &GameConsole::exportFunctions)
			.def("deleteFunctions", &GameConsole::deleteFunctions)
			.def("addCommand", &GameConsole::addCommandExtended)
			.def("removeCommand", &GameConsole::removeCommand);

		py::class_<Game, std::shared_ptr<Game>>(m, "Game")
			.def_property_readonly("console", &Game::getConsole, py::return_value_policy::reference)
			.def_static("currentInstance", &Game::currentInstance, "Gets the current instance of a game", py::arg("functionsFileName") = "functions.json", py::return_value_policy::reference)
			.def("addPlugin", &Game::addPlugin)
			.def("getPlugins", &Game::getPlugins)
			.def_property_readonly("starsiegePlugins", &Game::starsiegePlugins);
	}
}

#endif