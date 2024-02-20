#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

#include <win32_controls.hpp>
#include <cassert>

struct bitmap_window
{
    constexpr static std::u8string_view formats = u8".jpg .jpeg .gif .png .tag .bmp .dib .pba .dmb .db0 .db1 .db2 .hba .hb0 .hb1 .hb2";

    win32::hwnd_t self;

    bitmap_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

    auto on_create(const win32::create_message&)
    {
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

        auto strategy_toolbar = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | CCS_NOPARENTALIGN | TBSTYLE_LIST,   
                        .cy = 400,
						}, self, win32::tool_bar::class_name, L"Colour strategy");

        assert(strategy_toolbar);

        win32::tool_bar::SetExtendedStyle(*strategy_toolbar, win32::tool_bar::mixed_buttons | win32::tool_bar::draw_drop_down_arrows);
     
        std::array<TBBUTTON, 3> buttons{{
            TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 0, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Do nothing")},
            TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 1, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Remap")},
            TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 2, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Remap (only unique colours)")},
         }};

        assert(win32::tool_bar::AddButtons(*strategy_toolbar, buttons));

        auto palettes_tree = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD,
                        .y = 2,
                        .cy = 300,
						}, self, win32::tree_view::class_name, L"Palettes");

        // TODO add example palette file names as root items and then palette names as children

        auto palettes_list = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD,
                        .y = 3,
                        .cy = 300,
						}, self, win32::list_view::class_name, L"Palettes");

      //  // TODO add example palette file names as groups and then palette names as items

      //  auto static_image = win32::CreateWindowExW(DLGITEMTEMPLATE{
						//.style = WS_VISIBLE | WS_CHILD,
      //                  .y = 4,
      //                  .cy = 300,
						//}, self, win32::static_control::class_name, L"Image");

        auto children = std::array{*group_box, *strategy_toolbar};
        win32::StackChildren(*win32::GetClientSize(self), children);

        auto rect = win32::GetClientRect(*group_box);
        rect->top += 15;
        rect->left += 5;

        auto radios = std::array{*do_nothing, *remap, *remap_unique};
        win32::StackChildren(SIZE{.cx = rect->right, .cy = rect->bottom - 20}, radios, win32::StackDirection::Horizontal,
                POINT{.x = rect->left, .y = rect->top});

        return 0;
    }

    auto on_size(win32::size_message sized)
	{
	//	win32::ForEachDirectChildWindow(self, [&](auto child) {
	//		win32::SetWindowPos(child, sized.client_size);
		//});
		return std::nullopt;
	}

    static bool is_bitmap(std::istream& raw_data)
    {
        return false;
    }
};

#endif