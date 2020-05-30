#ifndef GAMERUNTIME_HPP
#define GAMERUNTIME_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <array>

// Most Darkstar games have functions using Borland fastcall,
// so this is considered the standard calling convention for functions
#define DARKCALL __fastcall


namespace GameRuntime
{
	using json = nlohmann::json;


	struct GameConsole;

	struct ConsoleConsumer
	{
		virtual void DARKCALL writeLine(GameConsole*, const char *consoleLine) = 0;
	};


	using GetConsoleFunc = GameConsole* (DARKCALL*)();
	using AddConsoleConsumerFunc = void (DARKCALL*) (GameConsole* console, ConsoleConsumer*);
	using ConsoleEvalFunc = const char* (DARKCALL*) (GameConsole* console, int functionId, int argc, const char** argv);


	struct GameFunctions
	{
		GetConsoleFunc GetConsole;
		AddConsoleConsumerFunc AddConsoleConsumer;
		ConsoleEvalFunc ConsoleEval;
	};


	GameFunctions loadFunctions(std::string functionsFileName = "functions.json")
	{
		std::ifstream fileStream(functionsFileName);
		std::string str((std::istreambuf_iterator<char>(fileStream)),
					 std::istreambuf_iterator<char>());
		auto functionData = json::parse(str);
		auto defaultMapping = functionData["default"].get<std::string>();
		auto gameMapping = functionData["mappings"][defaultMapping];

		return {
			(GetConsoleFunc)std::stoul(gameMapping["ConsoleGetConsole"].get<std::string>(), nullptr, 16),
			(AddConsoleConsumerFunc)std::stoul(gameMapping["ConsoleAddConsumer"].get<std::string>(), nullptr, 16),
			(ConsoleEvalFunc)std::stoul(gameMapping["ConsoleEval"].get<std::string>(), nullptr, 16),
		};
	}

	class GameInterpreter
	{
			 GameFunctions& _functions;
			 GameConsole* current;

			 public:
				GameInterpreter(GameFunctions& functions) : _functions(functions)
				{
					current = _functions.GetConsole();
				}

				std::string eval(std::string someCode)
				{
					std::array<const char*, 2> arguments {"eval", someCode.c_str()};
					return _functions.ConsoleEval(current, 0, arguments.size(), arguments.data());
				}

				void addConsumer(ConsoleConsumer* consumer)
				{
					_functions.AddConsoleConsumer(current, consumer);
				}
	};

	class Game
	{
		  GameFunctions functions = { nullptr };

		  static Game instance;

		  public:
			  void init(std::string functionsFileName = "functions.json")
			  {
				   if (functions.GetConsole == nullptr) {
					   functions = loadFunctions(functionsFileName);
				   }
			  }

			  static Game& currentInstance(std::string functionsFileName = "functions.json")
			  {
				   if (instance.functions.GetConsole == nullptr) {
						  instance.init(functionsFileName);
                   }
				return instance;
			  }

			  GameInterpreter getInterpreter()
			  {
				  return GameInterpreter(functions);
              }

	};
}

#endif
