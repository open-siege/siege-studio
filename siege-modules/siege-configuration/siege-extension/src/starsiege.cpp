#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/core/file.hpp>

extern "C"
{
    #define DARKCALL __attribute__((regparm(3)))

    static std::uint32_t ExportModuleIsValid = 0;
    static void* DARKCALL (*GetGameRoot)() = nullptr;
    static void* DARKCALL (*ConsoleGetConsole)(void*) = nullptr;
    static char* DARKCALL (*ConsoleEval)(void*, std::int32_t, std::int32_t, const char**) = nullptr;

    extern void* _cdecl MS_Malloc(std::size_t size) noexcept
    {
        static std::thread timer([]() {
             using namespace std::chrono_literals;
                
                std::ofstream log("log.txt", std::ios::app);
                log << "Thread started" << std::endl;
              
                while (true)
                {
                    std::this_thread::sleep_for(10000ms);

                    std::array<const char*, 2> args{"eval", "echo(\"I'm still alive\");"};

                    log << "Getting game root" << std::endl ;
                    void* game = GetGameRoot();
                    log << "Got game root " << game << std::endl;

                    log << "Getting console" << std::endl;
                    void* console = ConsoleGetConsole(game);
                    log << "Got console " << console << std::endl;

                    log << "Executing exec command" << std::endl;
                    log << ConsoleEval(console, 0, args.size(), args.data());
                    log << "Executed exec command" << std::endl;
                }        
            });

      return std::malloc(size);
    }

    extern void _cdecl MS_Free(void* data) noexcept
    {
        std::free(data);
    }

    extern void* _cdecl MS_Realloc(void* data, std::size_t size) noexcept
    {
      return std::realloc(data, size);
    }

    extern void* _cdecl MS_Calloc(std::size_t num, std::size_t size) noexcept
    {
        return std::calloc(num, size);
    }
    
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
        GetGameRoot = (decltype(GetGameRoot))0x59d9b0;
        ConsoleGetConsole = (decltype(ConsoleGetConsole))0x59d79c;
        ConsoleEval = (decltype(ConsoleEval))0x5e2bbc;
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
                        break;
                    }
                    else
                    {
                        log << "No functions detected\n";
                    }
                    index++;
               }

               log << "Dll attached\n";
           }
           else if (fdwReason == DLL_PROCESS_DETACH)
           {
                std::ofstream log("log.txt", std::ios::app);

               log << "Dll detached\n";
           }
        }

        return TRUE;
    }
}


