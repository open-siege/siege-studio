#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

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
						.style = WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
						}, self, win32::button::class_name, L"Colour strategy");

        assert(group_box);

        auto do_nothing = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
						}, *group_box, win32::button::class_name, L"Do nothing");

        auto remap = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
						}, *group_box, win32::button::class_name, L"Remap");

        auto remap_unique = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
						}, *group_box, win32::button::class_name, L"Remap (only unique colours)");

        auto strategy_toolbar = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | TBSTYLE_LIST,   
						}, *rebar, win32::tool_bar::class_name, L"Colour strategy");

        assert(strategy_toolbar);

        win32::tool_bar::SetExtendedStyle(*strategy_toolbar, win32::tool_bar::mixed_buttons | win32::tool_bar::draw_drop_down_arrows);
     
        std::array<TBBUTTON, 2> buttons{{
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
						}, self, win32::tree_view::class_name, L"Palettes");

        // TODO add example palette file names as root items and then palette names as children

        auto palettes_list = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD,
						}, self, win32::list_view::class_name, L"Palettes");

        // TODO add example palette file names as groups and then palette names as items

        auto static_image = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD,
						}, self, win32::static_control::class_name, L"Image");

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