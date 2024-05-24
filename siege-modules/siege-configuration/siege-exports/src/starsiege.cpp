#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <utility>
#include <string_view>
#include <fstream>
#include <siege/platform/win/core/file.hpp>

extern "C"
{
    extern std::uint32_t ExportModuleIsValid = 0;
    extern std::int8_t* ExportFunctionsStr = nullptr;
    extern std::int8_t* DeleteFunctionsStr = nullptr;
    extern std::int8_t* ExecStr = nullptr;
    extern void* GetGameRoot = nullptr;
    extern void* ConsoleGetConsole = nullptr;
    extern void* ConsoleExec = nullptr;

    constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = {{
    std::array<std::pair<std::string_view, std::size_t>, 3>{{
        {std::string_view("exportFunctions"), std::size_t(0x712d37)},
        {std::string_view("deleteFunctions"), std::size_t(0x712d47)},
        {std::string_view("exec"), std::size_t(0x712d57)}
        }}
    }};

    inline void set_1003_exports()
    {
        ExportModuleIsValid = -1;
        ExportFunctionsStr = (int8_t*)0x712d37;
        DeleteFunctionsStr = (int8_t*)0x712d47;
        ExecStr = (int8_t*)0x712d47;
        GetGameRoot = (void*)0x59d9b0;
        ConsoleGetConsole = (void*)0x59d79c;
        ConsoleExec = (void*)0x5e3678;
    }

    constexpr std::array<void(*)(), 1> export_functions = {{
            set_1003_exports
        }};

	BOOL WINAPI DllMain(
        HINSTANCE hinstDLL,  
        DWORD fdwReason, 
        LPVOID lpvReserved ) noexcept
    {

        if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
        {
            if (lpvReserved != nullptr)
            {
                return TRUE; // do not do cleanup if process termination scenario
            }

           if (fdwReason == DLL_PROCESS_ATTACH)
           {
               int index = 0;

               std::ofstream log("log.txt", std::ios::app);

               for (const auto& item : verification_strings)
               {
                    auto all_detected = std::all_of(item.begin(), item.end(), [](const auto& str) {
                        return std::memcmp(str.first.data(), (void*)str.second, str.first.size()) == 0;        
                    });

                    if (all_detected)
                    {
                        log << "Functions detected\n";
                        export_functions[index]();
                        char** str =  (char**)::GetProcAddress(hinstDLL, "ExportFunctionsStr");

                        log << *str << '\n';
                        
                        break;
                    }
                    else
                    {
                        log << "No log detected\n";
                    }
                    index++;
               }
           }
           else if (fdwReason == DLL_PROCESS_DETACH)
           {

           }
        }

        return TRUE;
    }
}


