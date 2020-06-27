#ifndef GAMERUNTIME_HPP
#define GAMERUNTIME_HPP

#include <memory>
#include <map>
#include <utility>
#include "EngineFunctions.hpp"
#include "EngineExternalTypes.hpp"
#include "PythonTypes.hpp"
#include "GameRuntimeExternalTypes.hpp"
#include "EnginePlugins.hpp"

namespace GameRuntime
{
	using GameFunctions = Engine::GameFunctions;
	using GameRoot = Engine::GameRoot;
	using GamePlugin = Engine::GamePlugin;
	using ConsoleConsumer = Engine::ConsoleConsumer;
	using ConsoleCallback = Engine::ConsoleCallback;
	using ConsoleCallbackFunc = Engine::ConsoleCallbackFunc;
	using ExternalConsoleCallback = Engine::ExternalConsoleCallback;
	using PyConsoleCallback = Engine::Python::PyConsoleCallback;

	class GameConsole;

	class Game
	{
		GameRoot* current = nullptr;
		GameFunctions functions = { nullptr };

	public:
		void init(std::string functionsFileName = "functions.json");

		static std::shared_ptr<Game> currentInstance(std::string functionsFileName = "functions.json");

		std::shared_ptr<GameConsole> getConsole();

		void addPlugin(GamePlugin* plugin);

		std::vector<GamePlugin*> getPlugins();

        Engine::StarsiegePlugins starsiegePlugins();
	};

	class GameConsole
	{
		GameFunctions& _functions;
		Engine::GameConsole* current;
		std::map<std::string, std::shared_ptr<ConsoleCallback>> _wrappedCallbacksByName;
		std::map<ExternalConsoleCallback*, std::shared_ptr<ConsoleCallback>> _wrappedCallbacksByKey;

		template<typename TStringCollection>
		void copyArguments(std::vector<const char*>& arguments, const TStringCollection& args)
		{
			for (const std::string& argument : args) // access by const reference
			{
				arguments.push_back(argument.c_str());
			}
		}

	public:
		GameConsole(GameFunctions& functions) : _functions(functions)
		{
			current = _functions.GetConsole();
		}

		Engine::GameConsole* getRaw()
		{
			return current;
		}

		std::string cls();

		std::string sqrt(const std::string& someNumber);

		std::string floor(const std::string& someNumber);

		std::string echo(const std::string& message);

		template<typename TStringCollection>
		std::string echoRange(const TStringCollection& args)
		{
			std::vector<const char*> arguments;
			arguments.reserve(args.size() + 1);
			arguments.push_back("echo");
			copyArguments(arguments, args);
			return _functions.ConsoleEcho(current, 0, arguments.size(), arguments.data());
		}

		template<typename TStringCollection>
		std::string dbecho(const TStringCollection& args)
		{
			std::vector<const char*> arguments;
			arguments.reserve(args.size() + 1);
			arguments.push_back("dbecho");
			copyArguments(arguments, args);
			return _functions.ConsoleDbEcho(current, 0, arguments.size(), arguments.data());
		}

		template<typename TStringCollection>
		std::string strcat(const TStringCollection& args)
		{
			std::vector<const char*> arguments;
			arguments.reserve(args.size() + 1);
			arguments.push_back("strcat");
			copyArguments(arguments, args);
			return _functions.ConsoleStrCat(current, 0, arguments.size(), arguments.data());
		}

		std::string quit();

		std::string exec(const std::string& filename);

		std::string eval(const std::string& someCode);

		std::string exportVariables(const std::string& varableGlob, const std::string& filename, const std::string& append);

		std::string deleteVariables(const std::string& varableGlob);

		std::string exportFunctions(const std::string& functionGlob, const std::string& filename, const std::string& append);

		std::string deleteFunctions(const std::string& functionGlob);

		std::string trace();

		std::string debug();

		void addConsumer(ConsoleConsumer* consumer);

		void addCommandFunc(int id, const std::string& name, ConsoleCallbackFunc func, int runLevel = 0);

		void addCommandExtended(int id, const std::string& name, PyConsoleCallback* callback, int runLevel = 0);

		void addCommand(int id, const std::string& name, ConsoleCallback* callback, int runLevel = 0);

		bool removeCommand(const std::string& name);
	};
}

#endif
