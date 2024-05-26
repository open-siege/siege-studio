#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/win/core/module.hpp>

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
    
    constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 4> verification_strings = {{
    std::array<std::pair<std::string_view, std::size_t>, 4>{{
        {std::string_view("cls"), std::size_t(0x6fb741)},
        {std::string_view("trace"), std::size_t(0x6fb7af)},
        {std::string_view("Console::logBufferEnabled"), std::size_t(0x6fb7b5)},
        {std::string_view("Console::logMode"), std::size_t(0x6fb7fa)},
        }},
    std::array<std::pair<std::string_view, std::size_t>, 4>{{
        {std::string_view("cls"), std::size_t(0x6fe551)},
        {std::string_view("trace"), std::size_t(0x6fe5bf)},
        {std::string_view("Console::logBufferEnabled"), std::size_t(0x6fe5c5)},
        {std::string_view("Console::logMode"), std::size_t(0x6fe60a)},
        }},        
    std::array<std::pair<std::string_view, std::size_t>, 4>{{
        {std::string_view("cls"), std::size_t(0x712cf9)},
        {std::string_view("trace"), std::size_t(0x712d67)},
        {std::string_view("Console::logBufferEnabled"), std::size_t(0x712d6d)},
        {std::string_view("Console::logMode"), std::size_t(0x712db2)},
        }},
    std::array<std::pair<std::string_view, std::size_t>, 4>{{
        {std::string_view("cls"), std::size_t(0x723169)},
        {std::string_view("trace"), std::size_t(0x7231d7)},
        {std::string_view("Console::logBufferEnabled"), std::size_t(0x7231dd)},
        {std::string_view("Console::logMode"), std::size_t(0x723222)},
        }}
    }};

    inline void set_1000_exports()
    {
        ExportModuleIsValid = -1;
        GetGameRoot = (decltype(GetGameRoot))0x58eff8;
        ConsoleGetConsole = (decltype(ConsoleGetConsole))0x58ede4;
        ConsoleEval = (decltype(ConsoleEval))0x5d3d00;
    }

    inline void set_1002_exports()
    {
        ExportModuleIsValid = -1;
        GetGameRoot = (decltype(GetGameRoot))0x598968;
        ConsoleGetConsole = (decltype(ConsoleGetConsole))0x598754;
        ConsoleEval = (decltype(ConsoleEval))0x5d4dd8;
    }

    inline void set_1003_exports()
    {
        ExportModuleIsValid = -1;
        GetGameRoot = (decltype(GetGameRoot))0x59d9b0;
        ConsoleGetConsole = (decltype(ConsoleGetConsole))0x59d79c;
        ConsoleEval = (decltype(ConsoleEval))0x5e2bbc;
    }

    inline void set_1004_exports()
    {
        ExportModuleIsValid = -1;
        GetGameRoot = (decltype(GetGameRoot))0x5a1558;
        ConsoleGetConsole = (decltype(ConsoleGetConsole))0x5a0fb8;
        ConsoleEval = (decltype(ConsoleEval))0x5e6460;
    }

    constexpr std::array<void(*)(), 4> export_functions = {{
            set_1000_exports,
            set_1002_exports,
            set_1003_exports,
            set_1004_exports
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
               try
               {
                   auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

                   for (const auto& item : verification_strings)
                   {
                       win32::module_ref temp((void*)item[0].second);

                       if (temp != app_module)
                       {
                           continue;
                       }

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
                   }
                   catch(...)
                   {
                        log << "Exception thrown\n";               
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


