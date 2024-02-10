#include <win32_controls.hpp>
#include <bit>
#include <filesystem>

struct bitmap_window
{
    constexpr static std::u8string_view formats = u8".jpg .jpeg .gif .png .tag .bmp .dib .pba .dmb .db0 .db1 .db2 .hba .hb0 .hb1 .hb2";

    bitmap_window(win32::hwnd_t self, const CREATESTRUCTW&)
	{
	}

    static bool is_bitmap(std::istream& raw_data)
    {
        return false;
    }
};

struct pal_window
{
    constexpr static std::u8string_view formats = u8".pal .ipl .ppl .dpl";

    pal_window(win32::hwnd_t self, const CREATESTRUCTW&)
	{
	}

    static bool is_pal(std::istream& raw_data)
    {
        return false;
    }
};

struct pal_mapping_window
{
    constexpr static std::u8string_view formats = u8"palettes.settings.json";

    pal_mapping_window(win32::hwnd_t self, const CREATESTRUCTW&)
	{
	}
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


struct bmp_module
{
    HINSTANCE module_instance;
    std::uint32_t is_supported_id;

    bmp_module(win32::hwnd_t self, const CREATESTRUCTW& args)
    {
        module_instance = args.hInstance;
        win32::RegisterClassExW<bitmap_window>(WNDCLASSEXW{
            .hInstance = module_instance
            });
        win32::RegisterClassExW<pal_window>(WNDCLASSEXW{
            .hInstance = module_instance
            });
        win32::RegisterClassExW<pal_mapping_window>(WNDCLASSEXW{
            .hInstance = module_instance
            });

        SetPropW(self, win32::type_name<bitmap_window>().c_str(), std::bit_cast<void*>(bitmap_window::formats.data()));
        SetPropW(self, win32::type_name<pal_window>().c_str(), std::bit_cast<void*>(pal_window::formats.data()));
        SetPropW(self, win32::type_name<pal_mapping_window>().c_str(), std::bit_cast<void*>(pal_mapping_window::formats.data()));

        SetPropW(self, L"All Images", std::bit_cast<void*>(bitmap_window::formats.data()));
        SetPropW(self, L"All Palettes", std::bit_cast<void*>(pal_window::formats.data()));

        is_supported_id = RegisterWindowMessageW(L"is_supported_message");
    }

    ~bmp_module()
    {
       win32::UnregisterClassW<bitmap_window>(module_instance);
       win32::UnregisterClassW<pal_window>(module_instance);
       win32::UnregisterClassW<pal_mapping_window>(module_instance);
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

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{

    if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
    {
        if (lpvReserved != nullptr)
        {
            return TRUE; // do not do cleanup if process termination scenario
        }

        static win32::hwnd_t info_instance = nullptr;

        static std::wstring module_file_name(255, '\0');
        GetModuleFileNameW(hinstDLL, module_file_name.data(), module_file_name.size());

       std::filesystem::path module_path(module_file_name.data());


       if (fdwReason == DLL_PROCESS_ATTACH)
       {
           win32::RegisterStaticClassExW<bmp_module>(WNDCLASSEXW{
                  .hInstance = hinstDLL,
                  .lpszClassName = module_path.stem().c_str()
           });

          info_instance = win32::CreateWindowExW(CREATESTRUCTW{
                .hInstance = hinstDLL,
                .hwndParent = HWND_MESSAGE,
                .lpszClass = module_path.stem().c_str()
            });
        }
        else if (fdwReason == DLL_PROCESS_DETACH)
        {
            DestroyWindow(info_instance);
            UnregisterClassW(module_path.stem().c_str(), hinstDLL);
        }
    }

    return TRUE;
}