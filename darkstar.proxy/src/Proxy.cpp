#include <map>
#include <set>
#include <string_view>

#include "Darkstar/Proxy.hpp"

using namespace Core;

static Core::GameFunctions functions;

static std::map<std::string, ExternalConsoleCallbackFunc> func_callbacks;

const char* DARKCALL executeCallback(std::map<std::string, ExternalConsoleCallback*>& callbacks, GameConsole* console,
                                     std::int32_t callbackId,
                                     std::int32_t argc,
                                     const char** argv)
{
    if (argc == 0)
    {
    return "False";
    }

    std::string new_name = argv[0] + std::to_string(callbackId);
    auto real_func = callbacks.find(new_name);

    if (real_func != callbacks.end())
    {
    return real_func->second->doExecuteCallback(console, callbackId, argc, argv);
    }

    return "False";
}

struct ExternalCallbacks : ConsoleCallback
{
    static std::map<std::string, ExternalConsoleCallback*> callbacks;

    const char* DARKCALL executeCallback(GameConsole* console,
                                                 std::int32_t callbackId,
                                                 std::int32_t argc,
                                                 const char** argv) override
    {
        return ::executeCallback(callbacks, console, callbackId, argc, argv);
    }
} object_callbacks;

std::map<std::string, ExternalConsoleCallback*> ExternalCallbacks::callbacks = {};

struct ExternalConsumers : ConsoleConsumer
{
    static bool is_added;
    static std::set<ExternalConsoleConsumer*> consumers;

    void DARKCALL writeLine(GameConsole* console, const char *consoleLine) override
    {
        for (auto* consumer : consumers)
        {
            consumer->doWriteLine(console, consoleLine);
        }
    }
} object_consumers;

bool ExternalConsumers::is_added = false;
std::set<ExternalConsoleConsumer*> ExternalConsumers::consumers = {};

struct GamePlugins : GamePlugin
{
    static bool is_added;
    static std::set<ExternalGamePlugin*> plugins;

    void DARKCALL destroy() override
    {
        for (auto* plugin : plugins)
        {
            plugin->destroy();
        }
        plugins.clear();
    }

    void DARKCALL setManager(GameManager* manager) override
    {
        this->manager = manager;
    }

    void DARKCALL init() override
    {
        for (auto* plugin : plugins)
        {
            plugin->doInit();
        }
    }

    void DARKCALL startFrame() override
    {
        for (auto* plugin : plugins)
        {
            plugin->doStartFrame();
        }
    }

    void DARKCALL endFrame() override
    {
        for (auto* plugin : plugins)
        {
            plugin->doEndFrame();
        }
    }

    const char* DARKCALL executeCallback(GameConsole* console,
                                         std::int32_t callbackId,
                                         std::int32_t argc,
                                         const char** argv) override
    {
        return ::executeCallback(ExternalCallbacks::callbacks, console, callbackId, argc, argv);
    }

    DARKCALL ~GamePlugins() override = default;
} game_plugins;

bool GamePlugins::is_added = false;
std::set<ExternalGamePlugin*> GamePlugins::plugins = {};

extern "C"
{
    void APICALL DarkstarSetGameFunctions(const GameFunctions& newFunctions) noexcept
    {
        functions = newFunctions;
    }

    GameRoot* APICALL DarkstarGetGameRoot() noexcept
    {
        return functions.GetGameRoot();
    }

    void APICALL DarkstarAddGamePlugin(GameRoot* root, ExternalGamePlugin* plugin) noexcept
    {
        if (!GamePlugins::is_added)
        {
            functions.AddGamePlugin(root, &game_plugins);
        }

        GamePlugins::plugins.emplace(plugin);
    }

    GameConsole* APICALL DarkstarGetConsole() noexcept
    {
        return functions.GetConsole();
    }

    const char* DARKCALL RealCallback(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv)
    {
        if (argc == 0)
        {
            return "False";
        }
        std::string new_name = argv[0] + std::to_string(id);
        auto real_func = func_callbacks.find(new_name);

        if (real_func != func_callbacks.end())
        {
            return real_func->second(console, id, argc, argv);
        }

        return "False";
    }

    void APICALL DarkstarAddConsoleCallbackFunc(GameConsole* console, int id, const char* name, ExternalConsoleCallbackFunc func, int runLevel) noexcept
    {
        std::string new_name = name + std::to_string(id);
        func_callbacks[new_name] = func;

        functions.AddConsoleCallbackFunc(console, id, name, RealCallback, runLevel);
    }

    void APICALL DarkstarAddConsoleCallbackObject(GameConsole* console, int id, const char* name, ExternalConsoleCallback* object, int runLevel) noexcept
    {
        std::string new_name = name + std::to_string(id);
        ExternalCallbacks::callbacks[new_name] = object;

        functions.AddConsoleCallback(console, id, name, &object_callbacks, runLevel);
    }

    void APICALL DarkstarAddConsoleConsumer(GameConsole* console, ExternalConsoleConsumer* consumer) noexcept
    {
        ExternalConsumers::consumers.emplace(consumer);

        if (!ExternalConsumers::is_added)
        {
            functions.AddConsoleConsumer(console, &object_consumers);
            ExternalConsumers::is_added = true;
        }
    }

    const char* APICALL DarkstarConsoleCls(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleCls(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleSqrt(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleSqrt(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleFloor(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleFloor(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleEcho(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleEcho(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleDbEcho(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleDbEcho(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleStrCat(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleStrCat(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleQuit(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleQuit(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleExec(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleExec(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleEval(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleEval(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleExportVariables(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleExportVariables(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleDeleteVariables(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleDeleteVariables(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleExportFunctions(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleExportFunctions(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleDeleteFunctions(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleDeleteFunctions(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleTrace(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleTrace(console, id, argc, argv);
    }

    const char* APICALL DarkstarConsoleDebug(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return functions.ConsoleDebug(console, id, argc, argv);
    }
};

