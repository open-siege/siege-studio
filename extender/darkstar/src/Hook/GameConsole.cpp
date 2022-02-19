#include <memory>
#include <map>
#include <utility>
#include <array>
#include <string>
#include "Core/EngineFunctions.hpp"
#include "Core/EngineExternalTypes.hpp"
#include "Hook/GameConsole.hpp"

namespace Hook
{
    std::string GameConsole::cls()
    {
        std::array<const char*, 1> arguments{ "cls" };
        return DarkstarConsoleCls(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::sqrt(const std::string& someNumber)
    {
        std::array<const char*, 2> arguments{ "sqrt", someNumber.c_str() };
        return DarkstarConsoleSqrt(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::floor(const std::string& someNumber)
    {
        std::array<const char*, 2> arguments{ "floor", someNumber.c_str() };
        return DarkstarConsoleFloor(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::echo(const std::string& message)
    {
        std::array<const char*, 2> arguments{ "echo", message.c_str() };
        return DarkstarConsoleEcho(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::quit()
    {
        std::array<const char*, 1> arguments{ "quit" };
        return DarkstarConsoleQuit(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::exec(const std::string& filename)
    {
        std::array<const char*, 2> arguments{ "exec", filename.c_str() };
        return DarkstarConsoleExec(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::eval(const std::string& someCode)
    {
        std::array<const char*, 2> arguments{ "eval", someCode.c_str() };
        return DarkstarConsoleEval(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::exportVariables(const std::string& varableGlob, const std::string& filename, const std::string& append)
    {
        std::array<const char*, 4> arguments{ "export", varableGlob.c_str(), filename.c_str(), append.c_str() };
        return DarkstarConsoleExportVariables(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::deleteVariables(const std::string& varableGlob)
    {
        std::array<const char*, 2> arguments{ "deleteVariables", varableGlob.c_str() };
        return DarkstarConsoleDeleteVariables(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::exportFunctions(const std::string& functionGlob, const std::string& filename, const std::string& append)
    {
        std::array<const char*, 4> arguments{ "exportFunctions", functionGlob.c_str(), filename.c_str(), append.c_str() };
        return DarkstarConsoleExportFunctions(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::deleteFunctions(const std::string& functionGlob)
    {
        std::array<const char*, 2> arguments{ "deleteFunctions", functionGlob.c_str() };
		return DarkstarConsoleDeleteFunctions(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::trace()
    {
        std::array<const char*, 1> arguments{ "trace" };
        return DarkstarConsoleTrace(current, 0, arguments.size(), arguments.data());
    }

    std::string GameConsole::debug()
    {
        std::array<const char*, 1> arguments{ "debug" };
        return DarkstarConsoleDebug(current, 0, arguments.size(), arguments.data());
    }

    void GameConsole::addConsumer(Core::ExternalConsoleConsumer* consumer)
    {
        DarkstarAddConsoleConsumer(current, consumer);
    }

    void GameConsole::addCommandFunc(int id, const std::string& name, Core::ExternalConsoleCallbackFunc func, int runLevel)
    {
        DarkstarAddConsoleCallbackFunc(current, id, name.c_str(), func, runLevel);
    }

    void GameConsole::addCommand(int id, const std::string& name, Core::ExternalConsoleCallback* callback, int runLevel)
    {
        DarkstarAddConsoleCallbackObject(current, id, name.c_str(), callback, runLevel);
    }

	std::string GameConsole::removeCommand(const std::string& name)
    {
		return deleteFunctions(name);
    }
}
