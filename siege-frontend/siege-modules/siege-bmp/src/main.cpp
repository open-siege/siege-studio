#include <win32_controls.hpp>

struct bitmap_window
{
    constexpr static std::u8string_view formats = u8".jpg .jpeg .gif .png .tag .bmp .dib .pba .dmb .db0 .db1 .db2 .hba .hb0 .hb1 .hb2";

    static bool is_bitmap(std::istream& raw_data)
    {

    }
};

struct pal_window
{
    constexpr static std::u8string_view formats = ".pal .ipl .ppl .dpl";

    static bool is_pal(std::istream& raw_data)
    {
        std::array<std::byte, 4> header{};
        studio::read(raw_data, header.data(), sizeof(header));

        raw_data.seekg(-int(sizeof(header)), std::ios::cur);

        return header == dba_tag;
    }
};

struct pal_mapping_window
{
    constexpr static std::u8string_view formats = "palettes.settings.json";
};


struct is_supported_message
{
    enum class handle_type
    {
        path,
        file,
        view
    } type;

    int size;
};

struct module_info
{
    std::uint32_t is_supported_id;

    module_info(win32::hwnd_t self, const CREATESTRUCTW&)
    {
        SetPropA(self, typeid(bitmap_window).name(), bitmap_window::formats.data());
        SetPropA(self, typeid(pal_window).name(), pal_window::formats.data());
        SetPropA(self, typeid(pal_mapping_window).name(), pal_mapping_window::formats.data());

        SetPropW(self, L"All Images", bitmap_window::formats.data());
        SetPropW(self, L"All Palettes", pal_window::formats.data());

        is_supported_id = RegisterWindowMessageW(L"is_supported_message");
    }

    std::optional<win32::lresult_t> on_is_supported(is_supported_message message)
    {
        if (message.type == is_supported_message::handle_type::path)
        {
            return FALSE;
        }
        else if (message.type == is_supported_message::handle_type::file)
        {
            return FALSE;
        }
        else if (message.type == is_supported_message::handle_type::view)
        {
            return FALSE;
        }
    }
};



__declspec(dllexport) auto* name = "siege-bmp"; 


extern "C" BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:

        win32::RegisterClassExW<bitmap_window>(WNDCLASSEXW{});
        win32::RegisterClassExW<pal_window>(WNDCLASSEXW{});
        win32::RegisterClassExW<pal_mapping_window>(WNDCLASSEXW{});

        win32::RegisterStaticClassExW<module_info>(WNDCLASSEXW{});

        static auto info_instance = win32::CreateWindowExW(CREATESTRUCTW{
            .hwndParent = HWND_MESSAGE
        });
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
        
            if (lpvReserved != nullptr)
            {
                break; // do not do cleanup if process termination scenario
            }

        DestroyWindow(info_instance);
        win32::UnregisterClassW<module_info>(hinstDLL);

        win32::UnregisterClassW<bitmap_window>(hinstDLL);
        win32::UnregisterClassW<pal_window>(hinstDLL);
        win32::UnregisterClassW<pal_mapping_window>(hinstDLL);
            
         // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}