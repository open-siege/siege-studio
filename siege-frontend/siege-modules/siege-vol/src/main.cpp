#include <win32_controls.hpp>
#include <bit>
#include <filesystem>

struct volume_window
{
    constexpr static std::u8string_view formats = 
            u8".vol .rmf .mis .rmf .map .rbx .tbv .zip .vl2 .pk3 .iso .mds .cue .nrg .7z .tgz .rar .cab .z .cln .atd";

    win32::hwnd_t self;

    volume_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

    auto on_create(const win32::create_message&)
    {
        RECT parent_size{};

		if (GetClientRect(self, &parent_size))
		{
						
		}

        auto table = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
						.x = 0,       
						.y = 0,
						.cx = short(parent_size.right),  
						.cy = short(parent_size.bottom)       
						}, self, win32::list_view::class_name, L"Volume");

        std::array<LVCOLUMNW, 4> columns{{
            LVCOLUMNW{.mask = LVCF_TEXT | LVCF_WIDTH, .cx = parent_size.right / 4, .pszText = const_cast<wchar_t*>(L"Filename")},
            LVCOLUMNW{.mask = LVCF_TEXT | LVCF_WIDTH, .cx = parent_size.right / 4, .pszText = const_cast<wchar_t*>(L"Path")},
            LVCOLUMNW{.mask = LVCF_TEXT | LVCF_WIDTH, .cx = parent_size.right / 4 ,.pszText = const_cast<wchar_t*>(L"Size (in bytes)")},
            LVCOLUMNW{.mask = LVCF_TEXT | LVCF_WIDTH, .cx = parent_size.right / 4 ,.pszText = const_cast<wchar_t*>(L"Compression Method")}
    }};

        auto index = 0;
        for (auto& column : columns)
        {
            SendMessageW(table, LVM_INSERTCOLUMNW, index, std::bit_cast<win32::lparam_t>(&column));
            index++;
        }

        return 0;
    }

    static bool is_bitmap(std::istream& raw_data)
    {
        return false;
    }
};

struct vol_module
{
    HINSTANCE module_instance;
    std::uint32_t is_supported_id;

    vol_module(win32::hwnd_t self, const CREATESTRUCTW& args)
    {
        module_instance = args.hInstance;
        win32::RegisterClassExW<volume_window>(WNDCLASSEXW{
            .hInstance = module_instance
            });

        SetPropW(self, win32::type_name<volume_window>().c_str(), std::bit_cast<void*>(volume_window::formats.data()));

        is_supported_id = RegisterWindowMessageW(L"is_supported_message");
    }

    ~vol_module()
    {
       win32::UnregisterClassW<volume_window>(module_instance);
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
           win32::RegisterStaticClassExW<vol_module>(WNDCLASSEXW{
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