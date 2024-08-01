#ifndef SIEGE_MAIN_WINDOW_HPP
#define SIEGE_MAIN_WINDOW_HPP

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
#include <dwmapi.h>

namespace siege::views
{
  struct cursor_deleter
  {
    void operator()(HCURSOR cursor)
    {
      //      assert(::DestroyCursor(cursor) == TRUE);
    }
  };

  struct siege_main_window final : win32::window_ref
    , win32::tree_view::notifications
    , win32::tab_control::notifications
    , win32::menu::notifications
  {
    using win32::tree_view::notifications::wm_notify;
    using win32::menu::notifications::wm_draw_item;
    using win32::tab_control::notifications::wm_draw_item;

    win32::tree_view dir_list;
    win32::button separator;
    win32::tab_control tab_control;
    win32::button close_button;
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

    HMENU light_menu;
    HMENU dark_menu;
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

      light_menu = ::CreateMenu();
      dark_menu = ::CreateMenu();


      auto menus = std::array<HMENU, 2>{ light_menu, dark_menu };

      bool is_light = true;

      for (auto& menu : menus)
      {
        std::size_t id = 1u;
        auto file_menu = ::CreatePopupMenu();
        auto edit_menu = ::CreatePopupMenu();
        auto popup = is_light ? MF_POPUP : MF_OWNERDRAW | MF_POPUP;
        auto string = is_light ? MF_STRING : MF_OWNERDRAW | MF_STRING;
        auto separator = is_light ? MF_SEPARATOR : MF_OWNERDRAW | MF_SEPARATOR;

        AppendMenuW(menu, popup, reinterpret_cast<INT_PTR>(file_menu), L"File");
        AppendMenuW(file_menu, string, open_id, L"Open...");
        AppendMenuW(file_menu, string, open_new_tab_id, L"Open in New Tab...");
        AppendMenuW(file_menu, string, open_workspace_id, L"Open Folder as Workspace");
        AppendMenuW(file_menu, separator, id++, nullptr);
        AppendMenuW(file_menu, string, RegisterWindowMessageW(L"COMMAND_EXIT"), L"Quit");

        AppendMenuW(menu, popup, reinterpret_cast<INT_PTR>(edit_menu), L"Edit");
        AppendMenuW(edit_menu, string, edit_theme_id, L"Theme");
        AppendMenuW(menu, string, id++, L"View");
        AppendMenuW(menu, string, id++, L"Help");
        is_light = false;
      }

      MENUINFO mi = { 0 };
      mi.cbSize = sizeof(mi);
      mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
      mi.hbrBack = ::CreateSolidBrush(0x00383838);

      SetMenuInfo(dark_menu, &mi);
      SetMenu(self, light_menu);
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

      close_button = *factory.CreateWindowExW<win32::button>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"X" });

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    std::optional<LRESULT> wm_destroy()
    {
      PostQuitMessage(0);
      return 0;
    }

    void on_size(SIZE new_size)
    {
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

      dir_list.SetWindowPos(POINT{});
      dir_list.SetWindowPos(tree_size);

      separator.SetWindowPos(POINT{ .x = tree_size.cx });
      separator.SetWindowPos(divider);

      tab_control.SetWindowPos(POINT{ .x = tree_size.cx + divider.cx });
      tab_control.SetWindowPos(right_size);

      auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value); }).value().second;

      if (auto count = tab_control.GetItemCount(); count > 1)
      {
        auto rect = tab_control.GetItemRect(tab_control.GetCurrentSelection());
        auto width = (rect->right - rect->left) / 12;
        close_button.SetWindowPos(POINT{ .x = tab_rect.left + rect->right - width, .y = tab_rect.top + rect->top });
        close_button.SetWindowPos(SIZE{ .cx = width, .cy = rect->bottom - rect->top });
        close_button.SetWindowPos(HWND_TOP);
      }

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
            if (value == 1)
            {
              static auto props = [] {
                std::vector<std::wstring_view> results;
                results.reserve(32);
                std::copy(win32::properties::tree_view::props.begin(), win32::properties::tree_view::props.end(), std::back_inserter(results));
                std::copy(win32::properties::button::props.begin(), win32::properties::button::props.end(), std::back_inserter(results));
                std::copy(win32::properties::list_view::props.begin(), win32::properties::list_view::props.end(), std::back_inserter(results));
                std::copy(win32::properties::tool_bar::props.begin(), win32::properties::tool_bar::props.end(), std::back_inserter(results));
                std::copy(win32::properties::list_box::props.begin(), win32::properties::list_box::props.end(), std::back_inserter(results));
                std::copy(win32::properties::window::props.begin(), win32::properties::window::props.end(), std::back_inserter(results));
                std::copy(win32::properties::menu::props.begin(), win32::properties::menu::props.end(), std::back_inserter(results));
                std::copy(win32::properties::header::props.begin(), win32::properties::header::props.end(), std::back_inserter(results));
                std::copy(win32::properties::static_control::props.begin(), win32::properties::static_control::props.end(), std::back_inserter(results));
                std::copy(win32::properties::tab_control::props.begin(), win32::properties::tab_control::props.end(), std::back_inserter(results));
                return results;
              }();

              for (auto& prop : props)
              {
                this->RemovePropW(prop);
              }

              win32::apply_theme(*this, dir_list);
              win32::apply_theme(*this, tab_control);
              SetMenu(*this, light_menu);
              BOOL value = FALSE;
              ::DwmSetWindowAttribute(*this, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
            }
            else
            {
              COLORREF bk_color = 0x00000000;
              COLORREF text_color = 0x00FFFFFF;
              COLORREF text_bk_color = 0x00111111;
              COLORREF text_highlight_color = 0x00383838;

              this->SetPropW(win32::properties::button::bk_color, bk_color);
              this->SetPropW(win32::properties::button::text_color, text_color);

              this->SetPropW(win32::properties::tree_view::bk_color, bk_color);
              this->SetPropW(win32::properties::tree_view::text_color, text_color);
              this->SetPropW(win32::properties::tree_view::line_color, 0x00383838);
              this->SetPropW(win32::properties::list_view::bk_color, bk_color);
              this->SetPropW(win32::properties::list_view::text_color, 0x00FFFFFF);
              this->SetPropW(win32::properties::list_view::text_bk_color, text_bk_color);
              this->SetPropW(win32::properties::list_view::outline_color, 0x00AAAAAA);

              this->SetPropW(win32::properties::list_box::bk_color, bk_color);
              this->SetPropW(win32::properties::list_box::text_color, text_color);
              this->SetPropW(win32::properties::list_box::text_bk_color, text_bk_color);
              this->SetPropW(win32::properties::list_box::text_highlight_color, 0x00383838);

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
              //  this->SetPropW(win32::properties::tool_bar::btn_shadow_color, 0x00AAAAAA);
              // this->SetPropW(win32::properties::tool_bar::btn_shadow_color, 0x00AAAAAA);
              // this->SetPropW(win32::properties::tool_bar::text_highlight_color, 0x00AAAAAA);
              // this->SetPropW(win32::properties::tool_bar::mark_color, 0x00AAAAAA);
              this->SetPropW(win32::properties::window::bk_color, bk_color);
              this->SetPropW(win32::properties::static_control::bk_color, bk_color);
              this->SetPropW(win32::properties::static_control::text_color, text_color);

              win32::apply_theme(*this, dir_list);
              win32::apply_theme(*this, tab_control);

              SetMenu(*this, dark_menu);
              BOOL value = TRUE;
              ::DwmSetWindowAttribute(*this, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
            }

            this->SetPropW(L"AppsUseDarkTheme", is_dark_mode);

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

    SIZE wm_measure_item(win32::menu, const MEASUREITEMSTRUCT& item) override
    {
      if (item.itemData)
      {
        HDC hDC = ::GetDC(*this);
        TEXTMETRIC tm{};
        ::GetTextMetricsW(hDC, &tm);
        ReleaseDC(*this, hDC);

        return SIZE{ .cx = (LONG)(tm.tmAveCharWidth * std::wcslen((wchar_t*)item.itemData)), .cy = ::GetSystemMetrics(SM_CYMENUSIZE) };
      }
      else
      {
        return SIZE{ .cx = ::GetSystemMetrics(SM_CXMENUSIZE) * 2, .cy = ::GetSystemMetrics(SM_CYMENUSIZE) };
      }
    }

    std::optional<win32::lresult_t> wm_draw_item(win32::menu, DRAWITEMSTRUCT& item) override
    {
      static auto black_brush = ::CreateSolidBrush(0x00000000);
      static auto grey_brush = ::CreateSolidBrush(0x00383838);

      auto context = win32::gdi_drawing_context_ref(item.hDC);

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

      ::SetTextColor(context, 0x00FFFFFF);
      auto rect = item.rcItem;
      rect.left += (rect.right - rect.left) / 10;
      ::DrawTextW(context, (LPCWSTR)item.itemData, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

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

        if (auto count = tab_control.GetItemCount(); count > 1)
        {
          auto rect = tab_control.GetItemRect(current_index);
          auto width = (rect->right - rect->left) / 3;
          close_button.SetWindowPos(POINT{ .x = tab_rect.left + rect->right - width, .y = tab_rect.top + rect->top });
          close_button.SetWindowPos(SIZE{ .cx = width, .cy = rect->bottom - rect->top });
          close_button.SetWindowPos(HWND_TOP);
        }


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
      if (identifier == edit_theme_id)
      {
        theme_window = *win32::window_module_ref::current_module().CreateWindowExW(CREATESTRUCTW{
          .lpCreateParams = (HWND)*this,
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