#ifndef GAMERUNTIME_HPP
#define GAMERUNTIME_HPP

#include <memory>
#include <map>
#include "EngineFunctions.hpp"
#include "EngineExternalTypes.hpp"
#include "PythonTypes.hpp"


namespace GameRuntime
{
	using GameFunctions = Engine::GameFunctions;
	using GameRoot = Engine::GameRoot;
	using GamePlugin = Engine::GamePlugin;
	using ConsoleConsumer = Engine::ConsoleConsumer;
	using ConsoleCallback = Engine::ConsoleCallback;
	using ConsoleCallbackFunc = Engine::ConsoleCallbackFunc;
	using ExternalConsoleCallback = Engine::ExternalConsoleCallback;
	using ConsoleCallbackWrapper = Engine::ConsoleCallbackWrapper;
    using PyConsoleCallback = Engine::Python::PyConsoleCallback;

	class GameConsole
	{
			 GameFunctions& _functions;
			 Engine::GameConsole* current;
			 std::map<std::string, std::shared_ptr<ConsoleCallbackWrapper>> _wrappedCallbacks;

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

				std::string cls()
				{
					std::array<const char*, 1> arguments {"cls"};
					return _functions.ConsoleCls(current, 0, arguments.size(), arguments.data());
				}


				std::string sqrt(const std::string& someNumber)
				{
					std::array<const char*, 2> arguments {"sqrt", someNumber.c_str()};
					return _functions.ConsoleSqrt(current, 0, arguments.size(), arguments.data());
				}

				std::string floor(const std::string& someNumber)
				{
					std::array<const char*, 2> arguments {"floor", someNumber.c_str()};
					return _functions.ConsoleFloor(current, 0, arguments.size(), arguments.data());
				}

				std::string echo(const std::string& message)
				{
					std::array<const char*, 2> arguments {"echo", message.c_str()};
					return _functions.ConsoleEcho(current, 0, arguments.size(), arguments.data());
				}


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

				std::string quit()
				{
					std::array<const char*, 1> arguments {"quit"};
					return _functions.ConsoleQuit(current, 0, arguments.size(), arguments.data());
				}

				std::string exec(const std::string& filename)
				{
					std::array<const char*, 2> arguments {"exec", filename.c_str()};
					return _functions.ConsoleExec(current, 0, arguments.size(), arguments.data());
				}

				std::string eval(const std::string& someCode)
				{
					std::array<const char*, 2> arguments {"eval", someCode.c_str()};
					return _functions.ConsoleEval(current, 0, arguments.size(), arguments.data());
				}

				std::string exportVariables(const std::string& varableGlob, const std::string& filename, const std::string& append)
				{
					std::array<const char*, 4> arguments {"export", varableGlob.c_str(), filename.c_str(), append.c_str()};
					return _functions.ConsoleExportVariables(current, 0, arguments.size(), arguments.data());
				}

				std::string deleteVariables(const std::string& varableGlob)
				{
					std::array<const char*, 2> arguments {"deleteVariables", varableGlob.c_str()};
					return _functions.ConsoleDeleteVariables(current, 0, arguments.size(), arguments.data());
				}

				std::string exportFunctions(const std::string& functionGlob, const std::string& filename, const std::string& append)
				{
					std::array<const char*, 4> arguments {"exportFunctions", functionGlob.c_str(), filename.c_str(), append.c_str()};
					return _functions.ConsoleExportFunctions(current, 0, arguments.size(), arguments.data());
				}

				std::string deleteFunctions(const std::string& functionGlob)
				{
					std::array<const char*, 2> arguments {"deleteFunctions", functionGlob.c_str()};
					return _functions.ConsoleDeleteFunctions(current, 0, arguments.size(), arguments.data());
				}

				std::string trace()
				{
					std::array<const char*, 1> arguments {"trace"};
					return _functions.ConsoleTrace(current, 0, arguments.size(), arguments.data());
				}

				std::string debug()
				{
					std::array<const char*, 1> arguments {"debug"};
					return _functions.ConsoleDebug(current, 0, arguments.size(), arguments.data());
				}

				void addConsumer(ConsoleConsumer* consumer)
				{
					_functions.AddConsoleConsumer(current, consumer);
				}

				void addCommandFunc(int id, const std::string& name, ConsoleCallbackFunc func, int runLevel = 0)
				{
					   _functions.AddConsoleCallbackFunc(current, id, name.c_str(), func, runLevel);
				}


				void addCommandExtended(int id, const std::string& name, PyConsoleCallback* callback, int runLevel = 0)
				{
					auto newCallback = _wrappedCallbacks[name] = std::make_shared<ConsoleCallbackWrapper>(callback);
					  addCommand(id, name, newCallback.get(), runLevel);
				}


				void addCommand(int id, const std::string& name, ConsoleCallback* callback, int runLevel = 0)
				{
					   _functions.AddConsoleCallback(current, id, name.c_str(), callback, runLevel);
				}

				bool removeCommand(const std::string& name)
				{
                    return false;
                }
	};

	class Game
	{
		  GameRoot* current = nullptr;
		  GameFunctions functions = { nullptr };

		  public:
			  void init(std::string functionsFileName = "functions.json")
			  {
				   if (functions.GetConsole == nullptr) {
					   functions = Engine::loadFunctions(functionsFileName);
				   }

                   current = functions.GetGameRoot();
			  }

			  static std::shared_ptr<Game> currentInstance(std::string functionsFileName = "functions.json")
			  {
				static std::shared_ptr<Game> instance = std::make_shared<Game>();

				if (instance->functions.GetConsole == nullptr) {
						  instance->init(functionsFileName);
				}
				return instance;
			  }

			  std::shared_ptr<GameConsole> getConsole()
			  {
				  static std::shared_ptr<GameConsole> console = std::make_shared<GameConsole>(functions);
				  return console;
			  }

			  void addPlugin(GamePlugin* plugin)
			  {
                  functions.AddGamePlugin(current, plugin);
			  }

			  std::vector<GamePlugin*> getPlugins()
			  {
				  std::vector<GamePlugin*> result;
				  auto rawArray = (Engine::DynamicArray<GamePlugin*>*)((std::uint8_t*)current + 264);

				  result.reserve(rawArray->capacity);

				  for (int i = 0; i < rawArray->size; i++)
				  {
                        result.push_back(rawArray->data[i]);
				  }
				  return result;
              }
	};
}

#endif
