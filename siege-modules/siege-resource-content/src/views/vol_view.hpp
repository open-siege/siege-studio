#ifndef VOL_VIEW_HPP
#define VOL_VIEW_HPP

#include <siege/platform/win/desktop/win32_common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/win32_menu.hpp>
#include <spanstream>
#include "vol_controller.hpp"

namespace siege::views
{
	struct vol_view : win32::window
    {
        constexpr static auto formats = std::array<std::wstring_view, 20>{{
            L".vol", L".rmf", L".mis", L".map", L".rbx", L".tbv" , L".zip", L".vl2", L".pk3",
            L".iso", L".mds", L".cue", L".nrg", L".7z", L".tgz", L".rar", L".cab", L".z", L".cln", L".atd"    
        }};

        vol_controller controller;

        vol_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window(self)
	    {
	    }

        auto on_create(const win32::create_message& data)
        {
            auto factory = win32::window_factory(win32::window_ref(*this));

            auto root = factory.CreateWindowExW<win32::rebar>(::CREATESTRUCTW{
                .style{WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                                            WS_CLIPCHILDREN | CCS_TOP | 
                                            CCS_VERT | RBS_AUTOSIZE | RBS_FIXEDORDER | RBS_VERTICALGRIPPER },
                .lpszClass = win32::rebar::class_Name,
            });

            assert(root);

            auto rebar = factory.CreateWindowExW<win32::rebar>(::CREATESTRUCTW{
                .style{WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                    WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_BANDBORDERS},
                .lpszClass = win32::rebar::class_Name,
            });

            auto toolbar = factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{
						    .style = WS_VISIBLE | WS_CHILD | CCS_NOPARENTALIGN | CCS_NORESIZE | TBSTYLE_LIST,   
						    });

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
                });

            rebar->InsertBand(-1, REBARBANDINFOW{
                .fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS,
                .lpText = const_cast<wchar_t*>(L"Search"),
                .hwndChild = [&]{
                  auto search = factory.CreateWindowExW<win32::edit>(CREATESTRUCTW{
						    .style = WS_VISIBLE | WS_BORDER | WS_EX_STATICEDGE | WS_CHILD
						    });

                    assert(search);
                    search->SetCueBanner(false, L"Enter search text here");

                    SendMessageW(*search, CCM_SETWINDOWTHEME , 0, reinterpret_cast<win32::lparam_t>(L"SearchBoxEdit"));
                    return search->release();
                }()
                });


            auto table = factory.CreateWindowExW<win32::list_view>(CREATESTRUCTW{
						    .style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
						    }); 

            table->InsertColumn(-1, LVCOLUMNW {
                    .pszText = const_cast<wchar_t*>(L"Filename"),
            });

            table->InsertColumn(-1, LVCOLUMNW {
                    .pszText = const_cast<wchar_t*>(L"Path"),
            });

            table->InsertColumn(-1, LVCOLUMNW {
                    .pszText = const_cast<wchar_t*>(L"Size (in bytes)"),
            });

            table->InsertColumn(-1, LVCOLUMNW {
                  .pszText = const_cast<wchar_t*>(L"Compression Method"),
            });

            root->InsertBand(-1, REBARBANDINFOW {
                .fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE | RBBS_TOPALIGN,
                .hwndChild = *rebar
                });

            root->InsertBand(-1, REBARBANDINFOW {
                .fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE | RBBS_TOPALIGN,
                .hwndChild = *table
                });
            return 0;
        }

        auto on_size(win32::size_message sized)
	    {
		    return std::nullopt;
	    }

        auto on_copy_data(win32::copy_data_message<char> message)
		{
			std::spanstream stream(message.data);
			
			if (vol_controller::is_vol(stream))
			{
                std::optional<std::filesystem::path> path;

                if (wchar_t* filename = this->GetPropW<wchar_t*>(L"Filename"); filename)
                {
                    path = filename;
                }

                auto count = controller.load_volume(stream, path);

                if (count > 0)
                {
                    return TRUE;
                }

                return FALSE;
			}

			return FALSE;
		}
    };

}

#endif