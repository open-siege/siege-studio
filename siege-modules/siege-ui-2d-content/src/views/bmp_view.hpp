#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

#include <siege/platform/win/desktop/win32_controls.hpp>
#include <siege/platform/win/desktop/win32_builders.hpp>
#include <siege/platform/win/auto_handle.hpp>
#include <siege/platform/win/core/com_collection.hpp>
#include <cassert>
#include <sstream>
#include <vector>
#include <memory>
#include <istream>
#include <spanstream>
#include <oleacc.h>
#include "bmp_controller.hpp"

namespace siege::views
{
 
    struct gdi_deleter
    {
        void operator()(HGDIOBJ gdi_obj)
        {
            assert(::DeleteObject(gdi_obj) == TRUE);
        }
    };

    using gdi_bitmap = win32::auto_handle<HBITMAP, gdi_deleter>;
    using gdi_brush = win32::auto_handle<HBRUSH, gdi_deleter>;
    using gdi_palette = win32::auto_handle<HPALETTE, gdi_deleter>;
    using gdi_pen = win32::auto_handle<HPEN, gdi_deleter>;
    using gdi_font = win32::auto_handle<HFONT, gdi_deleter>;

	struct bmp_view
    {
        constexpr static auto formats = std::array<std::wstring_view, 16>{{
        L".jpg", L".jpeg", L".gif", L".png", L".tag", L".bmp", L".dib" , L".pba", L".dmb", L".db0", L".db1", L".db2", L".hba", L".hb0", L".hb1", L".hb2"    
        }};

        win32::hwnd_t self;
        win32::static_control static_image;
        bmp_controller controller;

        win32::auto_handle<HBITMAP, gdi_deleter> current_bitmap;

        bmp_view(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	    {
	    }

        auto on_create(const win32::create_message& info)
        {
            auto mfcModule = GetModuleHandleW(L"siege-win-mfc.dll");
            assert(mfcModule);

            auto parent_size = win32::GetClientSize(self);

            auto width = parent_size->cx;
            auto dialog_template = win32::MakeDialogTemplate(::DLGTEMPLATE{ .style = WS_VISIBLE | WS_CHILD , .x = short(width - width / 3), .cx = short(width / 3), .cy = short(parent_size->cy), });
            auto control_dialog = ::CreateDialogIndirectParamW(info.data.hInstance, &dialog_template.dialog, self, [](win32::hwnd_t, std::uint32_t, win32::wparam_t, win32::lparam_t) -> INT_PTR {
                    return FALSE;
                }, 0);

            auto group_box = win32::CreateWindowExW(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                            .cy = 100
						    }, control_dialog, win32::button::class_name, L"Colour strategy");

            assert(group_box);


            auto do_nothing = win32::CreateWindowExW<win32::button>(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON
						    }, control_dialog, win32::button::class_name, L"Do nothing");

            auto ideal_size = do_nothing->GetIdealSize();
            win32::SetWindowPos(*do_nothing, *ideal_size);

            auto remap = win32::CreateWindowExW<win32::button>(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
						    }, control_dialog, win32::button::class_name, L"Remap");

            ideal_size = remap->GetIdealSize();
            win32::SetWindowPos(*remap, *ideal_size);

            auto remap_unique = win32::CreateWindowExW<win32::button>(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
						    }, control_dialog, win32::button::class_name, L"Remap (only unique colours)");

            ideal_size = remap_unique->GetIdealSize();
            win32::SetWindowPos(*remap_unique, *ideal_size);


            std::wstring temp = L"menu.pal";
        

            win32::list_view palettes_list = [&] {
                auto palettes_list = *win32::CreateWindowExW<win32::list_view>(CREATESTRUCTW{
                            .hInstance = mfcModule,
                            .hwndParent = control_dialog,
						    .cy = 300,  
                            .cx = 400,
	            		    .y = 3,		
                            .style = WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                            .lpszName =  L"Palettes",
                            .lpszClass = L"MFC::CMFCListCtrl"
                });

                palettes_list.SetView(win32::list_view::view_type::details_view);
            assert(palettes_list.EnableGroupView(true));
            assert(palettes_list.SetTileViewInfo(LVTILEVIEWINFO {
                .dwFlags = LVTVIF_FIXEDWIDTH,
                .sizeTile = SIZE {.cx = win32::GetClientSize(control_dialog)->cx, .cy = 50},
                }));
        
            palettes_list.SetExtendedListViewStyle(0, 
                    LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

            palettes_list.win32::list_view::InsertColumn(-1, LVCOLUMNW {
                 .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L""),
                .cxMin = 100,
                .cxDefault = win32::GetClientSize(control_dialog)->cx,
                });

            assert(palettes_list.InsertGroup(-1, LVGROUP {
                .pszHeader = temp.data(),
                .iGroupId = 1
                }) == 0);

            palettes_list.InsertGroup(-1, LVGROUP {
                .pszHeader = const_cast<wchar_t*>(L"test.dpl"),
                .iGroupId = 2
                });

            palettes_list.InsertGroup(-1, LVGROUP {
                .pszHeader = const_cast<wchar_t*>(L"ui.pal"),
                .iGroupId = 3
                });

            assert(palettes_list.InsertItem(-1, LVITEMW{
                .pszText = temp.data(),
                .iGroupId = 1
                }) == 0);

            palettes_list.InsertItem(-1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 2"),
                .iGroupId = 1
                });

            palettes_list.InsertItem(-1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 3"),
                .iGroupId = 1
                });

            palettes_list.InsertItem(-1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 1"),
                .iGroupId = 2
                });

            palettes_list.InsertItem(-1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 2"),
                .iGroupId = 2
                });

            palettes_list.InsertItem(-1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 3"),
                .iGroupId = 2
                });


            palettes_list.InsertItem(-1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 1"),
                .iGroupId = 3
                });

            palettes_list.InsertItem(-1, LVITEMW{
                .pszText = const_cast<wchar_t*>(L"palette 2"),
                .iGroupId = 3
                });

                return palettes_list;
          }();

          //  // TODO add example palette file names as groups and then palette names as items

            static_image = *win32::CreateWindowExW<win32::static_control>(DLGITEMTEMPLATE{
						    .style = WS_VISIBLE | WS_CHILD | SS_BITMAP | SS_REALSIZECONTROL,
                            .x = 0,
                            .y = 0,
                            .cx = short((width / 3) * 2),
                            .cy = 300
						    }, self, win32::static_control::class_name, L"Image");
           
            auto root_children = std::array<win32::hwnd_t, 2>{static_image, control_dialog};

            win32::StackChildren(*win32::GetClientSize(self), root_children, win32::StackDirection::Horizontal);
           

            auto children = std::array<win32::hwnd_t, 2>{*group_box, palettes_list};
            win32::StackChildren(*win32::GetClientSize(control_dialog), children);
            
            auto rect = win32::GetClientRect(*group_box);
            rect->top += 15;
            rect->left += 5;

            auto radios = std::array<win32::hwnd_t, 3>{*do_nothing, *remap, *remap_unique};
            win32::StackChildren(SIZE{.cx = rect->right, .cy = rect->bottom - 20}, radios, win32::StackDirection::Horizontal,
              POINT{.x = rect->left, .y = rect->top});

            return 0;
        }

        std::optional<LRESULT> on_get_object(win32::get_object_message message)
        {
            if (message.object_id == OBJID_NATIVEOM)
            {
                auto collection = std::make_unique<win32::com::OwningCollection<std::unique_ptr<IStream, void(*)(IStream*)>>>();

                auto result = LresultFromObject(__uuidof(IDispatch), message.flags, static_cast<IDispatch*>(collection.get()));

                collection.release()->Release();

                return result;
            }

            return std::nullopt;
        }

        auto on_copy_data(win32::copy_data_message<char> message)
		{
			std::spanstream stream(message.data);
			
			if (bmp_controller::is_bmp(stream))
			{
                auto count = controller.load_bitmap(stream);

                if (count > 0)
                {
                    auto size = win32::GetClientSize(static_image);
                    BITMAPINFO info{
                                .bmiHeader {
                                    .biSize = sizeof(BITMAPINFOHEADER),
                                    .biWidth = LONG(size->cx),
                                    .biHeight = LONG(size->cy),
                                    .biPlanes = 1,
                                    .biBitCount = 32,
                                    .biCompression = BI_RGB
                                }
                            };
                   auto wnd_dc = ::GetDC(nullptr);

                    void* pixels = nullptr;
                    current_bitmap.reset(::CreateDIBSection(wnd_dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0));

                    controller.convert(0, std::make_pair(size->cx, size->cy), 32, std::span(reinterpret_cast<std::byte*>(pixels), size->cx * size->cy * 4));

                    static_image.SetImage(current_bitmap.get());

                    return TRUE;
                }

                return FALSE;
			}

			return FALSE;
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