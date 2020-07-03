#include <memory>
#include <map>
#include <utility>
#include "Core/EngineFunctions.hpp"
#include "Core/EngineExternalTypes.hpp"
#include "Python/PythonTypes.hpp"
#include "Hook/GameConsole.hpp"
#include "Hook/ConsoleWrappers.hpp"

namespace Hook
{
    std::string GameConsole::cls()
    {
        std::array<const char*, 1> arguments{ "cls" };
        return _functions.ConsoleCls(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::sqrt(const std::string& someNumber)
    {
        std::array<const char*, 2> arguments{ "sqrt", someNumber.c_str() };
        return _functions.ConsoleSqrt(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::floor(const std::string& someNumber)
    {
        std::array<const char*, 2> arguments{ "floor", someNumber.c_str() };
        return _functions.ConsoleFloor(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::echo(const std::string& message)
    {
        std::array<const char*, 2> arguments{ "echo", message.c_str() };
        return _functions.ConsoleEcho(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::quit()
    {
        std::array<const char*, 1> arguments{ "quit" };
        return _functions.ConsoleQuit(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::exec(const std::string& filename)
    {
        std::array<const char*, 2> arguments{ "exec", filename.c_str() };
        return _functions.ConsoleExec(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::eval(const std::string& someCode)
    {
        std::array<const char*, 2> arguments{ "eval", someCode.c_str() };
        return _functions.ConsoleEval(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::exportVariables(const std::string& varableGlob, const std::string& filename, const std::string& append)
    {
        std::array<const char*, 4> arguments{ "export", varableGlob.c_str(), filename.c_str(), append.c_str() };
        return _functions.ConsoleExportVariables(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::deleteVariables(const std::string& varableGlob)
    {
        std::array<const char*, 2> arguments{ "deleteVariables", varableGlob.c_str() };
        return _functions.ConsoleDeleteVariables(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::exportFunctions(const std::string& functionGlob, const std::string& filename, const std::string& append)
    {
        std::array<const char*, 4> arguments{ "exportFunctions", functionGlob.c_str(), filename.c_str(), append.c_str() };
        return _functions.ConsoleExportFunctions(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::deleteFunctions(const std::string& functionGlob)
    {
        std::array<const char*, 2> arguments{ "deleteFunctions", functionGlob.c_str() };
		return _functions.ConsoleDeleteFunctions(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::trace()
    {
        std::array<const char*, 1> arguments{ "trace" };
        return _functions.ConsoleTrace(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::debug()
    {
        std::array<const char*, 1> arguments{ "debug" };
        return _functions.ConsoleDebug(current, 0, arguments.size(), arguments.data());
    }

    void GameConsole::addConsumer(Core::ConsoleConsumer* consumer)
    {
        _functions.AddConsoleConsumer(current, consumer);
    }

    void GameConsole::addCommandFunc(int id, const std::string& name, Core::ConsoleCallbackFunc func, int runLevel)
    {
        _functions.AddConsoleCallbackFunc(current, id, name.c_str(), func, runLevel);
    }

    void GameConsole::addCommandExtended(int id, const std::string& name, Python::PyConsoleCallback* callback, int runLevel)
    {
        auto key = static_cast<Core::ExternalConsoleCallback*>(callback);

        auto existingWrapper = _wrappedCallbacksByKey.find(key);

        if (existingWrapper == _wrappedCallbacksByKey.end()) {
            auto newCallback = std::make_shared<ConsoleCallbackWrapper<Python::PyConsoleCallback>>(callback);
            _wrappedCallbacksByKey[key] = newCallback;
            _wrappedCallbacksByName[name] = newCallback;

            addCommand(id, name, newCallback.get(), runLevel);
            return;
        }

        // If the name is new, we can reuse the same callback
		auto wrapperByName = _wrappedCallbacksByName.find(name);
        if (wrapperByName == _wrappedCallbacksByName.end())
        {
            _wrappedCallbacksByName[name] = existingWrapper->second;

            addCommand(id, name, existingWrapper->second.get(), runLevel);
        }
    }

    void GameConsole::addCommand(int id, const std::string& name, ConsoleCallback* callback, int runLevel)
    {
        _functions.AddConsoleCallback(current, id, name.c_str(), callback, runLevel);
    }

	std::string GameConsole::removeCommand(const std::string& name)
    {
		return deleteFunctions(name);
    }
}