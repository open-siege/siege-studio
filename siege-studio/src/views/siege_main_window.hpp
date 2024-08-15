#ifndef SIEGE_MAIN_WINDOW_HPP
#define SIEGE_MAIN_WINDOW_HPP

#include <oleacc.h>
#include <dwmapi.h>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/shell.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/content_module.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include "views/theme_view.hpp"
#include <map>
#include <spanstream>


namespace siege::views
{
  struct cursor_deleter
  {
    void operator()(HCURSOR cursor)
    {
      //      assert(::DestroyCursor(cursor) == TRUE);
    }
  };

  win32::com::com_ptr<IAccPropServices> get_acc_props()
  {
    win32::com::com_ptr<IAccPropServices> result;
    if (CoCreateInstance(CLSID_CAccPropServices, nullptr, CLSCTX_SERVER, IID_IAccPropServices, result.put_void()) != S_OK)
    {
      throw std::runtime_error("Could not create accessible service");
    }

    return result;
  }

  struct siege_main_window final : win32::window_ref
    , win32::tree_view::notifications
    , win32::tab_control::notifications
    , win32::tool_bar::notifications
    , win32::menu::notifications
  {
    using win32::tree_view::notifications::wm_notify;
    using win32::tool_bar::notifications::wm_notify;
    using win32::menu::notifications::wm_draw_item;
    using win32::tab_control::notifications::wm_draw_item;

    win32::tree_view dir_list;
    win32::button separator;
    win32::tab_control tab_control;
    //    win32::button close_button;
    win32::window theme_window;

    std::list<platform::content_module> loaded_modules;
    std::map<std::wstring, std::int32_t> extensions;
    std::set<std::u16string> categories;

    std::list<std::filesystem::path> folders;
    std::list<std::filesystem::path> files;
    std::list<std::filesystem::path>::iterator selected_file;

    std::uint32_t open_id = RegisterWindowMessageW(L"COMMAND_OPEN");
    std::uint32_t open_new_tab_id = RegisterWindowMessageW(L"COMMAND_OPEN_NEW_TAB");
    std::uint32_t open_workspace_id = RegisterWindowMessageW(L"COMMAND_OPEN_WORKSPACE");
    std::uint32_t edit_theme_id = RegisterWindowMessageW(L"COMMAND_EDIT_THEME");

    bool is_resizing = false;
    win32::auto_handle<HCURSOR, cursor_deleter> resize_cursor;
    HCURSOR previous_cursor = nullptr;
    SIZE tree_size{};

    std::wstring buffer;

    win32::tool_bar main_menu;
    std::array<win32::menu, 4> popup_menus;
    std::vector<MSAAMENUINFO> menu_item_text;

    HIMAGELIST shell_images = nullptr;

    bool is_dark_mode = false;

    siege_main_window(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self), tab_control(nullptr)
    {
      std::filesystem::path app_path = std::filesystem::path(win32::module_ref(params.hInstance).GetModuleFileName()).parent_path();
      loaded_modules = platform::content_module::load_modules(std::filesystem::path(app_path));

      buffer.resize(64);

      for (auto& module : loaded_modules)
      {
        auto module_exts = module.get_supported_extensions();
        std::transform(module_exts.begin(), module_exts.end(), std::inserter(extensions, extensions.begin()), [&](auto ext) {
          return std::make_pair(std::move(ext), module.get_default_file_icon());
        });

        auto category_exts = module.get_supported_format_categories();
        std::transform(category_exts.begin(), category_exts.end(), std::inserter(categories, categories.begin()), [&](auto& ext) {
          return std::move(ext);
        });
      }

      selected_file = files.end();

      this->SetPropW(L"AppsUseDarkTheme", is_dark_mode);

      resize_cursor.reset((HCURSOR)LoadImageW(nullptr, IDC_SIZEWE, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED));
    }

    void repopulate_tree_view(std::filesystem::path path)
    {
      dir_list.DeleteItem(nullptr);
      folders.clear();
      files.clear();

      auto& current_path = folders.emplace_back(std::move(path));
      std::array<win32::tree_view_item, 1> root{ win32::tree_view_item(current_path) };

      HRESULT hresult = S_FALSE;

      if (!shell_images)
      {
        hresult = SHGetImageList(SHIL_SMALL, IID_IImageList, (void**)&shell_images);
      }

      SHSTOCKICONINFO info{ .cbSize = sizeof(SHSTOCKICONINFO) };

      if (shell_images)
      {
        dir_list.SetImageList(TVSIL_NORMAL, shell_images);
        hresult = SHGetStockIconInfo(SIID_FOLDER, SHGSI_SYSICONINDEX, &info);

        if (hresult == S_OK)
        {
          root[0].item.iImage = info.iSysImageIndex;
        }
      }

      for (auto const& dir_entry : std::filesystem::directory_iterator{ current_path })
      {
        if (dir_entry.is_directory())
        {
          folders.emplace_back(dir_entry.path());
        }
        else
        {
          files.emplace_back(dir_entry.path());
        }
      }

      for (auto& folder : folders)
      {
        if (folder == current_path)
        {
          continue;
        }
        auto& info = root[0].children.emplace_back(folder, folder.filename());
        info.item.iImage = root[0].item.iImage;
        info.item.iSelectedImage = root[0].item.iImage;
      }

      for (auto& file : files)
      {
        auto& tree_item = root[0].children.emplace_back(file, file.filename());

        auto ext_icon = extensions.find(file.extension());

        if (ext_icon != extensions.end())
        {
          hresult = SHGetStockIconInfo(SHSTOCKICONID(ext_icon->second), SHGSI_SYSICONINDEX, &info);

          if (hresult == S_OK)
          {
            tree_item.item.iImage = info.iSysImageIndex;
            tree_item.item.iSelectedImage = info.iSysImageIndex;
          }
        }
      }

      dir_list.InsertRoots(root);
    }

    LRESULT static CALLBACK static_sub_class(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
      auto self = (siege_main_window*)dwRefData;
      if (msg == WM_MOUSEMOVE)
      {
        auto previous = ::SetCursor(self->resize_cursor);

        if (previous != self->resize_cursor)
        {
          self->previous_cursor = previous;
        }
      }

      if (msg == WM_LBUTTONDOWN)
      {
        self->is_resizing = true;
      }

      if (msg == WM_LBUTTONUP)
      {
        self->is_resizing = false;
      }

      if (self->is_resizing && (msg == WM_MOUSEMOVE || msg == WM_LBUTTONDOWN))
      {
        POINT temp{};
        if (GetCursorPos(&temp) && ScreenToClient(*self, &temp))
        {
          auto window_rect = self->dir_list.GetWindowRect();
          auto size = SIZE{ .cx = temp.x, .cy = window_rect->bottom - window_rect->top };

          self->tree_size.cx = temp.x;

          self->on_size(*self->GetClientSize());
        }
      }

      if (msg == WM_MOUSELEAVE)
      {
        ::SetCursor(self->previous_cursor);
      }

      if (msg == WM_NCDESTROY)
      {
        TRACKMOUSEEVENT event{
          .cbSize = sizeof(TRACKMOUSEEVENT),
          .dwFlags = TME_LEAVE | TME_CANCEL,
          .hwndTrack = hWnd
        };
        ::TrackMouseEvent(&event);
        RemoveWindowSubclass(hWnd, static_sub_class, uIdSubclass);
      }

      return DefSubclassProc(hWnd, msg, wParam, lParam);
    }

    auto wm_create()
    {
      win32::window_factory factory(ref());

      dir_list = *factory.CreateWindowExW<win32::tree_view>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE });

      separator = *factory.CreateWindowEx<win32::button>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | BS_FLAT });

      if (::SetWindowSubclass(separator, static_sub_class, (UINT_PTR)this, (DWORD_PTR)this))
      {
        TRACKMOUSEEVENT event{
          .cbSize = sizeof(TRACKMOUSEEVENT),
          .dwFlags = TME_LEAVE,
          .hwndTrack = separator,
          .dwHoverTime = HOVER_DEFAULT
        };
        assert(::TrackMouseEvent(&event) == TRUE);
      }

      repopulate_tree_view(std::filesystem::current_path());

      tab_control = *factory.CreateWindowExW<win32::tab_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | TCS_MULTILINE | TCS_FORCELABELLEFT });
      tab_control.InsertItem(0, TCITEMW{
                                  .mask = TCIF_TEXT,
                                  .pszText = const_cast<wchar_t*>(L"+"),
                                });

      // close_button = *factory.CreateWindowExW<win32::button>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"X" });


      main_menu = *factory.CreateWindowExW<win32::tool_bar>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | CCS_NOPARENTALIGN | CCS_NODIVIDER });

      main_menu.InsertButton(
        -1, TBBUTTON{ .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_GROUP | BTNS_AUTOSIZE | BTNS_SHOWTEXT | BTNS_DROPDOWN, .iString = (INT_PTR)L"&File" });

      main_menu.InsertButton(
        -1, TBBUTTON{ .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_GROUP | BTNS_AUTOSIZE | BTNS_SHOWTEXT | BTNS_DROPDOWN, .iString = (INT_PTR)L"Edit" });

      main_menu.InsertButton(
        -1, TBBUTTON{ .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_GROUP | BTNS_AUTOSIZE | BTNS_SHOWTEXT | BTNS_DROPDOWN, .iString = (INT_PTR)L"View" });

      main_menu.InsertButton(
        -1, TBBUTTON{ .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_GROUP | BTNS_AUTOSIZE | BTNS_SHOWTEXT | BTNS_DROPDOWN, .iString = (INT_PTR)L"Help" });

      VARIANT var{};
      var.vt = VT_I4;
      var.lVal = ROLE_SYSTEM_MENUBAR;

      auto acc_props = get_acc_props();
      assert(acc_props->SetHwndProp(main_menu, OBJID_CLIENT, CHILDID_SELF, PROPID_ACC_ROLE, var) == S_OK);


      menu_item_text.reserve(32);

      std::size_t id = 1u;
      popup_menus[0] = win32::menu(::CreatePopupMenu());
      popup_menus[1] = win32::menu(::CreatePopupMenu());

      auto popup = MF_OWNERDRAW | MF_POPUP;
      auto string = MF_OWNERDRAW | MF_STRING;
      auto separator = MF_OWNERDRAW | MF_SEPARATOR;

      MENUITEMINFOW info{
        .cbSize = sizeof(MENUITEMINFOW),
        .fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID
      };

      info.fType = string;
      info.dwItemData = (ULONG_PTR)&menu_item_text.emplace_back(MSAAMENUINFO{ MSAA_MENU_SIG, 0, (wchar_t*)L"Open..." });
      info.wID = open_id;
      InsertMenuItemW(popup_menus[0], open_id, FALSE, &info);

      info.dwItemData = (ULONG_PTR)&menu_item_text.emplace_back(MSAAMENUINFO{ MSAA_MENU_SIG, 0, (wchar_t*)L"Open in New Tab..." });
      info.wID = open_new_tab_id;
      InsertMenuItemW(popup_menus[0], open_new_tab_id, FALSE, &info);

      info.dwItemData = (ULONG_PTR)&menu_item_text.emplace_back(MSAAMENUINFO{ MSAA_MENU_SIG, 0, (wchar_t*)L"Open Folder as Workspace" });
      info.wID = open_workspace_id;
      InsertMenuItemW(popup_menus[0], open_workspace_id, FALSE, &info);

      AppendMenuW(popup_menus[0], separator, id++, nullptr);

      info.wID = RegisterWindowMessageW(L"COMMAND_EXIT");
      info.dwItemData = (ULONG_PTR)&menu_item_text.emplace_back(MSAAMENUINFO{ MSAA_MENU_SIG, 0, (wchar_t*)L"Quit" });
      InsertMenuItemW(popup_menus[0], RegisterWindowMessageW(L"COMMAND_EXIT"), FALSE, &info);

      info.wID = edit_theme_id;
      info.dwItemData = (ULONG_PTR)&menu_item_text.emplace_back(MSAAMENUINFO{ MSAA_MENU_SIG, 0, (wchar_t*)L"Theme" });
      InsertMenuItemW(popup_menus[1], edit_theme_id, FALSE, &info);

      for (auto& item : menu_item_text)
      {
        item.cchWText = std::wcslen(item.pszWText);
      }

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    std::optional<LRESULT> wm_destroy()
    {
      PostQuitMessage(0);
      return 0;
    }


    void on_size(SIZE total_size)
    {
      auto menu_size = total_size;
      menu_size.cy = menu_size.cy / 20;

      main_menu.SetWindowPos(POINT{});
      main_menu.SetWindowPos(menu_size);
      main_menu.AutoSize();

      auto new_size = total_size;
      new_size.cy -= menu_size.cy;


      if (tree_size.cx == 0)
      {
        tree_size = new_size;
        tree_size.cx = tree_size.cx / 6;
      }

      tree_size.cy = new_size.cy;

      auto divider = new_size;
      divider.cx /= 128;

      auto right_size = new_size;
      right_size.cx -= tree_size.cx - divider.cx;

      dir_list.SetWindowPos(POINT{ .y = menu_size.cy });
      dir_list.SetWindowPos(tree_size);

      separator.SetWindowPos(POINT{ .x = tree_size.cx, .y = menu_size.cy });
      separator.SetWindowPos(divider);

      tab_control.SetWindowPos(POINT{ .x = tree_size.cx + divider.cx, .y = menu_size.cy });
      tab_control.SetWindowPos(right_size);

      auto width = std::clamp<int>(right_size.cx / tab_control.GetItemCount() + 1, 150, right_size.cx);

      TabCtrl_SetPadding(tab_control, 10, 0);
      auto old_height = HIWORD(TabCtrl_SetItemSize(tab_control, width, 40));
      TabCtrl_SetItemSize(tab_control, width, old_height);
      auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value); }).value().second;

      // TODO bring this back when the time is right
      /*if (auto count = tab_control.GetItemCount(); count > 1)
      {
        auto rect = tab_control.GetItemRect(tab_control.GetCurrentSelection());
        auto width = 50;
        close_button.SetWindowPos(POINT{ .x = tab_rect.left + rect->right - width, .y = tab_rect.top + rect->top });
        close_button.SetWindowPos(SIZE{ .cx = width, .cy = rect->bottom - rect->top });
        close_button.SetWindowPos(HWND_TOP);
      }*/

      SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&tab_rect));

      for (auto i = 0; i < tab_control.GetItemCount(); ++i)
      {
        auto tab_item = tab_control.GetItem(i);

        if (tab_item->lParam)
        {
          assert(win32::window_ref(win32::hwnd_t(tab_item->lParam)).SetWindowPos(tab_rect));
        }
      }
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      on_size(client_size);

      return 0;
    }

    auto wm_copy_data(win32::copy_data_message<std::byte> message)
    {
      auto filename = GetPropW<wchar_t*>(L"FilePath");

      if (!filename)
      {
        return FALSE;
      }

      siege::platform::storage_info storage{
        .type = siege::platform::storage_info::buffer
      };
      storage.info.data.data = message.data.data();
      storage.info.data.size = message.data.size();

      auto plugin = std::find_if(loaded_modules.begin(), loaded_modules.end(), [&](auto& module) {
        return module.is_stream_supported(storage);
      });

      if (plugin != loaded_modules.end())
      {
        auto class_name = plugin->get_window_class_for_stream(std::move(storage));

        auto tab_rect = tab_control.GetClientRect()
                          .and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value); })
                          .value()
                          .second;

        SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&tab_rect));

        auto child = plugin->CreateWindowExW(::CREATESTRUCTW{
          .hwndParent = *this,
          .cy = tab_rect.bottom - tab_rect.top,
          .cx = tab_rect.right - tab_rect.left,
          .y = tab_rect.top,
          .x = tab_rect.left,
          .style = WS_CHILD,
          .lpszClass = class_name.c_str(),
        });

        if (child->CopyData(*this, COPYDATASTRUCT{ .cbData = DWORD(message.data.size()), .lpData = message.data.data() }))
        {
          auto index = tab_control.GetItemCount() - 1;

          tab_control.InsertItem(index, TCITEMW{ .mask = TCIF_TEXT | TCIF_PARAM, .pszText = filename, .lParam = win32::lparam_t(child->get()) });

          SetWindowLongPtrW(*child, GWLP_ID, index + 1);

          wm_notify(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGING });

          tab_control.SetCurrentSelection(index);

          wm_notify(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGE });
        }
        else
        {
          ::DestroyWindow(*child);
        }
      }

      return FALSE;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        HKEY key = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER,
              L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
              0,
              KEY_READ,
              &key)
            == ERROR_SUCCESS)
        {
          DWORD value = 0;
          DWORD size = sizeof(DWORD);

          DWORD type = REG_DWORD;
          if (RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS)
          {
            is_dark_mode = value == 0;

            this->SetPropW(L"AppsUseDarkTheme", is_dark_mode);

            COLORREF bk_color = RGB(0x20, 0x20, 0x20);
            COLORREF text_color = 0x00FFFFFF;
            COLORREF text_bk_color = RGB(0x2b, 0x2b, 0x2b);
            COLORREF text_highlight_color = RGB(0x2d, 0x2d, 0x2d);
            COLORREF btn_shadow_color = 0x00AAAAAA;

            if (!is_dark_mode)
            {
              bk_color = RGB(0xfb, 0xfb, 0xfb);
              text_color = RGB(0x1a, 0x1a, 0x1a);
              text_bk_color = RGB(0xf3, 0xf3, 0xf3);
              text_highlight_color = RGB(127, 127, 255);
              btn_shadow_color = RGB(1, 1, 1);
            }

            this->SetPropW(win32::properties::button::bk_color, bk_color);
            this->SetPropW(win32::properties::button::text_color, text_color);
            this->SetPropW(win32::properties::button::line_color, text_color);

            this->SetPropW(win32::properties::tree_view::bk_color, bk_color);
            this->SetPropW(win32::properties::tree_view::text_color, text_color);
            this->SetPropW(win32::properties::tree_view::line_color, 0x00383838);
            this->SetPropW(win32::properties::list_view::bk_color, bk_color);
            this->SetPropW(win32::properties::list_view::text_color, text_color);
            this->SetPropW(win32::properties::list_view::text_bk_color, text_bk_color);
            this->SetPropW(win32::properties::list_view::outline_color, 0x00AAAAAA);

            this->SetPropW(win32::properties::list_box::bk_color, bk_color);
            this->SetPropW(win32::properties::list_box::text_color, text_color);
            this->SetPropW(win32::properties::list_box::text_bk_color, text_bk_color);
            this->SetPropW(win32::properties::list_box::text_highlight_color, text_highlight_color);

            this->SetPropW(win32::properties::header::bk_color, bk_color);
            this->SetPropW(win32::properties::header::text_color, text_color);
            this->SetPropW(win32::properties::header::text_bk_color, text_bk_color);
            this->SetPropW(win32::properties::header::text_highlight_color, text_highlight_color);

            this->SetPropW(win32::properties::tab_control::bk_color, bk_color);
            this->SetPropW(win32::properties::tab_control::text_color, text_color);
            this->SetPropW(win32::properties::tab_control::text_bk_color, text_bk_color);
            this->SetPropW(win32::properties::tab_control::text_highlight_color, text_highlight_color);

            this->SetPropW(win32::properties::tool_bar::bk_color, bk_color);
            this->SetPropW(win32::properties::tool_bar::text_color, text_color);
            this->SetPropW(win32::properties::tool_bar::btn_face_color, text_bk_color);
            this->SetPropW(win32::properties::tool_bar::btn_highlight_color, text_highlight_color);
            this->SetPropW(win32::properties::tool_bar::btn_shadow_color, btn_shadow_color);
            this->SetPropW(win32::properties::window::bk_color, bk_color);
            this->SetPropW(win32::properties::menu::bk_color, bk_color);
            this->SetPropW(win32::properties::menu::text_color, text_color);
            this->SetPropW(win32::properties::menu::text_highlight_color, text_highlight_color);
            this->SetPropW(win32::properties::static_control::bk_color, bk_color);
            this->SetPropW(win32::properties::static_control::text_color, text_color);

            MENUINFO mi = { 0 };
            mi.cbSize = sizeof(mi);
            mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS | MIM_STYLE;
            mi.hbrBack = win32::get_solid_brush(bk_color);
            mi.dwStyle = MNS_MODELESS;

            SetMenuInfo(popup_menus[0], &mi);
            SetMenuInfo(popup_menus[1], &mi);

            win32::apply_theme(*this, dir_list);
            win32::apply_theme(*this, tab_control);
            win32::apply_theme(*this, main_menu);
            win32::apply_theme(*this, separator);
            win32::apply_theme(*this, *this);

            for (auto i = 0; i < tab_control.GetItemCount(); ++i)
            {
              auto tab_item = tab_control.GetItem(i);

              if (tab_item->lParam)
              {
                ::SendMessageW((HWND)tab_item->lParam, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet");
              }
            }
          }

          RegCloseKey(key);
          return 0;
        }
      }

      return std::nullopt;
    }

    bool menu_open = false;
    bool menu_bar_tracked = false;
    bool hot_tracking_enabled = false;

    std::optional<win32::lresult_t> wm_enter_menu_loop(bool is_popup_menu)
    {
      menu_open = true;

      return 0;
    }

    std::optional<win32::lresult_t> wm_exit_menu_loop(bool is_popup_menu)
    {
      menu_open = false;

      if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0)
      {
        hot_tracking_enabled = false;
      }
      else if ((GetKeyState(VK_RBUTTON) & 0x8000) != 0)
      {
        hot_tracking_enabled = false;
      }

      auto last_item = SendMessageW(this->main_menu, TB_GETHOTITEM, 0, 0);
      if (last_item >= 0)
      {
        static NMTBHOTITEM notice{};
        notice = {
          .hdr{
            .hwndFrom = this->main_menu,
            .code = TBN_HOTITEMCHANGE,
          },
          .idOld = last_item,
          .idNew = last_item,
          .dwFlags = HICF_RESELECT
        };
        PostMessageW(main_menu, WM_NOTIFY, (WPARAM)this->main_menu.get(), (LPARAM)&notice);
      }

      return 0;
    }


    std::optional<win32::lresult_t> wm_notify(win32::tool_bar menu_bar, const NMTBHOTITEM& notice) override
    {
      if (notice.dwFlags & HICF_ENTERING)
      {
        menu_bar_tracked = true;
      }
      else if (notice.dwFlags & HICF_LEAVING)
      {
        menu_bar_tracked = false;
      }

      if (notice.idNew != notice.idOld && menu_bar_tracked && menu_open)
      {
        EndMenu();
      }

      if (!menu_open && hot_tracking_enabled)
      {
        auto window_rect = main_menu.GetWindowRect();
        auto height = window_rect->bottom - window_rect->top;

        RECT item_rect{};

        SendMessageW(main_menu, TB_GETITEMRECT, notice.idNew, (LPARAM)&item_rect);
        window_rect->left += item_rect.left;
        window_rect->top += item_rect.top;
        window_rect->right += item_rect.right;
        window_rect->bottom += item_rect.bottom;
        ::TrackPopupMenu(popup_menus[notice.idNew], 0, window_rect->left, window_rect->top + height, 0, *this, nullptr);
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_notify(win32::tool_bar menu_bar, const NMTOOLBARW& notice) override
    {
      auto window_rect = menu_bar.GetWindowRect();
      auto height = window_rect->bottom - window_rect->top;

      window_rect->left += notice.rcButton.left;
      window_rect->top += notice.rcButton.top;
      window_rect->right += notice.rcButton.right;
      window_rect->bottom += notice.rcButton.bottom;

      auto result = ::TrackPopupMenu(this->popup_menus[notice.iItem], 0, window_rect->left, window_rect->top + height, 0, *this, nullptr);
      hot_tracking_enabled = true;

      return TBDDRET_DEFAULT;
    }

    SIZE wm_measure_item(win32::menu, const MEASUREITEMSTRUCT& item) override
    {
      HDC hDC = ::GetDC(*this);
      auto font = win32::load_font(LOGFONTW{
        .lfPitchAndFamily = VARIABLE_PITCH,
        .lfFaceName = L"Segoe UI" });

      SelectFont(hDC, font);

      std::wstring text = item.itemData ? ((MSAAMENUINFO*)item.itemData)->pszWText : L"__________";
      SIZE char_size{};
      auto result = GetTextExtentPoint32W(hDC, text.data(), text.size(), &char_size);
      char_size.cx += (GetSystemMetrics(SM_CXMENUCHECK) * 2);
      ReleaseDC(*this, hDC);

      return char_size;
    }

    std::optional<win32::lresult_t> wm_draw_item(win32::menu, DRAWITEMSTRUCT& item) override
    {
      auto font = win32::load_font(LOGFONTW{
        .lfPitchAndFamily = VARIABLE_PITCH,
        .lfFaceName = L"Segoe UI" });

      SelectFont(item.hDC, font);


      auto bk_color = *this->FindPropertyExW<COLORREF>(win32::properties::menu::bk_color);
      auto text_highlight_color = *this->FindPropertyExW<COLORREF>(win32::properties::menu::text_highlight_color);
      auto black_brush = win32::get_solid_brush(bk_color);
      auto grey_brush = win32::get_solid_brush(*this->FindPropertyExW<COLORREF>(win32::properties::menu::text_highlight_color));
      auto text_color = this->FindPropertyExW<COLORREF>(win32::properties::menu::text_color);

      auto context = win32::gdi::drawing_context_ref(item.hDC);

      SelectObject(context, GetStockObject(DC_PEN));
      SelectObject(context, GetStockObject(DC_BRUSH));

      SetBkMode(context, TRANSPARENT);

      if (item.itemState & ODS_HOTLIGHT)
      {
        context.FillRect(item.rcItem, grey_brush);
      }
      else if (item.itemState & ODS_SELECTED)
      {
        context.FillRect(item.rcItem, grey_brush);
      }
      else
      {
        context.FillRect(item.rcItem, black_brush);
      }

      if (text_color)
      {
        ::SetTextColor(context, *text_color);
      }

      auto rect = item.rcItem;
      rect.left += (rect.right - rect.left) / 10;
      if (item.itemData)
      {
        auto& menu_item_info = *(MSAAMENUINFO*)item.itemData;
        ::DrawTextW(context, menu_item_info.pszWText, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
      }

      return TRUE;
    }

    std::optional<LRESULT> wm_notify(win32::tab_control sender, const NMHDR& notification) override
    {
      auto code = notification.code;

      switch (code)
      {
      case TCN_SELCHANGING: {
        auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);

        auto tab_item = tab_control.GetItem(current_index);
        ::ShowWindow(win32::hwnd_t(tab_item->lParam), SW_HIDE);
        return 0;
      }
      case TCN_SELCHANGE: {
        auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);

        if (current_index == -1)
        {
          return 0;
        }

        auto tab_item = tab_control.GetItem(current_index);


        if (tab_item->lParam == 0)
        {
          return std::nullopt;
        }

        auto temp_window = win32::window_ref(win32::hwnd_t(tab_item->lParam));

        temp_window.SetWindowPos(HWND_TOP);

        auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value); }).value().second;

        /*if (auto count = tab_control.GetItemCount(); count > 1)
        {
          auto rect = tab_control.GetItemRect(current_index);
          auto width = (rect->right - rect->left) / 3;
          close_button.SetWindowPos(POINT{ .x = tab_rect.left + rect->right - width, .y = tab_rect.top + rect->top });
          close_button.SetWindowPos(SIZE{ .cx = width, .cy = rect->bottom - rect->top });
          close_button.SetWindowPos(HWND_TOP);
        }*/


        ShowWindow(win32::hwnd_t(tab_item->lParam), SW_SHOW);

        return 0;
      }
      default: {
        return std::nullopt;
      }
      }
    }

    bool AddTabFromPath(std::filesystem::path file_path)
    {
      try
      {
        auto path_ref = file_path.c_str();
        win32::file file_to_read(file_path, GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

        auto mapping = file_to_read.CreateFileMapping(std::nullopt, PAGE_READONLY, 0, 0, L"");

        if (!mapping)
        {
          return false;
        }

        std::size_t size = (std::size_t)file_to_read.GetFileSizeEx().value_or(LARGE_INTEGER{}).QuadPart;

        auto view = mapping->MapViewOfFile(FILE_MAP_READ, size);

        siege::platform::storage_info storage{
          .type = siege::platform::storage_info::buffer
        };
        storage.info.data.data = (std::byte*)view.get();
        storage.info.data.size = size;

        auto plugin = std::find_if(loaded_modules.begin(), loaded_modules.end(), [&](auto& module) {
          return module.is_stream_supported(storage);
        });

        if (plugin != loaded_modules.end())
        {
          auto class_name = plugin->get_window_class_for_stream(storage);

          if (::FindWindowExW(*this, nullptr, class_name.c_str(), file_path.c_str()) != nullptr)
          {
            return false;
          }

          auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value); }).value().second;

          SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&tab_rect));

          auto child = plugin->CreateWindowExW(::CREATESTRUCTW{
            .hwndParent = *this,
            .cy = tab_rect.bottom - tab_rect.top,
            .cx = tab_rect.right - tab_rect.left,
            .y = tab_rect.top,
            .x = tab_rect.left,
            .style = WS_CHILD,
            .lpszName = file_path.c_str(),
            .lpszClass = class_name.c_str(),
          });


          assert(child);

          COPYDATASTRUCT data{
            .cbData = DWORD(size),
            .lpData = view.get()
          };

          child->SetPropW(L"FilePath", path_ref);

          if (child->CopyData(*this, data))
          {
            child->RemovePropW(L"FilePath");

            auto index = tab_control.GetItemCount() - 1;

            tab_control.InsertItem(index, TCITEMW{
                                            .mask = TCIF_TEXT | TCIF_PARAM,
                                            .pszText = (wchar_t*)file_path.filename().c_str(),
                                            .lParam = win32::lparam_t(child->get()),
                                          });

            SetWindowLongPtrW(*child, GWLP_ID, index + 1);

            wm_notify(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGING });

            tab_control.SetCurrentSelection(index);

            wm_notify(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGE });
          }
          else
          {
            child->RemovePropW(L"FilePath");
            ::DestroyWindow(*child);
          }

          return true;
        }
      }
      catch (...)
      {
      }

      return false;
    }

    std::optional<LRESULT> wm_notify(win32::tree_view sender, const NMHDR& notification) override
    {
      auto code = notification.code;

      if (code == NM_DBLCLK && sender == dir_list && selected_file != files.end() && AddTabFromPath(*selected_file))
      {
        return 0;
      }

      return std::nullopt;
    }

    std::optional<LRESULT> wm_notify(win32::tree_view, const NMTREEVIEWW& notification) override
    {
      auto sender = notification.hdr.hwndFrom;
      auto code = notification.hdr.code;

      switch (code)
      {
      case TVN_SELCHANGED: {
        selected_file = std::find_if(files.begin(), files.end(), [&](const auto& existing) {
          return existing.c_str() == (wchar_t*)notification.itemNew.lParam;
        });

        auto folder = std::find_if(folders.begin(), folders.end(), [&](const auto& existing) {
          return existing.c_str() == (wchar_t*)notification.itemNew.lParam;
        });

        if (folder != folders.end() && notification.itemNew.cChildren == 0)
        {
          for (auto const& dir_entry : std::filesystem::directory_iterator{ *folder })
          {
            if (!dir_entry.is_directory())
            {
              auto& temp = files.emplace_back(dir_entry.path());
              win32::tree_view_item child(temp, temp.filename());
              child.hParent = notification.itemNew.hItem;
              child.hInsertAfter = TVI_LAST;


              dir_list.InsertItem(child);
            }
          }
        }

        return 0;
      }
      case TVN_ITEMEXPANDING: {

        return FALSE;
      }
      default: {
        return std::nullopt;
      }
      }
    }

    std::optional<LRESULT> wm_command(win32::menu, int identifier) override
    {
      hot_tracking_enabled = false;
      if (identifier == edit_theme_id)
      {
        theme_window = *win32::window_module_ref::current_module().CreateWindowExW(CREATESTRUCTW{
          .lpCreateParams = (HWND)this->get(),
          .hwndParent = *this,
          .cx = CW_USEDEFAULT,
          .x = CW_USEDEFAULT,
          .style = WS_OVERLAPPEDWINDOW,
          .lpszName = L"Theme Window",
          .lpszClass = win32::type_name<theme_view>().c_str() });

        ShowWindow(theme_window, SW_NORMAL);
      }

      if (identifier == open_workspace_id)
      {
        auto prop = this->FindPropertyExW(win32::properties::button::bk_color);
        auto dialog = win32::com::CreateFileOpenDialog();

        if (dialog)
        {
          auto open_dialog = *dialog;
          open_dialog->SetOptions(FOS_PICKFOLDERS);
          auto result = open_dialog->Show(nullptr);

          if (result == S_OK)
          {
            auto selection = open_dialog.GetResult();

            if (selection)
            {
              auto path = selection.value().GetFileSysPath();
              std::filesystem::current_path(*path);

              repopulate_tree_view(std::move(*path));
              return 0;
            }
          }
        }
      }

      if (identifier == open_id)
      {
        auto dialog = win32::com::CreateFileOpenDialog();

        if (dialog)
        {
          struct filter : COMDLG_FILTERSPEC
          {
            std::wstring name;
            std::wstring spec;

            filter(std::wstring name, std::wstring spec) noexcept : name(std::move(name)), spec(std::move(spec))
            {
              this->pszName = this->name.c_str();
              this->pszSpec = this->spec.c_str();
            }
          };

          std::vector<filter> temp;

          temp.reserve(extensions.size());

          for (auto& extension : extensions)
          {
            temp.emplace_back(extension.first, L"*" + extension.first);
          }

          std::vector<COMDLG_FILTERSPEC> filetypes(temp.begin(), temp.end());
          dialog.value()->SetFileTypes(filetypes.size(), filetypes.data());
          auto result = dialog.value()->Show(nullptr);

          if (result == S_OK)
          {
            auto item = dialog.value().GetResult();

            if (item)
            {
              auto path = item.value().GetFileSysPath();

              if (path && AddTabFromPath(*path))
              {
                return 0;
              }
            }
          }
        }
      }

      return std::nullopt;
    }
  };
}// namespace siege::views

#endif