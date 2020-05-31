#ifndef GAMERUNTIME_HPP
#define GAMERUNTIME_HPP

#include "EngineFunctions.hpp"

namespace GameRuntime
{
	using GameFunctions = Engine::GameFunctions;
    using ConsoleConsumer = Engine::ConsoleConsumer;

	class GameConsole
	{
			 GameFunctions& _functions;
			 Engine::GameConsole* current;

			 public:
				GameConsole(GameFunctions& functions) : _functions(functions)
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

		  public:
			  void init(std::string functionsFileName = "functions.json")
			  {
				   if (functions.GetConsole == nullptr) {
					   functions = Engine::loadFunctions(functionsFileName);
				   }
			  }

			  static Game& currentInstance(std::string functionsFileName = "functions.json")
			  {
				static Game instance;

				if (instance.functions.GetConsole == nullptr) {
						  instance.init(functionsFileName);
				}
				return instance;
			  }

			  GameConsole getInterpreter()
			  {
				  return GameConsole(functions);
              }

	};
}

#endif
