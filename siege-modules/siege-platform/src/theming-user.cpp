#include <set>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/direct_2d.hpp>
#include <siege/platform/stream.hpp>
#include <VersionHelpers.h>
#include <dwmapi.h>
#include <unordered_set>

namespace win32
{
  bool is_parent_from_system(HWND parent);
  gdi::brush_ref get_solid_brush(COLORREF color);

  // TODO put the shared code into a private header
  struct superclass_handle
  {
    HWND dummy = nullptr;
    std::add_pointer_t<decltype(DefWindowProcW)> control_proc = DefWindowProcW;

    void init(const wchar_t* class_name, LONG_PTR sub_class)
    {
      dummy = ::CreateWindowExW(0, class_name, L"Dummy", 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
      control_proc = (decltype(control_proc))::SetClassLongPtrW(dummy, GCLP_WNDPROC, sub_class);
    }

    void dispose(std::unordered_set<HWND>& controls, LONG_PTR sub_class)
    {
      if (!dummy)
      {
        return;
      }

      for (auto control : controls)
      {
        auto existing_proc = ::GetWindowLongPtrW(control, GWLP_WNDPROC);

        if (existing_proc == sub_class)
        {
          ::SetWindowLongW(control, GWLP_WNDPROC, (LONG_PTR)control_proc);
        }
      }

      ::SetClassLongPtrW(dummy, GCLP_WNDPROC, (LONG_PTR)control_proc);
      ::DestroyWindow(dummy);
      dummy = nullptr;
    }

    ~superclass_handle()
    {
      if (dummy)
      {
        ::SetClassLongPtrW(dummy, GCLP_WNDPROC, (LONG_PTR)control_proc);
        ::DestroyWindow(dummy);
        dummy = nullptr;
      }
    }
  };

  struct root_window_handler
  {
    static LRESULT __stdcall handle_root_message(HWND root, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
      if (dwRefData && message == WM_SETTINGCHANGE && lParam && std::wstring_view((wchar_t*)lParam) == L"ImmersiveColorSet")
      {
        auto result = DefSubclassProc(root, message, wParam, lParam);
        auto& controls = *(std::unordered_set<HWND>*)dwRefData;

        for (auto& control : controls)
        {
          SendMessage(control, WM_SETTINGCHANGE, lParam, wParam);
          SendMessage(control, WM_THEMECHANGED, 0, 0);
        }

        return result;
      }

      if (message == WM_NCDESTROY)
      {
        ::RemoveWindowSubclass(root, handle_root_message, 0);
      }

      return DefSubclassProc(root, message, wParam, lParam);
    }
  };

  void apply_window_theme(win32::window_ref& control)
  {
    struct sub_class : win32::menu::notifications
    {
      std::optional<win32::gdi::memory_drawing_context> memory_dc;
      SIZE main_menu_size{};

      sub_class()
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
        auto bk_color = win32::get_color_for_class(L"#32768", win32::properties::menu::bk_color);

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
          MENUITEMINFOW item_info{ .cbSize = sizeof(MENUITEMINFO), .fMask = MIIM_FTYPE | MIIM_BITMAP };

          for (auto i = 0; i < GetMenuItemCount(menu_bar_info.hMenu); ++i)
          {
            if (GetMenuItemInfoW(menu_bar_info.hMenu, i, TRUE, &item_info))
            {
              if (item_info.fType & MFT_BITMAP && item_info.hbmpItem)
              {
                ::BITMAP bitmap_info{};
                if (::GetObject(item_info.hbmpItem, sizeof(bitmap_info), &bitmap_info) > 0)
                {
                  char_size.cy = bitmap_info.bmHeight;
                  break;
                }
              }

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

        auto bk_color = win32::get_color_for_class(L"#32768", win32::properties::menu::bk_color);
        auto text_highlight_color = win32::get_color_for_class(L"#32768", win32::properties::menu::text_highlight_color);
        auto black_brush = win32::get_solid_brush(bk_color);
        auto grey_brush = win32::get_solid_brush(text_highlight_color);
        auto text_color = win32::get_color_for_class(L"#32768", win32::properties::menu::text_color);

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

        auto bk_color = win32::get_color_for_class(L"#32768", win32::properties::menu::bk_color);
        auto text_highlight_color = win32::get_color_for_class(L"#32768", win32::properties::menu::text_highlight_color);
        auto black_brush = win32::get_solid_brush(bk_color);
        auto grey_brush = win32::get_solid_brush(win32::get_color_for_class(L"#32768", win32::properties::menu::text_highlight_color));
        auto text_color = win32::get_color_for_class(L"#32768", win32::properties::menu::text_color);

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

            auto bk_color = win32::get_color_for_class(L"", win32::properties::window::bk_color);
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
          auto bk_color = win32::get_color_for_class(L"", win32::properties::window::bk_color);

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

    DWORD_PTR existing_object{};

    constexpr static auto* subclass_id = L"Window";

    if (!::GetWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)subclass_id, &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)subclass_id, (DWORD_PTR) new sub_class());
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

  void apply_button_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;

    struct handlers
    {

      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          if (create_params->style & BS_OWNERDRAW)
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};
          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          if (!::GetWindowSubclass(parent, handle_parent_message, 0, &existing_object))
          {
            ::SetWindowSubclass(parent, handle_parent_message, 0, 0);
          }
          controls.emplace(self);
          return superclass.control_proc(self, message, wParam, lParam);
        }

        if (message == WM_CREATE && controls.contains(self))
        {
          auto result = superclass.control_proc(self, message, wParam, lParam);

          auto font = win32::load_font(LOGFONTW{
            .lfPitchAndFamily = VARIABLE_PITCH,
            .lfFaceName = L"Segoe UI" });

          ::SendMessageW(self, CCM_DPISCALE, TRUE, 0);
          SendMessageW(self, WM_SETFONT, (WPARAM)font.get(), FALSE);
          SendMessageW(self, WM_SETTINGCHANGE, 0, 0);
          return result;
        }

        if (message == WM_SIZE)
        {
          auto shape = (int)::GetPropW(self, L"Shape");

          ::SetPropW(self, L"UseDirect2d", (HANDLE)(shape == EMR_ROUNDRECT ? TRUE : FALSE));
        }

        if ((message == WM_PAINT || message == WM_PRINTCLIENT) && ::GetPropW(self, L"UseDirect2d") && controls.contains(self))
        {
          PAINTSTRUCT info{};

          if (wParam)
          {
            info.hdc = (HDC)wParam;
            GetClientRect(self, &info.rcPaint);
          }
          else if (message == WM_PAINT)
          {
            BeginPaint(self, &info);
          }

          LRESULT result = 0;
          if (info.hdc)
          {
            auto& render_target = win32::direct2d::dc_render_target::for_thread(D2D1_RENDER_TARGET_PROPERTIES{
              .type = D2D1_RENDER_TARGET_TYPE_SOFTWARE,
              .pixelFormat = D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_IGNORE),
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

            auto parent = ::GetParent(self);

            SendMessageW(parent, WM_ERASEBKGND, (WPARAM)target_hdc.get(), 0);

            interop_target.release_dc();

            render_target.push_layer(D2D1::LayerParameters(D2D1::InfiniteRect(), &rounded_rectangle.object()), layer);
            target_hdc = interop_target.get_dc(D2D1_DC_INITIALIZE_MODE_COPY);

            result = superclass.control_proc(self, message, (WPARAM)target_hdc.get(), lParam);

            interop_target.release_dc();
            render_target.pop_layer();
            render_target.end_draw();
          }

          if (message == WM_PAINT && !wParam)
          {
            EndPaint(self, &info);
          }
          return result;
        }


        if (message == WM_NCDESTROY && controls.contains(self))
        {
          ::RemovePropW(self, L"UseDirect2d");
          ::RemovePropW(self, L"Shape");
          controls.erase(self);
        }

        return superclass.control_proc(self, message, wParam, lParam);
      }

      static LRESULT __stdcall handle_parent_message(HWND parent, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_CTLCOLORBTN && controls.contains((HWND)lParam))
        {
          auto text_color = win32::get_color_for_window(win32::window_ref((HWND)lParam), properties::button::text_color);

          if (text_color)
          {
            ::SetTextColor((HDC)wParam, text_color);
          }

          auto bk_color = win32::get_color_for_window(win32::window_ref((HWND)lParam), properties::button::bk_color);

          if (bk_color)
          {
            ::SetBkColor((HDC)wParam, bk_color);
            ::SetDCBrushColor((HDC)wParam, bk_color);
            return (LRESULT)::GetStockObject(DC_BRUSH);
          }

          return (LRESULT)::GetStockObject(HOLLOW_BRUSH);
        }

        if (message == WM_NOTIFY)
        {
          NMHDR* header = (NMHDR*)lParam;

          if (header->code == NM_CUSTOMDRAW && controls.contains(header->hwndFrom))
          {
            auto& custom_draw = *(NMCUSTOMDRAW*)header;
            if (custom_draw.dwDrawStage == CDDS_PREPAINT)
            {
              auto button = win32::window_ref(header->hwndFrom);
              auto rect = custom_draw.rc;
              auto new_size = SIZE(rect.right - rect.left, rect.bottom - rect.top);

              auto text_color = win32::get_color_for_window(button.ref(), properties::button::text_color);
              auto bk_color = win32::get_color_for_window(button.ref(), properties::button::bk_color);
              auto hot_color = win32::get_color_for_window(button.ref(), properties::button::hot_bk_color);
              auto focus_color = win32::get_color_for_window(button.ref(), properties::button::focus_bk_color);
              auto pushed_color = win32::get_color_for_window(button.ref(), properties::button::pushed_bk_color);
              auto state = Button_GetState(button);

              auto font = (HFONT)::SendMessageW(button, WM_GETFONT, 0, 0);
              SelectFont(custom_draw.hdc, font);
              if (state & BST_HOT)
              {
                ::SetTextColor(custom_draw.hdc, text_color);
                ::SetBkColor(custom_draw.hdc, hot_color);
                ::SetDCBrushColor(custom_draw.hdc, hot_color);
              }
              else if (state & BST_FOCUS)
              {
                ::SetTextColor(custom_draw.hdc, text_color);
                ::SetBkColor(custom_draw.hdc, focus_color);
                ::SetDCBrushColor(custom_draw.hdc, focus_color);
              }
              else if (state & BST_PUSHED)
              {
                ::SetTextColor(custom_draw.hdc, text_color);
                ::SetBkColor(custom_draw.hdc, pushed_color);
                ::SetDCBrushColor(custom_draw.hdc, pushed_color);
              }
              else
              {
                ::SetTextColor(custom_draw.hdc, text_color);
                ::SetBkColor(custom_draw.hdc, bk_color);
                ::SetDCBrushColor(custom_draw.hdc, bk_color);
              }
              ::FillRect(custom_draw.hdc, &custom_draw.rc, (HBRUSH)::GetStockObject(DC_BRUSH));

              return CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
            }

            if (custom_draw.dwDrawStage == CDDS_POSTPAINT)
            {
              static std::wstring temp(255, L'\0');
              Button_GetText(header->hwndFrom, temp.data(), temp.size());

              ::DrawTextExW(custom_draw.hdc, temp.data(), -1, &custom_draw.rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP, nullptr);
            }

            return CDRF_DODEFAULT;
          }
        }

        if (message == WM_NCDESTROY)
        {
          ::RemoveWindowSubclass(parent, handle_parent_message, 0);
        }

        return DefSubclassProc(parent, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(WC_BUTTONW, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }

  void apply_static_control_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;

    struct handlers
    {
      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          if (create_params->style == (WS_CHILD | WS_VISIBLE | SS_OWNERDRAW))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          if (create_params->style == (WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_OWNERDRAW))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          if (create_params->style == (WS_CHILD | WS_VISIBLE | SS_ICON | SS_OWNERDRAW))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};

          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          if (!::GetWindowSubclass(parent, handle_parent_message, 0, &existing_object))
          {
            ::SetWindowSubclass(parent, handle_parent_message, 0, 0);
          }

          controls.emplace(self);

          return superclass.control_proc(self, message, wParam, lParam);
        }

        static std::map<HBITMAP, HBITMAP> bitmap_cache;

        if (message == WM_CREATE && controls.contains(self))
        {
          auto result = superclass.control_proc(self, message, wParam, lParam);

          auto child = ::FindWindowExW(self, nullptr, nullptr, nullptr);
          HFONT font = win32::load_font(LOGFONTW{
            .lfPitchAndFamily = VARIABLE_PITCH,
            .lfFaceName = L"Segoe UI" });

          SendMessageW(self, WM_SETFONT, (WPARAM)font, FALSE);
          return result;
        }

        if (message == WM_NCDESTROY && controls.contains(self))
        {
          RemovePropW(self, L"PreviousWidth");
          RemovePropW(self, L"PreviousHeight");
          controls.erase(self);
        }


        return superclass.control_proc(self, message, wParam, lParam);
      }

      static LRESULT __stdcall handle_parent_message(HWND parent, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_CTLCOLORSTATIC && controls.contains((HWND)lParam))
        {
          win32::static_control static_control((HWND)lParam);
          SIZE previous_size{
            .cx = (LONG)GetPropW(static_control, L"PreviousWidth"),
            .cy = (LONG)GetPropW(static_control, L"PreviousHeight"),
          };

          win32::gdi::drawing_context_ref context((HDC)wParam);


          auto current_size = static_control.GetClientSize().value_or(SIZE{});
          HBRUSH result = nullptr;

          if (std::memcmp(&current_size, &previous_size, sizeof(current_size)) == 0)
          {
            result = (HBRUSH)GetStockObject(NULL_BRUSH);
          }
          else
          {
            auto text_color = win32::get_color_for_window(static_control.ref(), properties::static_control::text_color);

            ::SetTextColor(context, text_color);

            auto bk_color = win32::get_color_for_window(static_control.ref(), properties::static_control::bk_color);
            ::SetBkColor(context, bk_color);
            ::SetDCBrushColor(context, bk_color);
            result = (HBRUSH)::GetStockObject(DC_BRUSH);
          }
          SetPropW(static_control, L"PreviousWidth", (HANDLE)current_size.cx);
          SetPropW(static_control, L"PreviousHeight", (HANDLE)current_size.cy);

          return (LRESULT)result;
        }

        if (message == WM_NCDESTROY)
        {
          ::RemoveWindowSubclass(parent, handle_parent_message, 0);
        }

        return DefSubclassProc(parent, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(WC_STATICW, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }


  void apply_list_box_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;
    static std::unordered_map<HMENU, HWND> controls_by_id;

    struct handlers
    {
      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          if (create_params->style & LBS_OWNERDRAWFIXED)
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          if (create_params->style & LBS_OWNERDRAWVARIABLE)
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          create_params->style = create_params->style | LBS_OWNERDRAWFIXED;

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};

          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          if (!::GetWindowSubclass(parent, handle_parent_message, 0, &existing_object))
          {
            ::SetWindowSubclass(parent, handle_parent_message, 0, 0);
          }
          controls.emplace(self);

          auto menu_id = create_params->hMenu;
          auto result = superclass.control_proc(self, message, wParam, lParam);

          if (!menu_id)
          {
            SetWindowLongPtrW(self, GWLP_ID, (LONG_PTR)self);
            menu_id = (HMENU)self;
          }

          controls_by_id[menu_id] = self;
          return result;
        }

        if (message == WM_CREATE && controls.contains(self))
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;
          create_params->style = create_params->style | LBS_OWNERDRAWFIXED;
          auto result = superclass.control_proc(self, message, wParam, lParam);

          gdi::font_ref font = win32::load_font(LOGFONTW{
            .lfPitchAndFamily = VARIABLE_PITCH,
            .lfFaceName = L"Segoe UI" });

          SendMessageW(self, WM_SETFONT, (WPARAM)font.get(), FALSE);

          auto size = get_font_size_for_string(std::move(font), L"A");

          if (size)
          {
            ListBox_SetItemHeight(self, 0, size->cy * 2);
          }

          return result;
        }

        return superclass.control_proc(self, message, wParam, lParam);
      }

      static LRESULT __stdcall handle_parent_message(HWND parent, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_CTLCOLORLISTBOX && controls.contains((HWND)lParam))
        {
          auto bk_color = win32::get_color_for_window(win32::window_ref((HWND)lParam), properties::list_box::bk_color);

          SetDCBrushColor((HDC)wParam, bk_color);
          return (LRESULT)::GetStockObject(DC_BRUSH);
        }

        if (message == WM_MEASUREITEM)
        {
          auto& context = *(MEASUREITEMSTRUCT*)lParam;

          if (context.CtlType == ODT_LISTBOX && controls_by_id.contains((HMENU)context.CtlID))
          {
            context.itemHeight = ListBox_GetItemHeight(controls_by_id[(HMENU)context.itemID], context.itemID);
          }
        }

        if (message == WM_DRAWITEM && controls.contains(((DRAWITEMSTRUCT*)lParam)->hwndItem))
        {
          auto& context = *(DRAWITEMSTRUCT*)lParam;

          thread_local std::wstring buffer;

          auto& render_target = win32::direct2d::dc_render_target::for_thread(D2D1_RENDER_TARGET_PROPERTIES{
            .type = D2D1_RENDER_TARGET_TYPE_SOFTWARE,
            .pixelFormat = D2D1::PixelFormat(
              DXGI_FORMAT_B8G8R8A8_UNORM,
              D2D1_ALPHA_MODE_IGNORE),
            .usage = D2D1_RENDER_TARGET_USAGE_NONE,
            .minLevel = D2D1_FEATURE_LEVEL_DEFAULT });

          render_target.bind_dc(win32::gdi::drawing_context_ref(context.hDC), context.rcItem);

          render_target.begin_draw();

          auto bounds = D2D1::RectF(0, 0, (float)context.rcItem.right - context.rcItem.left, (float)context.rcItem.bottom - context.rcItem.top);

          auto list = win32::list_box(context.hwndItem);
          auto bk_color = win32::get_color_for_window(list.ref(), win32::properties::list_box::bk_color);
          auto text_highlight_color = win32::get_color_for_window(list.ref(), win32::properties::list_box::text_highlight_color);
          auto text_bk_color = win32::get_color_for_window(list.ref(), win32::properties::list_box::text_bk_color);
          auto text_color = win32::get_color_for_window(list.ref(), win32::properties::list_box::text_color);

          render_target.object().Clear(D2D1::ColorF(bk_color, 1.0));

          if (context.itemState & ODS_SELECTED)
          {
            auto selected_brush = render_target.create_solid_color_brush(D2D1::ColorF(text_highlight_color, 1.0));
            render_target.fill_rectangle(bounds, selected_brush);
          }
          else
          {
            auto normal_brush = render_target.create_solid_color_brush(D2D1::ColorF(text_bk_color, 1.0));
            render_target.fill_rectangle(bounds, normal_brush);
          }


          buffer.resize(list.GetTextLength(context.itemID), '\0');

          list.GetText(context.itemID, buffer.data());

          // TODO cache format
          win32::directwrite::text_format format({ .family_name = L"Segoe UI", .size = 18.0f });

          format.object().SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

          auto text_brush = render_target.create_solid_color_brush(D2D1::ColorF(text_color, 1.0));

          render_target.draw_text(buffer, format, bounds, text_brush);

          render_target.end_draw();

          return TRUE;
        }

        if (message == WM_NCDESTROY)
        {
          ::RemoveWindowSubclass(parent, handle_parent_message, 0);
        }

        return DefSubclassProc(parent, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(WC_LISTBOX, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }
}// namespace win32