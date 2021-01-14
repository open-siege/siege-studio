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

#include "Hook/Game.hpp"
#include "Hook/GameConsole.hpp"

static std::atomic_int32_t noAllocs{ 0 };
static std::atomic_bool isRunning{ false };

void runExtender() noexcept
{
	try
	{
		std::ofstream file{ "test.log" };
		file << "Hello there!" << std::endl;

		auto game = Hook::Game::currentInstance();

		auto console = game->getConsole();

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

		file << "raw console: " << console->getRaw() << std::endl;
		file << "raw plugin console: " << plugins[0]->console << std::endl;
	}
	catch (const std::exception & ex)
	{
		std::ofstream file{ "darkstar-hook-errors.log", std::ios_base::app };
		file << ex.what() << std::endl;
	}
}

extern "C"
{
    extern __declspec(dllexport) void* _cdecl MS_Malloc(std::size_t size) noexcept
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
        if (noAllocs >= 55 && !isRunning)
        {
            isRunning = true;
            runExtender();
        }

        return std::malloc(size);
    }

    extern __declspec(dllexport) void _cdecl MS_Free(void* data) noexcept
    {
        noAllocs--;
        std::free(data);
    }

    extern __declspec(dllexport) void* _cdecl MS_Realloc(void* data, std::size_t size) noexcept
    {
        return std::realloc(data, size);
    }

    extern __declspec(dllexport) void* _cdecl MS_Calloc(std::size_t num, std::size_t size) noexcept
    {
        return std::calloc(num, size);
    }
}
