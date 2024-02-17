#include <win32_controls.hpp>
#include <win32_builders.hpp>
#include <bit>
#include <filesystem>
#include <cassert>


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
        auto parent_size = win32::GetClientRect(self);

        short height = 200;


        auto rebar = win32::CreateWindowExW(win32::window_params<>{
            .parent = self,
            .class_name = win32::rebar::class_Name,
            .style{win32::window_style(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                                        WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_BANDBORDERS) }
        });

        auto toolbar = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | CCS_NOPARENTALIGN | CCS_NORESIZE | TBSTYLE_LIST,   
						}, *rebar, win32::tool_bar::class_name, L"Toolbar");

        assert(toolbar);

        win32::tool_bar::SetExtendedStyle(*toolbar, win32::tool_bar::mixed_buttons | win32::tool_bar::draw_drop_down_arrows);
     
        std::array<TBBUTTON, 2> buttons{{
              TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 0, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT, .iString = INT_PTR(L"Open")},
              TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 1, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_DROPDOWN | BTNS_SHOWTEXT, .iString = INT_PTR(L"Extract")},
         }};

         if (!win32::tool_bar::AddButtons(*toolbar, buttons))
         {
            DebugBreak();       
         }

         auto button_size = win32::tool_bar::GetButtonSize(*toolbar);

        assert(rebar);
        if (!win32::rebar::InsertBand(*rebar, -1, REBARBANDINFOW{
            .fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS,
            .lpText = const_cast<wchar_t*>(L""),
            .hwndChild = *toolbar,
            .cxMinChild = UINT(button_size.cx * 2),
            .cyMinChild = UINT(button_size.cy),
            .cx = UINT(parent_size->right / 3 * 2),
            .cyChild = UINT(button_size.cy)
            }))
        {
            DebugBreak();
        }

        if (!win32::rebar::InsertBand(*rebar, -1, REBARBANDINFOW{
            .fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS,
            .lpText = const_cast<wchar_t*>(L"Search"),
            .hwndChild = [&]{
              auto search = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_BORDER | WS_EX_STATICEDGE | WS_CHILD
						}, *rebar, win32::edit::class_name, L"");

                assert(search);

                win32::edit::SetCueBanner(*search, false, L"Enter search text here");

                return *search;
            }(),
            .cx = UINT(parent_size->right / 3)
            }))
        {
            DebugBreak();
        }

        auto table = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
						.x = 0,       
						.y = short(win32::rebar::GetBarHeight(*rebar) + 2),
						.cx = short(parent_size->right),  
						.cy = short(parent_size->bottom - 2 - win32::rebar::GetBarHeight(*rebar))       
						}, self, win32::list_view::class_name, L"Volume");

        std::array<LVCOLUMNW, 4> columns{{
            LVCOLUMNW{.mask = LVCF_TEXT | LVCF_WIDTH, .cx = parent_size->right / 4, .pszText = const_cast<wchar_t*>(L"Filename")},
            LVCOLUMNW{.mask = LVCF_TEXT | LVCF_WIDTH, .cx = parent_size->right / 4, .pszText = const_cast<wchar_t*>(L"Path")},
            LVCOLUMNW{.mask = LVCF_TEXT | LVCF_WIDTH, .cx = parent_size->right / 4 ,.pszText = const_cast<wchar_t*>(L"Size (in bytes)")},
            LVCOLUMNW{.mask = LVCF_TEXT | LVCF_WIDTH, .cx = parent_size->right / 4 ,.pszText = const_cast<wchar_t*>(L"Compression Method")}
        }};

        auto index = 0;
        for (auto& column : columns)
        {
            SendMessageW(*table, LVM_INSERTCOLUMNW, index, std::bit_cast<win32::lparam_t>(&column));
            SendMessageW(*table, LVM_SETCOLUMNWIDTH, index, LVSCW_AUTOSIZE);
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
        win32::TrackPopupMenuEx(LoadMenuIndirectW(builder.result()), 0, POINT{mapped_result->second.left, mapped_result->second.bottom}, sender, TPMPARAMS {
            .rcExclude = mapped_result->second
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

          info_instance = *win32::CreateWindowExW(CREATESTRUCTW{
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