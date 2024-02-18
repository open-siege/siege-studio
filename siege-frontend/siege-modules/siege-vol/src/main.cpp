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

        auto root = win32::CreateWindowExW(win32::window_params<>{
            .parent = self,
            .class_name = win32::rebar::class_Name,
            .style{win32::window_style(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                                        WS_CLIPCHILDREN | CCS_TOP | 
                                        CCS_VERT | RBS_AUTOSIZE | RBS_FIXEDORDER | RBS_VERTICALGRIPPER) }
        });

        assert(root);

        auto rebar = win32::CreateWindowExW(win32::window_params<>{
            .parent = *root,
            .class_name = win32::rebar::class_Name,
            .style{win32::window_style(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_BANDBORDERS)}
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
        win32::rebar::InsertBand(*rebar, -1, REBARBANDINFOW{
            .fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS,
            .lpText = const_cast<wchar_t*>(L""),
            .hwndChild = *toolbar,
            .cxMinChild = UINT(button_size.cx * 2),
            .cyMinChild = UINT(button_size.cy),
            .cx = UINT(parent_size->right / 3 * 2),
            .cyChild = UINT(button_size.cy)
            });

        win32::rebar::InsertBand(*rebar, -1, REBARBANDINFOW{
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
            });

        auto table = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
				//		.x = 0,       
				//		.y = short(win32::GetClientRect(*rebar)->bottom + 2),
				//		.cx = short(parent_size->right),  
				//		.cy = short(parent_size->bottom - 2 - win32::rebar::GetBarHeight(*rebar))       
						}, *root, win32::list_view::class_name, L"Volume");

        // TODO: make table columns have a split button
         win32::list_view::InsertColumn(*table, -1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Filename"),
                .cxMin = parent_size->right / 10
        });

        win32::list_view::InsertColumn(*table, -1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Path"),
                .cxMin = parent_size->right / 10,
        });

        win32::list_view::InsertColumn(*table, -1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Size (in bytes)"),
                .cxMin = parent_size->right / 10,
        });

        win32::list_view::InsertColumn(*table, -1, LVCOLUMNW {
              .cx = LVSCW_AUTOSIZE,
              .pszText = const_cast<wchar_t*>(L"Compression Method"),
              .cxMin = parent_size->right / 10
        });


        auto min_height = parent_size->bottom / 10;
        auto min_width = parent_size->right / 2;

       // auto rebar_rect = win32::GetClientRect(*toolbar);

        win32::rebar::InsertBand(*root, -1, REBARBANDINFOW {
            .fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE | RBBS_TOPALIGN,
            .hwndChild = *rebar,  
            .cxMinChild = UINT(button_size.cy), // min height
            .cyMinChild = UINT(button_size.cx), // min width
            .cx = UINT(button_size.cy), // default height
   //         .cyChild = UINT(parent_size->right), // default width
            .cyChild = UINT(parent_size->right),
       //     .cxIdeal = UINT(parent_size->right - 20),
         //  .cx = UINT(parent_size->right - 100),
            });

        win32::rebar::InsertBand(*root, -1, REBARBANDINFOW {
            .fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE | RBBS_TOPALIGN,
            .hwndChild = *table,
            .cxMinChild = UINT(min_height), // min height
            .cyMinChild = UINT(parent_size->right), // min width
            .cx = UINT(parent_size->bottom - button_size.cy), // default height
            .cyChild = UINT(parent_size->right),
    //        .cxIdeal = UINT(parent_size->right - 20)
       //     .cx = UINT(parent_size->right - 100),
            });


    //    win32::rebar::MaximizeBand(*root, 0, parent_size->right - 100);
 //       win32::rebar::MaximizeBand(*root, 1, parent_size->right - 100);

        return 0;
    }

    auto on_notify(win32::toolbar_notify_message notification)
    {
        auto [sender, id, code] = notification;
        auto mapped_result = win32::MapWindowPoints(sender, HWND_DESKTOP, *win32::tool_bar::GetRect(sender, id));

        win32::menu_builder builder;

        // TODO: create and add menu items
        win32::TrackPopupMenuEx(LoadMenuIndirectW(builder.result()), 0, POINT{mapped_result->second.left, mapped_result->second.bottom}, sender, TPMPARAMS {
            .rcExclude = mapped_result->second
        });

        return 0;
    }


   auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(self, [&](auto child) {            
            for (auto i = 0; i < win32::rebar::GetBandCount(child); ++i)
            {
                auto info = win32::rebar::GetBandChildSize(child, i);

                if (info)
                {
                    info->cyMinChild = sized.client_size.cx;
                    info->cyChild = sized.client_size.cx;
                    win32::rebar::SetBandInfo(child, i, std::move(*info));
                }
            }
		});

		return std::nullopt;
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