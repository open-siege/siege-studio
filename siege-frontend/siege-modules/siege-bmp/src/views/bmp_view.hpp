#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

#include <win32_controls.hpp>
#include <win32_com_server.hpp>
#include <cassert>
#include <sstream>
#include <vector>
#include <memory>
#include <oleacc.h>
#include "bmp_controller.hpp"

namespace siege::views
{
	struct bmp_view
    {
        constexpr static auto formats = std::array<std::wstring_view, 16>{{
        L".jpg", L".jpeg", L".gif", L".png", L".tag", L".bmp", L".dib" , L".pba", L".dmb", L".db0", L".db1", L".db2", L".hba", L".hb0", L".hb1", L".hb2"    
        }};

        win32::hwnd_t self;

        bmp_view(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	    {
	    }

        auto on_create(const win32::create_message& info)
        {
            auto mfcModule = GetModuleHandleW(L"siege-mfc.dll");
            assert(mfcModule);

            auto group_box = win32::CreateWindowExW(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                            .cy = 100
						    }, self, win32::button::class_name, L"Colour strategy");

            assert(group_box);


            auto do_nothing = win32::CreateWindowExW(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON
						    }, self, win32::button::class_name, L"Do nothing");

            auto ideal_size = win32::button::GetIdealSize(*do_nothing);
            win32::SetWindowPos(*do_nothing, *ideal_size);

            auto remap = win32::CreateWindowExW(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
						    }, self, win32::button::class_name, L"Remap");

            ideal_size = win32::button::GetIdealSize(*remap);
            win32::SetWindowPos(*remap, *ideal_size);

            auto remap_unique = win32::CreateWindowExW(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
						    }, self, win32::button::class_name, L"Remap (only unique colours)");

            ideal_size = win32::button::GetIdealSize(*remap_unique);
            win32::SetWindowPos(*remap_unique, *ideal_size);

            auto strategy_toolbar = win32::CreateWindowExW(CREATESTRUCTW {
                            .hInstance = mfcModule,
                            .hwndParent = self,
                            .cy = 100,
                            .cx = 300,
                            .y = 1,
						    .style = WS_VISIBLE | WS_CHILD | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
                            .lpszClass = L"MFC::CCheckListBox",
                });

            assert(strategy_toolbar);

            win32::list_box::InsertString(*strategy_toolbar, -1, L"Do nothing");
            win32::list_box::InsertString(*strategy_toolbar, -1, L"Remap");
            win32::list_box::InsertString(*strategy_toolbar, -1, L"Remap (only unique colours)");

            /*auto strategy_toolbar = win32::CreateWindowExW(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | CCS_NOPARENTALIGN | TBSTYLE_LIST,   
                            .cy = 400,
						    }, self, win32::tool_bar::class_name, L"Colour strategy");*/

        

            //win32::tool_bar::SetExtendedStyle(*strategy_toolbar, win32::tool_bar::mixed_buttons | win32::tool_bar::draw_drop_down_arrows);
     
            //std::array<TBBUTTON, 3> buttons{{
            //    TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 0, .fsState = TBSTATE_ENABLED, 
            //                        .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Do nothing")},
            //    TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 1, .fsState = TBSTATE_ENABLED, 
            //                        .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Remap")},
            //    TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 2, .fsState = TBSTATE_ENABLED, 
            //                        .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Remap (only unique colours)")},
            // }};

            //assert(win32::tool_bar::AddButtons(*strategy_toolbar, buttons));

            auto palettes_tree = win32::CreateWindowExW(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_CHECKBOXES, 
                            .y = 2,
                            .cy = 300,
						    }, self, win32::tree_view::class_name, L"Palettes");


            std::wstring temp = L"menu.pal";
        
            auto parent_size = win32::GetClientSize(self);

            auto root_item = win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
                .hInsertAfter = TVI_ROOT,
                .itemex = {
                    .pszText = temp.data(),
                    .cChildren = 1
                    }
                });

            win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
                .hParent = *root_item,
                .hInsertAfter = TVI_LAST,
                .itemex = {
                    .pszText = const_cast<wchar_t*>(L"palette 1")
                    }
                });

            win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
                .hParent = *root_item,
                .hInsertAfter = TVI_LAST,
                .itemex = {
                    .pszText = const_cast<wchar_t*>(L"palette 2")
                    }
                });

            win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
                .hParent = *root_item,
                .hInsertAfter = TVI_LAST,
                .itemex = {
                    .pszText = const_cast<wchar_t*>(L"palette 3")
                    }
                });

            win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
                .hParent = *root_item,
                .hInsertAfter = TVI_LAST,
                .itemex = {
                    .pszText = const_cast<wchar_t*>(L"palette 4")
                    }
                });

            root_item = win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
                .hInsertAfter = TVI_ROOT,
                .itemex = {
                    .pszText = const_cast<wchar_t*>(L"test.dpl")
                    }
                });

            root_item = win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
                .hInsertAfter = TVI_ROOT,
                .itemex = {
                    .pszText = const_cast<wchar_t*>(L"ui.ppl")
                    }
                });

            root_item = win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
                .hInsertAfter = TVI_ROOT,
                .itemex = {
                    .pszText = const_cast<wchar_t*>(L"other.pal")
                    }
                });

            // TODO add example palette file names as root items and then palette names as children

            auto palettes_list = win32::CreateWindowExW(CREATESTRUCTW{
                            .hInstance = mfcModule,
                            .hwndParent = self,
						    .cy = 300,  
                            .cx = 400,
	            		    .y = 3,		
                            .style = WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                            .lpszName =  L"Palettes",
                            .lpszClass = L"MFC::CMFCListCtrl"
                });


            win32::list_view::SetView(*palettes_list, win32::list_view::view_type::details_view);
            assert(*win32::list_view::EnableGroupView(*palettes_list, true));
            assert(win32::list_view::SetTileViewInfo(*palettes_list, LVTILEVIEWINFO {
                .dwFlags = LVTVIF_FIXEDWIDTH,
                .sizeTile = SIZE {.cx = win32::GetClientSize(self)->cx, .cy = 50},
                }));
        
            win32::list_view::SetExtendedListViewStyle(*palettes_list, 0, 
                    LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

            win32::list_view::InsertColumn(*palettes_list, -1, LVCOLUMNW {
                 .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L""),
                .cxMin = 100,
                .cxDefault = win32::GetClientSize(self)->cx,
                });

            assert(win32::list_view::InsertGroup(*palettes_list, -1, LVGROUP {
                .pszHeader = temp.data(),
                .iGroupId = 1
                }) == 0);

            win32::list_view::InsertGroup(*palettes_list, -1, LVGROUP {
                .pszHeader = const_cast<wchar_t*>(L"test.dpl"),
                .iGroupId = 2
                });

            win32::list_view::InsertGroup(*palettes_list, -1, LVGROUP {
                .pszHeader = const_cast<wchar_t*>(L"ui.pal"),
                .iGroupId = 3
                });

            assert(win32::list_view::InsertItem(*palettes_list, -1, LVITEMW{
                .pszText = temp.data(),
                .iGroupId = 1
                }) == 0);

            win32::list_view::InsertItem(*palettes_list, -1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 2"),
                .iGroupId = 1
                });

            win32::list_view::InsertItem(*palettes_list, -1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 3"),
                .iGroupId = 1
                });

            win32::list_view::InsertItem(*palettes_list, -1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 1"),
                .iGroupId = 2
                });

            win32::list_view::InsertItem(*palettes_list, -1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 2"),
                .iGroupId = 2
                });

            win32::list_view::InsertItem(*palettes_list, -1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 3"),
                .iGroupId = 2
                });


            win32::list_view::InsertItem(*palettes_list, -1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 1"),
                .iGroupId = 3
                });

            win32::list_view::InsertItem(*palettes_list, -1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 2"),
                .iGroupId = 3
                });

            auto palettes_button =  win32::CreateWindowExW(CREATESTRUCTW{
                            .hMenu = [] {
                                auto menu = CreatePopupMenu();

                                int id = 1;

                                auto menu2 = CreatePopupMenu();

                                AppendMenuW(menu2, MF_CHECKED | MF_STRING, id++, L"palette 1");
                                AppendMenuW(menu2, MF_UNCHECKED | MF_STRING, id++, L"palette 2");
                                AppendMenuW(menu2, MF_UNCHECKED | MF_STRING, id++, L"palette 3");


                                AppendMenuW(menu, MF_CHECKED | MF_STRING | MF_POPUP, reinterpret_cast<INT_PTR>(menu2), L"menu.pal");
                                AppendMenuW(menu, MF_SEPARATOR , id++, nullptr);
                                AppendMenuW(menu, MF_UNCHECKED | MF_STRING, id++, L"test.dpl");
                            
                                return menu;
                            }(),            
                            .hwndParent = self,                        
                            .cy = 100,
						    .style = WS_CHILD | BS_SPLITBUTTON,
                            .lpszName =  L"Palettes",
                            .lpszClass = win32::button::class_name,
            });


          //  // TODO add example palette file names as groups and then palette names as items

          //  auto static_image = win32::CreateWindowExW(DLGITEMTEMPLATE{
						    //.style = WS_VISIBLE | WS_CHILD,
          //                  .y = 4,
          //                  .cy = 300,
						    //}, self, win32::static_control::class_name, L"Image");

            auto children = std::array{*group_box, *strategy_toolbar, *palettes_tree, *palettes_list};
            win32::StackChildren(*win32::GetClientSize(self), children);

            auto rect = win32::GetClientRect(*group_box);
            rect->top += 15;
            rect->left += 5;

            auto radios = std::array{*do_nothing, *remap, *remap_unique};
            win32::StackChildren(SIZE{.cx = rect->right, .cy = rect->bottom - 20}, radios, win32::StackDirection::Horizontal,
                    POINT{.x = rect->left, .y = rect->top});

            return 0;
        }

        std::optional<LRESULT> on_get_object(win32::get_object_message message)
        {
            if (message.object_id == OBJID_NATIVEOM)
            {
                auto collection = std::make_unique<win32::com::OwningCollection<IStream>>();

                return LresultFromObject(__uuidof(IDispatch), message.flags, static_cast<IDispatch*>(collection.release()));   
            }

            return std::nullopt;
        }

        auto on_message(win32::message message)
        {
            if (message.message == WM_MEASUREITEM || message.message == WM_DRAWITEM)
            {
                win32::ForEachDirectChildWindow(self, [&](auto child) {

            
		        });
        
            }
		    return std::nullopt;
        }

        auto on_command(win32::command_message message)
        {
            auto sender = message.sender;

            auto menu = GetMenu(sender);

            if (menu)
            {
                auto rect = win32::GetClientRect(sender);

                POINT pos {
                    .x = rect->left,
                    .y = rect->top
                };

                ClientToScreen(sender, &pos);
                TrackPopupMenu(menu, TPM_LEFTALIGN, pos.x, pos.y + rect->bottom,  0, sender, nullptr);
            }

            return std::nullopt;
        }

        auto on_size(win32::size_message sized)
	    {
		    win32::ForEachDirectChildWindow(self, [&](auto child) {

    //			win32::SetWindowPos(child, sized.client_size);
		    });
		    return std::nullopt;
	    }

        static bool is_bitmap(std::istream& raw_data)
        {
            return false;
        }
    };
}

#endif