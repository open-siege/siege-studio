#include "Darkstar/Proxy.hpp"

using namespace Core;

static Core::GameFunctions functions;

const char* DARKCALL executeCallbackImpl(GameConsole* console,
                                         std::int32_t callbackId,
                                         std::int32_t argc,
                                         const char** argv)
{
    if (argc == 0)
    {
        return "False";
    }

    auto real_func = functions.FindCallbackObject(argv[0], callbackId);

    if (real_func != nullptr)
    {
        return real_func->doExecuteCallback(console, callbackId, argc, argv);
    }

    return "False";
}

struct ExternalCallbacks : ConsoleCallback
{
    const char* DARKCALL executeCallback(GameConsole* console,
                                                 std::int32_t callbackId,
                                                 std::int32_t argc,
                                                 const char** argv) override
    {
        return executeCallbackImpl(console, callbackId, argc, argv);
    }
} object_callbacks;

struct ExternalConsumers : ConsoleConsumer
{
    bool is_added = false;

    void DARKCALL writeLine(GameConsole* console, const char *consoleLine) override
    {
        for (auto i = 0u; i < functions.consumersSize; ++i)
        {
            if (functions.consumers[i])
            {
                functions.consumers[i]->doWriteLine(console, consoleLine);
            }
        }
    }
} object_consumers;

struct GamePlugins : GamePlugin
{
    bool is_added = false;

    void DARKCALL destroy() override
    {
        for (auto i = 0u; i < functions.pluginsSize; ++i)
        {
            if (functions.plugins[i])
            {
                functions.plugins[i]->destroy();
                functions.plugins[i] = nullptr;
            }
        }

        functions.pluginsSize = 0;
    }

    void DARKCALL setManager(GameManager* manager) override
    {
        this->manager = manager;
    }

    void DARKCALL init() override
    {
        for (auto i = 0u; i < functions.pluginsSize; ++i)
        {
            if (functions.plugins[i])
            {
                functions.plugins[i]->doInit();
            }
        }
    }

    void DARKCALL startFrame() override
    {
        for (auto i = 0u; i < functions.pluginsSize; ++i)
        {
            if (functions.plugins[i])
            {
                functions.plugins[i]->doStartFrame();
            }
        }
    }

    void DARKCALL endFrame() override
    {
        for (auto i = 0u; i < functions.pluginsSize; ++i)
        {
            if (functions.plugins[i])
            {
                functions.plugins[i]->doEndFrame();
            }
        }
    }

    const char* DARKCALL executeCallback(GameConsole* console,
                                         std::int32_t callbackId,
                                         std::int32_t argc,
                                         const char** argv) override
    {
        return executeCallbackImpl(console, callbackId, argc, argv);
    }

    DARKCALL ~GamePlugins() override = default;
} game_plugins;

const char* DARKCALL RealCallback(GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv)
{
    if (argc == 0)
    {
        return "False";
    }

    auto real_func = functions.FindCallbackFunc(argv[0], id);

    if (real_func != nullptr)
    {
        return real_func(console, id, argc, argv);
    }

    return "False";
}

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
        if (!game_plugins.is_added)
        {
            functions.AddGamePlugin(root, &game_plugins);
            game_plugins.is_added = true;
        }

        if (functions.pluginsSize < functions.pluginsCapacity)
        {
            functions.plugins[functions.pluginsSize] = plugin;
            functions.pluginsSize++;
        }
    }

    GameConsole* APICALL DarkstarGetConsole() noexcept
    {
        return functions.GetConsole();
    }

    void APICALL DarkstarAddConsoleCallbackFunc(GameConsole* console, int id, const char* name, ExternalConsoleCallbackFunc func, int runLevel) noexcept
    {
        functions.AddCallbackFunc(name, id, func);

        functions.AddConsoleCallbackFunc(console, id, name, RealCallback, runLevel);
    }

    void APICALL DarkstarAddConsoleCallbackObject(GameConsole* console, int id, const char* name, ExternalConsoleCallback* object, int runLevel) noexcept
    {
        functions.AddCallbackObject(name, id, object);

        functions.AddConsoleCallback(console, id, name, &object_callbacks, runLevel);
    }

    void APICALL DarkstarAddConsoleConsumer(GameConsole* console, ExternalConsoleConsumer* consumer) noexcept
    {
        if (!object_consumers.is_added)
        {
            functions.AddConsoleConsumer(console, &object_consumers);
            object_consumers.is_added = true;
        }

        if (functions.consumersSize < functions.consumersCapacity)
        {
            functions.consumers[functions.consumersSize] = consumer;
            functions.consumersSize++;
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

    const char* APICALL DarkstarPluginExecuteCallback(Core::GamePlugin* plugin, std::int32_t id, std::int32_t argc, const char** argv) noexcept
    {
        return plugin->executeCallback(plugin->console, id, argc, argv);
    }
};

