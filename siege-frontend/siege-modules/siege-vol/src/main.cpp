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

    auto on_create(const win32::create_message& data)
    {
        RECT parent_size{};

		if (GetClientRect(self, &parent_size))
		{
						
		}

        short height = 20;

        auto toolbar = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | CCS_TOP | TBSTYLE_LIST,
						.x = 0,       
						.y = height,
						.cx = short(parent_size.right),  
						.cy = height       
						}, self, win32::tool_bar::class_name, L"Toolbar");

        std::array<TBBUTTON, 4> buttons{{
            TBBUTTON{.idCommand = 0, .fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE},
            TBBUTTON{.idCommand = 1, .fsStyle = BTNS_DROPDOWN | BTNS_SHOWTEXT | BTNS_AUTOSIZE},
        }};

        buttons[0].iString = SendMessageW(toolbar, TB_ADDSTRING, 0, std::bit_cast<win32::lparam_t>(
            std::array<wchar_t, 6>{"Open"}.data()
        ));

        buttons[1].iString = SendMessageW(toolbar, TB_ADDSTRING, 0, std::bit_cast<win32::lparam_t>(
            std::array<wchar_t, 9>{"Extract"}.data()
        ));

        SendMessageW(toolbar, TB_ADDBUTTONS, wparam_t(buttons.size()), std::bit_cast<win32::lparam_t>(buttons.data()));


        auto table = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | LVS_REPORT | CCS_BOTTOM,
						.x = 0,       
						.y = height,
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
            SendMessageW(table, LVM_SETCOLUMNWIDTH, index, LVSCW_AUTOSIZE);
            index++;
        }

        return 0;
    }

    auto on_notify(win32::toolbar_notify_message notification)
    {
        auto [sender, id, code] = notification;
        auto mapped_result = win32::MapWindowPoints(sender, HWND_DESKTOP, *win32::tool_bar::GetRect(sender, id));

        // create menu here

        win32::menu_builder builder;

        // add menu items
        win32::TrackPopupMenuEx(LoadMenuIndirectW(builder.result()), 0, POINT{mapped_result.second.left, mapped.result.second.bottom}, sender, TPMPARAMS {
            .rcExclude = mapped_result.second
        });

        return 0;
    }

/*
    auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(self, [&](auto child) {
			win32::SetWindowPos(child, sized.client_size);
		});

		return std::nullopt;
	}
*/

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