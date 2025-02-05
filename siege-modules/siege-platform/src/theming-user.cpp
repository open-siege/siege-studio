#include <set>
#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/direct_2d.hpp>
#include <siege/platform/stream.hpp>
#include <VersionHelpers.h>
#include <dwmapi.h>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);

  void apply_theme(win32::window_ref& control)
  {
    struct sub_class : win32::menu::notifications
    {
      std::map<std::wstring_view, COLORREF> colors;
      std::optional<win32::gdi::memory_drawing_context> memory_dc;
      SIZE main_menu_size{};

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      std::optional<win32::lresult_t> wm_enter_menu_loop(bool) override
      {
        return std::nullopt;
      }

      std::optional<win32::lresult_t> wm_exit_menu_loop(bool) override
      {
        return std::nullopt;
      }

      std::optional<win32::lresult_t> wm_init_menu_popup(win32::popup_menu_ref sub_menu, int, bool) override
      {
        MENUINFO mi = { .cbSize = sizeof(MENUINFO), .fMask = MIM_BACKGROUND };
        auto bk_color = colors[win32::properties::menu::bk_color];
        mi.hbrBack = win32::get_solid_brush(bk_color);
        SetMenuInfo(sub_menu, &mi);
        return 0;
      }

      std::optional<win32::lresult_t> wm_uninit_menu_popup(win32::popup_menu_ref sub_menu, int) override
      {
        return 0;
      }

      auto& get_memory_dc()
      {
        if (!memory_dc)
        {
          auto screen_dc = ::GetDC(nullptr);
          memory_dc = win32::gdi::memory_drawing_context(win32::gdi::drawing_context_ref(screen_dc));

          ::ReleaseDC(nullptr, screen_dc);
        }
        return *memory_dc;
      }

      SIZE wm_measure_item(win32::popup_menu_ref window_menu, const MEASUREITEMSTRUCT& item) override
      {
        auto font = win32::load_font(LOGFONTW{
          .lfPitchAndFamily = VARIABLE_PITCH,
          .lfFaceName = L"Segoe UI" });

        SelectFont(get_memory_dc(), font);

        std::wstring text;

        if (item.itemData && ((MSAAMENUINFO*)item.itemData)->dwMSAASignature == MSAA_MENU_SIG && ((MSAAMENUINFO*)item.itemData)->pszWText)
        {
          text = ((MSAAMENUINFO*)item.itemData)->pszWText;
        }
        else
        {
          text = L"__________";
        }

        SIZE char_size{};
        auto result = GetTextExtentPoint32W(get_memory_dc(), text.data(), text.size(), &char_size);

        char_size.cx += (GetSystemMetrics(SM_CXMENUCHECK) * 2);

        return char_size;
      }

      SIZE wm_measure_item(win32::menu_ref window_menu, const MEASUREITEMSTRUCT& item) override
      {
        auto font = win32::load_font(LOGFONTW{
          .lfPitchAndFamily = VARIABLE_PITCH,
          .lfFaceName = L"Segoe UI" });

        SelectFont(get_memory_dc(), font);

        std::wstring text;

        if (item.itemData && ((MSAAMENUINFO*)item.itemData)->dwMSAASignature == MSAA_MENU_SIG && ((MSAAMENUINFO*)item.itemData)->pszWText)
        {
          text = ((MSAAMENUINFO*)item.itemData)->pszWText;
        }
        else
        {
          text = L"__________";
        }

        SIZE char_size{};
        auto result = GetTextExtentPoint32W(get_memory_dc(), text.data(), text.size(), &char_size);
        char_size.cy = GetSystemMetrics(SM_CYMENU);

        struct callback
        {
          static BOOL __stdcall do_callback(HWND window, LPARAM info)
          {
            if (GetMenuBarInfo(window, OBJID_MENU, 0, (MENUBARINFO*)info))
            {
              return FALSE;
            }
            return TRUE;
          }
        };

        MENUBARINFO menu_bar_info{ .cbSize = sizeof(MENUBARINFO) };

        if (EnumThreadWindows(GetCurrentThreadId(), callback::do_callback, (LPARAM)&menu_bar_info) == FALSE && menu_bar_info.hMenu)
        {
          MENUITEMINFOW item_info{ .cbSize = sizeof(MENUITEMINFO), .fMask = MIIM_FTYPE };

          for (auto i = 0; i < GetMenuItemCount(menu_bar_info.hMenu); ++i)
          {
            if (GetMenuItemInfoW(menu_bar_info.hMenu, i, TRUE, &item_info))
            {
              if (item_info.fType & MFT_MENUBARBREAK)
              {
                char_size.cy += GetSystemMetrics(SM_CYMENU);
              }
            }
          }
        }

        main_menu_size = char_size;
        return char_size;
      }

      std::optional<win32::lresult_t> wm_draw_item(win32::popup_menu_ref menu, DRAWITEMSTRUCT& item) override
      {
        auto font = win32::load_font(LOGFONTW{
          .lfPitchAndFamily = VARIABLE_PITCH,
          .lfFaceName = L"Segoe UI" });

        SelectFont(item.hDC, font);

        auto bk_color = colors[win32::properties::menu::bk_color];
        auto text_highlight_color = colors[win32::properties::menu::text_highlight_color];
        auto black_brush = win32::get_solid_brush(bk_color);
        auto grey_brush = win32::get_solid_brush(colors[win32::properties::menu::text_highlight_color]);
        auto text_color = colors[win32::properties::menu::text_color];

        auto context = win32::gdi::drawing_context_ref(item.hDC);

        SelectObject(context, GetStockObject(DC_PEN));
        SelectObject(context, GetStockObject(DC_BRUSH));

        SetBkMode(context, TRANSPARENT);

        if (item.itemData && ((MSAAMENUINFO*)item.itemData)->dwMSAASignature == MSAA_MENU_SIG && ((MSAAMENUINFO*)item.itemData)->pszWText)
        {
          auto rect = item.rcItem;

          if (item.itemState & ODS_HOTLIGHT)
          {
            context.FillRect(rect, grey_brush);
          }
          else if (item.itemState & ODS_SELECTED)
          {
            context.FillRect(rect, grey_brush);
          }
          else
          {
            context.FillRect(rect, black_brush);
          }

          ::SetTextColor(context, text_color);

          rect.left += (rect.right - rect.left) / 10;

          auto& menu_item_info = *(MSAAMENUINFO*)item.itemData;
          ::DrawTextW(context, menu_item_info.pszWText, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
        }

        return TRUE;
      }

      std::optional<win32::lresult_t> wm_draw_item(win32::menu_ref menu, DRAWITEMSTRUCT& item) override
      {

        auto font = win32::load_font(LOGFONTW{
          .lfPitchAndFamily = VARIABLE_PITCH,
          .lfFaceName = L"Segoe UI" });

        SelectFont(item.hDC, font);

        auto bk_color = colors[win32::properties::menu::bk_color];
        auto text_highlight_color = colors[win32::properties::menu::text_highlight_color];
        auto black_brush = win32::get_solid_brush(bk_color);
        auto grey_brush = win32::get_solid_brush(colors[win32::properties::menu::text_highlight_color]);
        auto text_color = colors[win32::properties::menu::text_color];

        auto context = win32::gdi::drawing_context_ref(item.hDC);

        SelectObject(context, GetStockObject(DC_PEN));
        SelectObject(context, GetStockObject(DC_BRUSH));

        SetBkMode(context, TRANSPARENT);

        if (item.itemData && ((MSAAMENUINFO*)item.itemData)->dwMSAASignature == MSAA_MENU_SIG && ((MSAAMENUINFO*)item.itemData)->pszWText)
        {
          auto rect = item.rcItem;

          rect.bottom = rect.top + main_menu_size.cy - GetSystemMetrics(SM_CYEDGE);
          if (item.itemState & ODS_HOTLIGHT)
          {
            context.FillRect(rect, grey_brush);
          }
          else if (item.itemState & ODS_SELECTED)
          {
            context.FillRect(rect, grey_brush);
          }
          else
          {
            context.FillRect(rect, black_brush);
          }

          ::SetTextColor(context, text_color);

          rect.left += (rect.right - rect.left) / 10;

          auto& menu_item_info = *(MSAAMENUINFO*)item.itemData;
          ::DrawTextW(context, menu_item_info.pszWText, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
        }

        return TRUE;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto result = win32::menu::notifications::dispatch_message((sub_class*)dwRefData, hWnd, message, wParam, lParam);

        if (result)
        {
          return *result;
        }

        if (message == WM_NCHITTEST)
        {
          MENUBARINFO menu_bar_info{ .cbSize = sizeof(MENUBARINFO) };

          if (GetMenuBarInfo(hWnd, OBJID_MENU, 0, &menu_bar_info) && menu_bar_info.hMenu)
          {
            auto x = GET_X_LPARAM(lParam);
            auto y = GET_Y_LPARAM(lParam);

            if (::PtInRect(&menu_bar_info.rcBar, POINT{ .x = x, .y = y }))
            {
              auto menu_height = GetSystemMetrics(SM_CYMENU);

              auto item = MenuItemFromPoint(hWnd, menu_bar_info.hMenu, POINT{ .x = x, .y = menu_bar_info.rcBar.top });

              for (auto i = 0; i < GetMenuItemCount(menu_bar_info.hMenu); ++i)
              {
                HiliteMenuItem(hWnd, menu_bar_info.hMenu, i, MF_BYPOSITION | MF_UNHILITE);
              }

              if (item != -1)
              {
                HiliteMenuItem(hWnd, menu_bar_info.hMenu, item, MF_BYPOSITION | MF_HILITE);
              }

              return HTMENU;
            }
          }
        }

        if ((message == WM_NCLBUTTONDOWN || message == WM_NCLBUTTONUP) && wParam == HTMENU && lParam)
        {
          MENUBARINFO menu_bar_info{ .cbSize = sizeof(MENUBARINFO) };

          if (GetMenuBarInfo(hWnd, OBJID_MENU, 0, &menu_bar_info) && menu_bar_info.hMenu)
          {
            auto x = GET_X_LPARAM(lParam);
            auto y = GET_Y_LPARAM(lParam);

            if (::PtInRect(&menu_bar_info.rcBar, POINT{ .x = x, .y = y }))
            {
              auto menu_height = GetSystemMetrics(SM_CYMENU);

              auto item = MenuItemFromPoint(hWnd, menu_bar_info.hMenu, POINT{ .x = x, .y = y });

              if (item == -1 || item == GetMenuItemCount(menu_bar_info.hMenu) - 1)
              {
                item = MenuItemFromPoint(hWnd, menu_bar_info.hMenu, POINT{ .x = x, .y = menu_bar_info.rcBar.top });

                if (item != -1)
                {
                  auto mouse_flag = message == WM_NCLBUTTONDOWN ? (DWORD)MOUSEEVENTF_LEFTDOWN : (DWORD)MOUSEEVENTF_LEFTUP;

                  auto target_rect = menu_bar_info.rcBar;
                  target_rect.bottom = target_rect.top + (menu_bar_info.rcBar.bottom - menu_bar_info.rcBar.top) / 2;

                  auto new_y = target_rect.bottom - y - 1;

                  INPUT simulated_input{
                    .type = INPUT_MOUSE, .mi{ .dx = 0, .dy = new_y, .dwFlags = MOUSEEVENTF_MOVE | mouse_flag }
                  };

                  SendInput(1, &simulated_input, sizeof(INPUT));
                }

                return 0;
              }
            }
          }
        }

        if (message == WM_NCPAINT || message == WM_NCACTIVATE)
        {
          auto result = DefSubclassProc(hWnd, message, wParam, lParam);
          MENUBARINFO menu_bar_info{ .cbSize = sizeof(MENUBARINFO) };

          if (GetMenuBarInfo(hWnd, OBJID_MENU, 0, &menu_bar_info))
          {
            RECT window_rect = { 0 };
            GetWindowRect(hWnd, &window_rect);

            RECT line_rect = menu_bar_info.rcBar;
            line_rect.top = line_rect.bottom;
            line_rect.bottom = line_rect.top + GetSystemMetrics(SM_CYBORDER) + 1;
            OffsetRect(&line_rect, -window_rect.left, -window_rect.top);

            auto bk_color = ((sub_class*)dwRefData)->colors[win32::properties::window::bk_color];
            HDC hdc = GetWindowDC(hWnd);
            FillRect(hdc, &line_rect, win32::get_solid_brush(bk_color));
            ReleaseDC(hWnd, hdc);
          }

          return result;
        }

        if (message == WM_ERASEBKGND)
        {
          auto context = win32::gdi::drawing_context_ref((HDC)wParam);

          auto self = win32::window_ref(hWnd);
          auto bk_color = ((sub_class*)dwRefData)->colors[win32::properties::window::bk_color];

          auto rect = self.GetClientRect();
          context.FillRect(*rect, get_solid_brush(bk_color));
          return TRUE;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::window::bk_color, win32::get_color_for_window(control.ref(), win32::properties::window::bk_color) },
      { win32::properties::menu::bk_color, win32::get_color_for_window(control.ref(), win32::properties::menu::bk_color) },
      { win32::properties::menu::text_color, win32::get_color_for_window(control.ref(), win32::properties::menu::text_color) },
      { win32::properties::menu::text_highlight_color, win32::get_color_for_window(control.ref(), win32::properties::menu::text_highlight_color) },
    };

    DWORD_PTR existing_object{};

    constexpr static auto* subclass_id = L"Window";

    if (!::GetWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)subclass_id, &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)subclass_id, (DWORD_PTR) new sub_class(std::move(color_map)));
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }

    if (control.GetWindowStyle() & ~WS_CHILD)
    {
      BOOL value = win32::is_dark_theme() ? TRUE : FALSE;
      if (::DwmSetWindowAttribute(control, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value)) == S_OK)
      {
        ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
      }

      auto menu = GetMenu(control);

      if (menu)
      {
        MENUINFO mi = {};
        mi.cbSize = sizeof(mi);
        mi.fMask = MIM_BACKGROUND;
        auto bk_color = win32::get_color_for_window(control.ref(), win32::properties::menu::bk_color);
        mi.hbrBack = win32::get_solid_brush(bk_color);

        SetMenuInfo(menu, &mi);
        DrawMenuBar(control);
      }
    }
  }

  void apply_theme(win32::edit& control)
  {
    struct sub_class
    {
      std::optional<HBRUSH> wm_control_color(win32::edit control, win32::gdi::drawing_context_ref dc)
      {
        return std::nullopt;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    DWORD_PTR existing_object{};

    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR) new sub_class());
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
  }

  void apply_theme(win32::button& control)
  {
    struct sub_class
    {
      std::function<void()> bind_remover;

      win32::gdi::font_ref font = win32::load_font(LOGFONTW{
        .lfPitchAndFamily = VARIABLE_PITCH,
        .lfFaceName = L"Segoe UI" });


      std::wstring test = std::wstring(255, '\0');

      std::map<std::wstring_view, COLORREF> colors;

      bool use_direct_2d = false;
      
      sub_class(win32::button& button, std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
        bind_remover = button.bind_custom_draw({ .nm_custom_draw = std::bind_front(&sub_class::nm_custom_draw, this),
          .wm_control_color = std::bind_front(&sub_class::wm_control_color, this) });
      }

      ~sub_class()
      {
        bind_remover();
      }

      win32::lresult_t nm_custom_draw(win32::button button, NMCUSTOMDRAW& custom_draw)
      {
        if (custom_draw.dwDrawStage == CDDS_PREPAINT)
        {
          auto rect = custom_draw.rc;
          auto new_size = SIZE(rect.right - rect.left, rect.bottom - rect.top);

          auto text_color = colors[properties::button::text_color];
          auto bk_color = colors[properties::button::bk_color];
          auto hot_color = colors[properties::button::hot_bk_color];
          auto focus_color = colors[properties::button::focus_bk_color];
          auto pushed_color = colors[properties::button::pushed_bk_color];
          auto state = Button_GetState(button);

          SelectFont(custom_draw.hdc, font);
          if (state & BST_HOT)
          {
            ::SetTextColor(custom_draw.hdc, text_color);
            ::SetBkColor(custom_draw.hdc, hot_color);
            ::FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(hot_color));
          }
          else if (state & BST_FOCUS)
          {
            ::SetTextColor(custom_draw.hdc, text_color);
            ::SetBkColor(custom_draw.hdc, focus_color);
            ::FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(focus_color));
          }
          else if (state & BST_PUSHED)
          {
            ::SetTextColor(custom_draw.hdc, text_color);
            ::SetBkColor(custom_draw.hdc, pushed_color);
            ::FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(pushed_color));
          }
          else
          {
            ::SetTextColor(custom_draw.hdc, text_color);
            ::SetBkColor(custom_draw.hdc, bk_color);
            ::FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(bk_color));
          }

          return CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
        }

        if (custom_draw.dwDrawStage == CDDS_POSTPAINT)
        {
          Button_GetText(button, test.data(), test.size());

          ::DrawTextExW(custom_draw.hdc, test.data(), -1, &custom_draw.rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP, nullptr);
        }

        return CDRF_DODEFAULT;
      }

      HBRUSH wm_control_color(win32::button button, win32::gdi::drawing_context_ref context)
      {
        auto text_color = colors[properties::button::text_color];

        if (text_color)
        {
          ::SetTextColor(context, text_color);
        }

        auto bk_color = colors[properties::button::bk_color];

        if (bk_color)
        {
          ::SetBkColor(context, bk_color);
          return get_solid_brush(bk_color);
        }

        return (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
          delete (sub_class*)dwRefData;
          return ::DefSubclassProc(hWnd, message, wParam, lParam);
        }

        return ::DefSubclassProc(hWnd, message, wParam, lParam);
      }

      static LRESULT __stdcall HandleChildMessage(HWND button, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto* self = (sub_class*)dwRefData;


        if (message == WM_SIZE)
        {
          UINT width = LOWORD(lParam);
          UINT height = HIWORD(lParam);
          self->use_direct_2d = width > height;
          
        }

        if ((message == WM_PAINT || message == WM_PRINTCLIENT) && self->use_direct_2d)
        {
          PAINTSTRUCT info{};

          if (wParam)
          {
            info.hdc = (HDC)wParam;
            GetClientRect(button, &info.rcPaint);
          }
          else if (message == WM_PAINT)
          {
            BeginPaint(button, &info);
          }

          LRESULT result = 0;
          if (info.hdc)
          {
            auto& render_target = win32::direct2d::dc_render_target::for_thread(D2D1_RENDER_TARGET_PROPERTIES{
              .type = D2D1_RENDER_TARGET_TYPE_SOFTWARE,
              .pixelFormat = D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_IGNORE),
              .dpiX = 0,
              .dpiY = 0,
              .usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
              .minLevel = D2D1_FEATURE_LEVEL_DEFAULT });
            ;
            render_target.bind_dc(win32::gdi::drawing_context_ref(info.hdc), info.rcPaint);

            render_target.begin_draw();

            auto layer = render_target.create_layer();
            auto rounded_rectangle = win32::direct2d::rounded_rectangle_geometry(D2D1_ROUNDED_RECT{
              .rect = {
                .right = (float)info.rcPaint.right,
                .bottom = (float)info.rcPaint.bottom,
              },
              .radiusX = 50,
              .radiusY = 50,
            });

            auto interop_target = render_target.get_interop_render_target();

            auto target_hdc = interop_target.get_dc(D2D1_DC_INITIALIZE_MODE_CLEAR);

            auto parent = ::GetParent(button);

            SendMessageW(parent, WM_ERASEBKGND, (WPARAM)target_hdc.get(), 0);

            interop_target.release_dc();

            render_target.push_layer(D2D1::LayerParameters(D2D1::InfiniteRect(), &rounded_rectangle.object()), layer);
            target_hdc = interop_target.get_dc(D2D1_DC_INITIALIZE_MODE_COPY);

            result = ::DefSubclassProc(button, message, (WPARAM)target_hdc.get(), lParam);

            interop_target.release_dc();
            render_target.pop_layer();
            render_target.end_draw();
          }

          if (message == WM_PAINT)
          {
            EndPaint(button, &info);
          }
          return result;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(button, sub_class::HandleChildMessage, uIdSubclass);
          return ::DefSubclassProc(button, message, wParam, lParam);
        }

        return ::DefSubclassProc(button, message, wParam, lParam);
      }
    };

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::button::bk_color, win32::get_color_for_window(control.ref(), win32::properties::button::bk_color) },
      { win32::properties::button::focus_bk_color, win32::get_color_for_window(control.ref(), win32::properties::button::focus_bk_color) },
      { win32::properties::button::hot_bk_color, win32::get_color_for_window(control.ref(), win32::properties::button::hot_bk_color) },
      { win32::properties::button::pushed_bk_color, win32::get_color_for_window(control.ref(), win32::properties::button::pushed_bk_color) },
      { win32::properties::button::text_color, win32::get_color_for_window(control.ref(), win32::properties::button::text_color) },
      { win32::properties::button::line_color, win32::get_color_for_window(control.ref(), win32::properties::button::line_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      auto object = (DWORD_PTR) new sub_class(control, std::move(color_map));
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), object);
      ::SetWindowSubclass(control, sub_class::HandleChildMessage, (UINT_PTR)control.get(), object);
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else if (existing_object)
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }

  void apply_theme(win32::static_control& control)
  {
    struct sub_class
    {
      std::map<std::wstring_view, COLORREF> colors;
      std::function<void()> bind_remover;
      SIZE previous_size{};

      sub_class(win32::static_control& control, std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
        bind_remover = control.bind_custom_draw({ .wm_control_color = std::bind_front(&sub_class::wm_control_color, this) });
      }

      ~sub_class()
      {
        bind_remover();
      }

      HBRUSH wm_control_color(win32::static_control static_control, win32::gdi::drawing_context_ref context)
      {
        auto current_size = static_control.GetClientSize().value_or(SIZE{});
        HBRUSH result = nullptr;

        if (std::memcmp(&current_size, &previous_size, sizeof(current_size)) == 0)
        {
          result = (HBRUSH)GetStockObject(NULL_BRUSH);
        }
        else
        {
          auto text_color = colors[properties::static_control::text_color];

          ::SetTextColor(context, text_color);

          auto bk_color = colors[properties::static_control::bk_color];
          ::SetBkColor(context, bk_color);
          result = get_solid_brush(bk_color).get();
        }
        previous_size = current_size;

        return result;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    HFONT font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font, FALSE);

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::static_control::bk_color, win32::get_color_for_window(control.ref(), win32::properties::static_control::bk_color) },
      { win32::properties::static_control::text_color, win32::get_color_for_window(control.ref(), win32::properties::static_control::text_color) }
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR) new sub_class(control, std::move(color_map)));

      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else if (existing_object)
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }


  void apply_theme(win32::list_box& control)
  {
    struct sub_class
    {
      std::map<std::wstring_view, COLORREF> colors;
      std::function<void()> bind_remover;
      
      sub_class(win32::list_box& control, std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
        bind_remover = control.bind_custom_draw({ .wm_control_color = std::bind_front(&sub_class::wm_control_color, this),
          .wm_measure_item = std::bind_front(&sub_class::wm_measure_item, this),
          .wm_draw_item = std::bind_front(&sub_class::wm_draw_item, this) });
      }

      ~sub_class()
      {
        bind_remover();
      }

      HBRUSH wm_control_color(win32::list_box list_box, win32::gdi::drawing_context_ref context)
      {
        auto bk_color = colors[properties::list_box::bk_color];
        return get_solid_brush(bk_color).get();
      }

      SIZE wm_measure_item(win32::list_box themed_selection, const MEASUREITEMSTRUCT& item)
      {
        return SIZE{ .cy = (LONG)themed_selection.GetItemHeight(item.itemID) };
      }

      win32::lresult_t wm_draw_item(win32::list_box list, DRAWITEMSTRUCT& item)
      {
        ;
        thread_local std::wstring buffer;

        auto& render_target = win32::direct2d::dc_render_target::for_thread(D2D1_RENDER_TARGET_PROPERTIES{
          .type = D2D1_RENDER_TARGET_TYPE_SOFTWARE,
          .pixelFormat = D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_IGNORE),
          .dpiX = 0,
          .dpiY = 0,
          .usage = D2D1_RENDER_TARGET_USAGE_NONE,
          .minLevel = D2D1_FEATURE_LEVEL_DEFAULT });
 
        render_target.bind_dc(win32::gdi::drawing_context_ref(item.hDC), item.rcItem);

        render_target.begin_draw();

        auto bounds = D2D1::RectF(0, 0, (float)item.rcItem.right - item.rcItem.left, (float)item.rcItem.bottom - item.rcItem.top);

        render_target.object().Clear(D2D1::ColorF(colors[properties::list_box::bk_color], 1.0));

        if (item.itemState & ODS_SELECTED)
        {
          // TODO cache brushes
          auto selected_brush = render_target.create_solid_color_brush(D2D1::ColorF(colors[properties::list_box::text_highlight_color], 1.0));
          render_target.fill_rectangle(bounds, selected_brush);
        }
        else
        {
          auto normal_brush = render_target.create_solid_color_brush(D2D1::ColorF(colors[properties::list_box::text_bk_color], 1.0));
          render_target.fill_rectangle(bounds, normal_brush);
        }


        buffer.resize(list.GetTextLength(item.itemID), '\0');

        list.GetText(item.itemID, buffer.data());

        // TODO cache format
        win32::directwrite::text_format format({ .family_name = L"Segoe UI", .size = 18.0f });

        format.object().SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        auto text_brush = render_target.create_solid_color_brush(D2D1::ColorF(colors[properties::list_box::text_color], 1.0));

        render_target.draw_text(buffer, format, bounds, text_brush);

        render_target.end_draw();

        return TRUE;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_NCDESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
          delete (sub_class*)dwRefData;
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    gdi::font_ref font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    auto size = get_font_size_for_string(std::move(font), L"A");

    if (size)
    {
      ListBox_SetItemHeight(control, 0, size->cy * 2);
    }

    auto style = control.GetWindowStyle();

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::list_box::bk_color, win32::get_color_for_window(control.ref(), win32::properties::list_box::bk_color) },
      { win32::properties::list_box::text_color, win32::get_color_for_window(control.ref(), win32::properties::list_box::text_color) },
      { win32::properties::list_box::text_bk_color, win32::get_color_for_window(control.ref(), win32::properties::list_box::text_bk_color) },
      { win32::properties::list_box::text_highlight_color, win32::get_color_for_window(control.ref(), win32::properties::list_box::text_highlight_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR) new sub_class(control, std::move(color_map)));
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else if (existing_object)
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }
}// namespace win32