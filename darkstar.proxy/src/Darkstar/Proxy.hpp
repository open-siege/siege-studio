#ifndef DARKSTAR_EXTENDER_TROMPLINE_HPP
#define DARKSTAR_EXTENDER_TROMPLINE_HPP

#include "Core/EngineFunctions.hpp"
#include "Core/EngineExternalTypes.hpp"

extern "C"
{
    void APICALL DarkstarSetGameFunctions(const Core::GameFunctions& newFunctions) noexcept;
    Core::GameRoot* APICALL DarkstarGetGameRoot() noexcept;
    void APICALL DarkstarAddGamePlugin(Core::GameRoot* root, Core::ExternalGamePlugin* plugin) noexcept;

    Core::GameConsole* APICALL DarkstarGetConsole() noexcept;
    void APICALL DarkstarAddConsoleCallbackFunc(Core::GameConsole* console, int id, const char* name, Core::ExternalConsoleCallbackFunc func, int runLevel) noexcept;
    void APICALL DarkstarAddConsoleCallbackObject(Core::GameConsole* console, int id, const char* name, Core::ExternalConsoleCallback* object, int runLevel) noexcept;
    void APICALL DarkstarAddConsoleConsumer(Core::GameConsole* console, Core::ExternalConsoleConsumer* consumer) noexcept;

    const char* APICALL DarkstarConsoleCls(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleSqrt(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleFloor(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleEcho(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleDbEcho(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleStrCat(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleQuit(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleExec(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleEval(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleExportVariables(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleDeleteVariables(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleExportFunctions(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleDeleteFunctions(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleTrace(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarConsoleDebug(Core::GameConsole* console, std::int32_t id, std::int32_t argc, const char** argv) noexcept;

    const char* APICALL DarkstarPluginExecuteCallback(Core::GamePlugin* plugin, std::int32_t id, std::int32_t argc, const char** argv) noexcept;
};



#endif //DARKSTAR_EXTENDER_TROMPLINE_HPP
