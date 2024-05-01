#include <siege/platform/win/desktop/win32_controls.hpp>
#include <bit>
#include <filesystem>
#include <cassert>
#include <atomic>
#include <siege/platform/win/core/com_collection.hpp>
#include <siege/platform/win/core/com_stream_buf.hpp>

struct volume_window : win32::window
{
    constexpr static auto formats = std::array<std::wstring_view, 20>{{
        L".vol", L".rmf", L".mis", L".map", L".rbx", L".tbv" , L".zip", L".vl2", L".pk3",
        L".iso", L".mds", L".cue", L".nrg", L".7z", L".tgz", L".rar", L".cab", L".z", L".cln", L".atd"    
    }};

    volume_window(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window(self)
	{
	}

    auto on_create(const win32::create_message& data)
    {
        auto parent_size = this->GetClientRect();

        auto root = win32::CreateWindowExW<win32::rebar>(win32::window_params<>{
            .parent = *this,
            .class_name = win32::rebar::class_Name,
            .style{win32::window_style(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                                        WS_CLIPCHILDREN | CCS_TOP | 
                                        CCS_VERT | RBS_AUTOSIZE | RBS_FIXEDORDER | RBS_VERTICALGRIPPER) }
        });

        assert(root);

        auto rebar = win32::CreateWindowExW<win32::rebar>(win32::window_params<>{
            .parent = *root,
            .class_name = win32::rebar::class_Name,
            .style{win32::window_style(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_BANDBORDERS)}
        });

        auto toolbar = win32::CreateWindowExW<win32::tool_bar>(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | CCS_NOPARENTALIGN | CCS_NORESIZE | TBSTYLE_LIST,   
						}, *rebar, win32::tool_bar::class_name, L"Toolbar");

        assert(toolbar);

        toolbar->SetExtendedStyle(win32::tool_bar::mixed_buttons | win32::tool_bar::draw_drop_down_arrows);
     
        std::array<TBBUTTON, 2> buttons{{
              TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 0, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT, .iString = INT_PTR(L"Open")},
              TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 1, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_DROPDOWN | BTNS_SHOWTEXT, .iString = INT_PTR(L"Extract")},
         }};

         if (!toolbar->AddButtons(buttons))
         {
            DebugBreak();       
         }

         auto button_size = toolbar->GetButtonSize();

        assert(rebar);
        rebar->InsertBand(-1, REBARBANDINFOW{
            .fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS,
            .lpText = const_cast<wchar_t*>(L""),
            .hwndChild = *toolbar,
            .cxMinChild = UINT(button_size.cx * 2),
            .cyMinChild = UINT(button_size.cy),
            .cx = UINT(parent_size->right / 3 * 2),
            .cyChild = UINT(button_size.cy)
            });

        rebar->InsertBand(-1, REBARBANDINFOW{
            .fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS,
            .lpText = const_cast<wchar_t*>(L"Search"),
            .hwndChild = [&]{
              auto search = win32::CreateWindowExW<win32::edit>(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_BORDER | WS_EX_STATICEDGE | WS_CHILD
						}, *rebar, win32::edit::class_name, L"");

                assert(search);
                search->SetCueBanner(false, L"Enter search text here");

                SendMessageW(*search, CCM_SETWINDOWTHEME , 0, reinterpret_cast<win32::lparam_t>(L"SearchBoxEdit"));
                return search->release();
            }(),
            .cx = UINT(parent_size->right / 8)
            });


        auto table = win32::CreateWindowExW<win32::list_view>(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
				//		.x = 0,       
				//		.y = short(win32::GetClientRect(*rebar)->bottom + 2),
				//		.cx = short(parent_size->right),  
				//		.cy = short(parent_size->bottom - 2 - win32::rebar::GetBarHeight(*rebar))       
						}, *root, win32::list_view::class_name, L"Volume");

        // TODO: make table columns have a split button
        table->InsertColumn(-1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Filename"),
                .cxMin = parent_size->right / 10
        });

        table->InsertColumn(-1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Path"),
                .cxMin = parent_size->right / 10,
        });

        table->InsertColumn(-1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Size (in bytes)"),
                .cxMin = parent_size->right / 10,
        });

        table->InsertColumn(-1, LVCOLUMNW {
              .cx = LVSCW_AUTOSIZE,
              .pszText = const_cast<wchar_t*>(L"Compression Method"),
              .cxMin = parent_size->right / 10
        });

        auto min_height = parent_size->bottom / 10;
        auto min_width = parent_size->right / 2;

       // auto rebar_rect = win32::GetClientRect(*toolbar);

        root->InsertBand(-1, REBARBANDINFOW {
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

        root->InsertBand(-1, REBARBANDINFOW {
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
        auto mapped_result = win32::MapWindowPoints(sender, HWND_DESKTOP, *win32::tool_bar(sender).GetRect(id));

        // TODO: create and add menu items
        win32::TrackPopupMenuEx(LoadMenuIndirectW(nullptr), 0, POINT{mapped_result->second.left, mapped_result->second.bottom}, sender, TPMPARAMS {
            .rcExclude = mapped_result->second
        });

        return 0;
    }


   auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(*this, [&](auto child) {            
            for (auto i = 0; i < win32::rebar(child).GetBandCount(); ++i)
            {
                auto info = win32::rebar(child).GetBandChildSize(i);

                if (info)
                {
                    info->cyMinChild = sized.client_size.cx;
                    info->cyChild = sized.client_size.cx;
                    win32::rebar(child).SetBandInfo(i, std::move(*info));
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

extern "C"
{
    HRESULT __stdcall GetSupportedExtensions(_Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static std::vector<std::wstring_view> supported_extensions = []{
                std::vector<std::wstring_view> extensions;
                extensions.reserve(32);

                std::copy(volume_window::formats.begin(), volume_window::formats.end(), std::back_inserter(extensions));
              
                return extensions;
            }();

        *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view>>(supported_extensions).release();

        return S_OK;
    }

    HRESULT __stdcall GetSupportedFormatCategories(_In_ LCID, _Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static auto categories = std::array<std::wstring_view, 2> {{
            L"All Archives"
        }};

        *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(categories)>>(categories).release();

        return S_OK;
    }

    HRESULT __stdcall GetSupportedExtensionsForCategory(_In_ const wchar_t* category, _Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!category)
        {
            return E_INVALIDARG;
        }

        if (!formats)
        {
            return E_POINTER;
        }

        std::wstring_view category_str = category;

        if (category_str == L"All Archives")
        {
            *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(volume_window::formats)>>(volume_window::formats).release();
        }
        else
        {
            *formats = std::make_unique<win32::com::OwningCollection<std::wstring_view>>().release();
        }

        return S_OK;
    }

    HRESULT __stdcall IsStreamSupported(_In_ IStream* data) noexcept
    {
        if (!data)
        {
            return E_INVALIDARG;
        }

        win32::com::StreamBufRef buffer(*data);
        std::istream stream(&buffer);

        return S_FALSE;
    }

    _Success_(return == S_OK || return == S_FALSE)
    HRESULT __stdcall GetWindowClassForStream(_In_ IStream* data, _Outptr_ wchar_t** class_name) noexcept
    {
        if (!data)
        {
            return E_INVALIDARG;
        }

        if (!class_name)
        {
            return E_POINTER;
        }

        static std::wstring empty;
        *class_name = empty.data();
        
        win32::com::StreamBufRef buffer(*data);
        std::istream stream(&buffer);

        static HMODULE hModule = []{
            HMODULE temp = nullptr;
            GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
               (LPCWSTR)&GetWindowClassForStream, &temp);

            return temp;
            }();

        if(!hModule)
        {
            return S_FALSE;        
        }

        thread_local WNDCLASSEXW info{};


        return S_FALSE;
    }

    
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
               win32::RegisterClassExW<volume_window>(WNDCLASSEXW{
                .hInstance = hinstDLL
                });
            }
            else if (fdwReason == DLL_PROCESS_DETACH)
            {
               win32::UnregisterClassW<volume_window>(hinstDLL);
            }
        }

        return TRUE;
    }
}