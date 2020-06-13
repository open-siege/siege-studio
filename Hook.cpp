// Important note about DLL memory management when your DLL uses the
// static version of the RunTime Library:
//
// If your DLL exports any functions that pass String objects (or structs/
// classes containing nested Strings) as parameter or function results,
// you will need to add the library MEMMGR.LIB to both the DLL project and
// any other projects that use the DLL.  You will also need to use MEMMGR.LIB
// if any other projects which use the DLL will be performing new or delete
// operations on any non-TObject-derived classes which are exported from the
// DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
// EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
// the file BORLNDMM.DLL should be deployed along with your DLL.
//
// To avoid using BORLNDMM.DLL, pass string information using "char *" or
// ShortString parameters.
//
// If your DLL uses the dynamic version of the RTL, you do not need to
// explicitly add MEMMGR.LIB as this will be done implicitly for you

#pragma hdrstop
#pragma argsused

#include <cstdint>
#include <memory>
#include <atomic>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstdint>
#include <array>
#include <vector>
#include <pybind11/embed.h>

#include "GameRuntime.hpp"
#include "PythonModule.hpp"

namespace py = pybind11;

static std::unique_ptr<std::thread> pythonThread{ nullptr };
static std::atomic_int32_t noAllocs{ 0 };
static std::atomic_bool isRunning{ false };

struct TestConsoleConsumer : public Engine::ConsoleConsumer
{
	virtual void DARKCALL writeLine(Engine::GameConsole*, const char* consoleLine) override
	{
		std::ofstream file{ "super-special-log.log", std::ios_base::app };
		file << consoleLine << std::endl;
	}
};


struct TestGamePlugin : public Engine::GamePlugin
{
	std::atomic_int32_t numFrames{ 0 };

    std::shared_ptr<GameRuntime::GameConsole> _console;

	virtual const char* DARKCALL executeCallback(Engine::GameConsole* console,
		std::int32_t callbackId,
		std::int32_t argc,
		const char** argv) override
	{
		std::ofstream file{ "cpp-plugin.log", std::ios_base::app };
		file << "executeCallback has been called" << std::endl;
        _console->echo("Hello world from new Starsiege plugin");
		return "False";
	}

    virtual void DARKCALL destroy() override
	{

    }

	virtual void DARKCALL setManager(Engine::GameManager* manager) override
	{
		std::ofstream file{ "cpp-plugin.log", std::ios_base::app };
		file << "setManager has been called with " << manager << std::endl;
	}

	virtual void DARKCALL init() override
	{
		std::ofstream file{ "cpp-plugin.log", std::ios_base::app };
		file << "init has been called" << std::endl;

		_console->addCommand(0, "cpp::testPlugin", this);
	}

	virtual void DARKCALL startFrame() override
	{
		if (numFrames == 0)
		{
			std::ofstream file{ "cpp-plugin.log", std::ios_base::app };
			file << "startFrame has been called" << std::endl;
		}
	}

	virtual void DARKCALL endFrame() override
	{
		if (numFrames == 0)
		{
			std::ofstream file{ "cpp-plugin.log", std::ios_base::app };
			file << "endFrame has been called" << std::endl;
		}

		numFrames++;
		if (numFrames == 9) {
			numFrames = 0;
		}
	}

	virtual DARKCALL ~TestGamePlugin() override = default;
};


struct TestConsoleCallback : public Engine::ConsoleCallback
{
	std::string _lastResult;

	virtual const char* DARKCALL executeCallback(Engine::GameConsole* otherConsole,
		std::int32_t callbackId,
		std::int32_t argc,
		const char** argv)
	{

		_lastResult = "\"";
		try
		{
			auto game = GameRuntime::Game::currentInstance();
			auto console = game->getConsole();
			std::ofstream file{ "test.log", std::ios_base::app };
			file << "callback game runtime: " << game.get() << std::endl;
			file << "callback console: " << console.get() << std::endl;
			file << console->echoRange(std::array<std::string, 1>{"Hello world from cpp::testCallback"}) << std::endl;
		}
		catch (const std::exception & ex)
		{
			std::ofstream file{ "darkstar-hook-errors.log", std::ios_base::app };
			file << ex.what() << std::endl;
		}

		std::vector<std::string> arguments(argv, argv + argc);
		for (const std::string& argument : arguments) // access by const reference
		{
			_lastResult += argument;
		}

		_lastResult += "\"";
		return _lastResult.c_str();
	}
};

void runPython()
{
	try
	{
		std::ofstream file{ "test.log" };
		file << "Hello there!" << std::endl;

		auto game = GameRuntime::Game::currentInstance();

		auto console = game->getConsole();

		console->addConsumer(new TestConsoleConsumer());
		console->addCommand(0, "cpp::testCallback", new TestConsoleCallback());
		auto plugin = new TestGamePlugin();
        plugin->_console = console;
		game->addPlugin(plugin);

		file << "init game runtime: " << game.get() << std::endl;
		file << "init console: " << console.get() << std::endl;
		file << console->echoRange(std::array<std::string, 1>{"Hello world from echo in C++ with array"}) << std::endl;
		file << console->echoRange(std::vector<std::string>{"Hello world from echo in C++ with vector"}) << std::endl;
		file << console->dbecho(std::array<std::string, 2>{"1", "Hello world from dbecho in C++ with array"}) << std::endl;
		file << console->dbecho(std::vector<std::string>{"1", "Hello world from dbecho in C++ with vector"}) << std::endl;
		file << console->strcat(std::array<std::string, 1>{"Hello world from strcat in C++ with array"}) << std::endl;
		file << console->strcat(std::vector<std::string>{"Hello world from strcat in C++ with vector"}) << std::endl;
		file << console->eval("echo(\"Hello world from eval in C++\");");

		std::ofstream newScript{ "test-script.cs" };
		newScript << "echo(\"Hello world from exec in C++ from test-script.cs\");" << std::endl;
		newScript.close();
		file << console->exec("test-script.cs");

		file << "Floor of 1.5: " << console->floor("1.5") << std::endl;
		file << "console.exportFunctions: " << console->exportFunctions("*", "exportFunctions.cs", "False") << std::endl;
		file << "console.exportVariables: " << console->exportVariables("*", "exportVariables.cs", "False") << std::endl;
		file << "Sqrt of 144: " << console->sqrt("144") << std::endl;

		auto plugins = game->getPlugins();

		file << "Number of plugins inside of game: " << plugins.size() << " "
			<< plugins.capacity() << " "
			<< std::endl;
		//plugins[0]->executeCallback(console.getRaw(), 3, 0, nullptr);

		try
		{
			static py::scoped_interpreter guard{};
			py::eval_file("simple.py");
		}
		catch (const std::exception & ex)
		{
			file << "An error ocurred with python" << std::endl;
			file << ex.what() << std::endl;
		}
	}
	catch (const std::exception & ex)
	{
		std::ofstream file{ "darkstar-hook-errors.log", std::ios_base::app };
		file << ex.what() << std::endl;
	}
}

extern "C" int _libmain(unsigned long reason)
{
	return 1;
}

extern "C" __declspec(dllexport) void* _cdecl MS_Malloc(std::size_t size)
{
	noAllocs++;

	// why 55? for now it's the number that works
	// but in reality, there are several allocations
	// of core engine interals taking place
	// and they pass right through this funciton.
	// By waiting for everything, we can then hook into parts of the game
	// before the rest of the game is loaded.
	// TODO put the 55 into a config section somewhere
	// because it could change per game or game version.
	if (noAllocs >= 55 && !isRunning) {
		isRunning = true;
		runPython();
	}

	return std::malloc(size);
}

extern "C" __declspec(dllexport) void _cdecl MS_Free(void* data)
{
	noAllocs--;
	std::free(data);
}

extern "C" __declspec(dllexport) void* _cdecl MS_Realloc(void* data, std::size_t size)
{
	return std::realloc(data, size);
}

extern "C" __declspec(dllexport) void* _cdecl MS_Calloc(std::size_t num, std::size_t size)
{
	return std::calloc(num, size);
}
