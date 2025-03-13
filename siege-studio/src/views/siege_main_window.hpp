#ifndef SIEGE_MAIN_WINDOW_HPP
#define SIEGE_MAIN_WINDOW_HPP

#include <dwmapi.h>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/file.hpp>
#include <siege/platform/content_module.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/layout.hpp>
#include <siege/platform/win/dialog.hpp>
#include "views/preferences_view.hpp"
#include "views/about_view.hpp"
#include "views/default_view.hpp"
#include <map>
#include <spanstream>

namespace siege::views
{
  // TODO update tree view to support multiple levels of navigation
  // TODO add filename filter for directory listing
  // TODO add category and extension filter for directory listing
  // TODO Support WM_DROPFILES. Instead of using the templated window handler, it makes sense to
  // move the dispatching into this class to support more bespoke messages.
  struct siege_main_window final : win32::window_ref
    , win32::menu::notifications
  {
    win32::tree_view dir_list;
    win32::popup_menu dir_list_menu;
    win32::button separator;
    win32::tab_control tab_control;
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
    std::uint32_t about_id = RegisterWindowMessageW(L"COMMAND_ABOUT");

    bool is_resizing = false;
    win32::gdi::cursor_ref resize_cursor;
    HCURSOR previous_cursor = nullptr;
    SIZE tree_size{};

    std::wstring buffer;

    win32::menu main_menu;
    std::array<win32::popup_menu, 3> popup_menus;
    win32::popup_menu tab_context_menu;

    win32::image_list shell_images;

    bool is_dark_mode = false;

    std::filesystem::path initial_working_dir;
    win32::gdi::bitmap menu_sizer = [] {
      win32::gdi::bitmap result(SIZE{ 1, ::GetSystemMetrics(SM_CYSIZE) * 2 }, win32::gdi::bitmap::skip_shared_handle);
      return result;
    }();

    siege_main_window(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self), tab_control(nullptr)
    {
      std::filesystem::path app_path = std::filesystem::path(win32::module_ref(params.hInstance).GetModuleFileName()).parent_path();
      loaded_modules = platform::content_module::load_modules(app_path);

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
      win32::set_is_dark_theme(is_dark_mode);

      resize_cursor.reset((HCURSOR)LoadImageW(nullptr, IDC_SIZEWE, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED));

      win32::apply_list_view_theme();
      win32::apply_header_theme();
      win32::apply_tree_view_theme();
      win32::apply_button_theme();
      win32::apply_list_box_theme();
      win32::apply_static_control_theme();
      win32::apply_tab_control_theme();
      win32::apply_tool_bar_theme();
      win32::apply_tooltip_theme();
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
        hresult = SHGetImageList(SHIL_SMALL, IID_IImageList, shell_images.put_void());
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

    LRESULT static CALLBACK button_sub_class(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
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

        auto state = Button_GetState(self->separator);

        Button_SetState(self->separator, state & ~BST_PUSHED);
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
        RemoveWindowSubclass(hWnd, button_sub_class, uIdSubclass);
      }

      return DefSubclassProc(hWnd, msg, wParam, lParam);
    }

    auto wm_create()
    {
      initial_working_dir = fs::current_path();

      dir_list = *win32::CreateWindowExW<win32::tree_view>(CREATESTRUCTW{ .hMenu = dir_list_menu.get(), .hwndParent = *this, .style = WS_CHILD | WS_VISIBLE });
      dir_list.bind_tvn_sel_changed(std::bind_front(&siege_main_window::dir_list_tvn_sel_changed, this));
      dir_list.bind_tvn_item_expanding(std::bind_front(&siege_main_window::dir_list_tvn_item_expanding, this));
      dir_list.bind_nm_dbl_click(std::bind_front(&siege_main_window::dir_list_nm_dbl_click, this));
      dir_list.bind_nm_rclick(std::bind_front(&siege_main_window::dir_list_nm_rclick, this));
      dir_list_menu.AppendMenuW(MF_OWNERDRAW, 1, L"Open in File Explorer");

      separator = *win32::CreateWindowEx<win32::button>(CREATESTRUCTW{ .hwndParent = *this, .style = WS_CHILD | WS_VISIBLE | BS_FLAT });

      if (::SetWindowSubclass(separator, button_sub_class, (UINT_PTR)this, (DWORD_PTR)this))
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
      tab_control = *win32::CreateWindowExW<win32::tab_control>(
        CREATESTRUCTW{
          .hMenu = tab_context_menu,
          .hwndParent = *this,
          .style = WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT,
        });


      tab_control.bind_nm_rclick(std::bind_front(&siege_main_window::tab_control_nm_rclick, this));
      tab_control.bind_tcn_sel_change(std::bind_front(&siege_main_window::tab_control_tcn_sel_change, this));
      tab_control.bind_tcn_sel_changing(std::bind_front(&siege_main_window::tab_control_tcn_sel_changing, this));
      win32::enable_closeable_tabs(tab_control);

      std::size_t id = 1u;

      popup_menus[0].AppendMenuW(MF_OWNERDRAW, open_id, L"Open...");
      popup_menus[0].AppendMenuW(MF_OWNERDRAW, open_new_tab_id, L"Open in New Tab...");
      popup_menus[0].AppendMenuW(MF_OWNERDRAW, open_workspace_id, L"Open Folder as Workspace");
      popup_menus[0].AppendMenuW(MF_SEPARATOR | MF_OWNERDRAW, id++);
      popup_menus[0].AppendMenuW(MF_OWNERDRAW, RegisterWindowMessageW(L"COMMAND_EXIT"), L"Quit");
      popup_menus[1].AppendMenuW(MF_OWNERDRAW, edit_theme_id, L"Preferences");
      popup_menus[2].AppendMenuW(MF_OWNERDRAW, about_id, L"About");

      SetMenu(*this, main_menu.get());

      main_menu.AppendMenuW(MF_POPUP | MF_OWNERDRAW, (UINT_PTR)popup_menus[0].get(), L"File");
      main_menu.AppendMenuW(MF_POPUP | MF_OWNERDRAW, (UINT_PTR)popup_menus[1].get(), L"Edit");
      main_menu.AppendMenuW(MF_POPUP | MF_OWNERDRAW, (UINT_PTR)popup_menus[2].get(), L"Help");
      main_menu.AppendMenuW(MF_BITMAP, id++, std::wstring_view((wchar_t*)menu_sizer.get(), sizeof(void*)));

      tab_context_menu.AppendMenuW(MF_OWNERDRAW, 1, L"Close");

      AddDefaultTab();

      SendMessageW(*this, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet");
      return 0;
    }

    std::optional<LRESULT> wm_destroy()
    {
      PostQuitMessage(0);
      return 0;
    }

    void on_size(SIZE total_size)
    {
      auto new_size = total_size;

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

      SendMessage(tab_control, WM_SETREDRAW, FALSE, 0);

      tab_control.SetWindowPos(POINT{ .x = tree_size.cx + divider.cx });
      tab_control.SetWindowPos(right_size);

      SendMessage(tab_control, WM_SETREDRAW, TRUE, 0);

      auto count = tab_control.GetItemCount();

      if (!count)
      {
        count = 1;
      }

      auto fallback = 150;
      auto total_width = right_size.cx;

      auto child = win32::window_ref(::FindWindowExW(tab_control, nullptr, UPDOWN_CLASSW, nullptr));
      auto child_width = total_width / 12;
      auto child_height = 100;

      if (child)
      {
        auto tab_size = tab_control.GetClientSize();
        auto child_size = child.GetClientSize();
        child_height = child_size->cy;

        fallback = child_width * 4;
        total_width = total_width - child_width;
      }

      auto width = std::clamp<int>(total_width / count, fallback, right_size.cx);

      TabCtrl_SetPadding(tab_control, 10, 0);
      auto old_height = HIWORD(TabCtrl_SetItemSize(tab_control, width, 40));
      TabCtrl_SetItemSize(tab_control, width, old_height);
      auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value); }).value().second;

      SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&tab_rect));

      for (auto i = 0; i < tab_control.GetItemCount(); ++i)
      {
        auto tab_item = tab_control.GetItem(i);

        if (tab_item->lParam)
        {
          assert(win32::window_ref(win32::hwnd_t(tab_item->lParam)).SetWindowPos(tab_rect));
        }
      }

      if (child)
      {
        child.SetWindowPos(POINT{ .x = right_size.cx - child_width - (child_width / 4), .y = 0 });
        child.SetWindowPos(SIZE{ .cx = child_width, .cy = child_height });
      }
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MAXIMIZED || type == SIZE_RESTORED)
      {
        on_size(client_size);
      }

      return 0;
    }

    BOOL wm_copy_data(win32::copy_data_message<std::byte> message)
    {
      if (fs::current_path() != initial_working_dir)
      {
        initial_working_dir = fs::current_path();
        repopulate_tree_view(initial_working_dir);
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

        auto ref = child->ref();
        win32::apply_window_theme(ref);

        auto file_path = win32::get_path_from_handle((HANDLE)message.data_type);

        if (file_path && child->CopyData(*this, COPYDATASTRUCT{ .dwData = (ULONG_PTR)message.data_type, .cbData = DWORD(message.data.size()), .lpData = message.data.data() }))
        {
          auto index = tab_control.GetItemCount();

          auto filename = fs::path(*file_path).filename().wstring();

          tab_control.InsertItem(index, TCITEMW{ .mask = TCIF_TEXT | TCIF_PARAM, .pszText = filename.data(), .lParam = win32::lparam_t(child->get()) });

          SetWindowLongPtrW(*child, GWLP_ID, index + 1);

          tab_control_tcn_sel_changing(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGING });

          tab_control.SetCurrentSelection(index);

          tab_control_tcn_sel_change(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGE });
          on_size(*this->GetClientSize());
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
          // Software\Microsoft\Windows\CurrentVersion\Themes\History\Colors
          DWORD value = 0;
          DWORD size = sizeof(DWORD);

          DWORD type = REG_DWORD;
          if (RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS)
          {
            is_dark_mode = value == 0;

            win32::set_is_dark_theme(is_dark_mode);

            COLORREF bk_color = RGB(0x20, 0x20, 0x20);
            COLORREF text_color = 0x00FFFFFF;
            COLORREF text_bk_color = RGB(0x2b, 0x2b, 0x2b);
            COLORREF text_highlight_color = RGB(0x2d, 0x2d, 0x2d);
            COLORREF btn_shadow_color = 0x00AAAAAA;

            COLORREF button_bk_color = RGB(0x2d, 0x2d, 0x2d);
            COLORREF pushed_bk_color = RGB(0x27, 0x27, 0x27);
            COLORREF focus_bk_color = RGB(0x32, 0x32, 0x32);
            COLORREF hot_bk_color = RGB(0x32, 0x32, 0x32);

            if (!is_dark_mode)
            {
              bk_color = RGB(0xfb, 0xfb, 0xfb);
              text_color = RGB(0x1a, 0x1a, 0x1a);
              text_bk_color = RGB(0xf3, 0xf3, 0xf3);
              text_highlight_color = RGB(0xea, 0xea, 0xea);
              btn_shadow_color = RGB(1, 1, 1);

              button_bk_color = RGB(0xfb, 0xfb, 0xfb);
              pushed_bk_color = RGB(0xf5, 0xf5, 0xf5);
              focus_bk_color = RGB(0xfb, 0xf6, 0xf6);
              hot_bk_color = RGB(0xfb, 0xf6, 0xf6);
            }

            using namespace win32;

            set_color_for_class(button::class_name, properties::button::bk_color, button_bk_color);
            set_color_for_class(button::class_name, properties::button::hot_bk_color, hot_bk_color);
            set_color_for_class(button::class_name, properties::button::focus_bk_color, focus_bk_color);
            set_color_for_class(button::class_name, properties::button::pushed_bk_color, pushed_bk_color);
            set_color_for_class(button::class_name, properties::button::bk_color, bk_color);
            set_color_for_class(button::class_name, properties::button::text_color, text_color);
            set_color_for_class(button::class_name, properties::button::line_color, text_color);

            set_color_for_class(tree_view::class_name, properties::tree_view::bk_color, bk_color);
            set_color_for_class(tree_view::class_name, properties::tree_view::text_color, text_color);
            set_color_for_class(tree_view::class_name, properties::tree_view::line_color, 0x00383838);

            set_color_for_class(list_view::class_name, properties::list_view::bk_color, bk_color);
            set_color_for_class(list_view::class_name, properties::list_view::text_color, text_color);
            set_color_for_class(list_view::class_name, properties::list_view::text_bk_color, text_bk_color);
            set_color_for_class(list_view::class_name, properties::list_view::outline_color, 0x00AAAAAA);

            set_color_for_class(list_box::class_name, properties::list_box::bk_color, bk_color);
            set_color_for_class(list_box::class_name, properties::list_box::text_color, text_color);
            set_color_for_class(list_box::class_name, properties::list_box::text_bk_color, text_bk_color);
            set_color_for_class(list_box::class_name, properties::list_box::text_highlight_color, text_highlight_color);

            set_color_for_class(track_bar::class_name, properties::track_bar::bk_color, bk_color);
            set_color_for_class(track_bar::class_name, properties::track_bar::text_color, text_color);
            set_color_for_class(track_bar::class_name, properties::track_bar::text_bk_color, text_bk_color);

            set_color_for_class(header::class_name, properties::header::bk_color, bk_color);
            set_color_for_class(header::class_name, properties::header::text_color, text_color);
            set_color_for_class(header::class_name, properties::header::text_bk_color, text_bk_color);
            set_color_for_class(header::class_name, properties::header::text_highlight_color, text_highlight_color);

            set_color_for_class(tab_control::class_name, properties::tab_control::bk_color, bk_color);
            set_color_for_class(tab_control::class_name, properties::tab_control::text_color, text_color);
            set_color_for_class(tab_control::class_name, properties::tab_control::text_bk_color, text_bk_color);
            set_color_for_class(tab_control::class_name, properties::tab_control::text_highlight_color, text_highlight_color);

            set_color_for_class(tool_bar::class_name, properties::tool_bar::bk_color, bk_color);
            set_color_for_class(tool_bar::class_name, properties::tool_bar::text_color, text_color);
            set_color_for_class(tool_bar::class_name, properties::tool_bar::btn_face_color, text_bk_color);
            set_color_for_class(tool_bar::class_name, properties::tool_bar::text_highlight_color, text_highlight_color);
            set_color_for_class(tool_bar::class_name, properties::tool_bar::btn_shadow_color, btn_shadow_color);

            set_color_for_class(L"", properties::window::bk_color, bk_color);
            set_color_for_class(L"#32768", properties::menu::bk_color, bk_color);
            set_color_for_class(L"#32768", properties::menu::text_color, text_color);
            set_color_for_class(L"#32768", properties::menu::text_highlight_color, text_highlight_color);
            set_color_for_class(static_control::class_name, properties::static_control::bk_color, bk_color);
            set_color_for_class(static_control::class_name, properties::static_control::text_color, text_color);

            for (auto& pixel : menu_sizer.get_pixels())
            {
              pixel.rgbRed = GetRValue(bk_color);
              pixel.rgbBlue = GetBValue(bk_color);
              pixel.rgbGreen = GetGValue(bk_color);
            }

            win32::apply_window_theme(*this);

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

    void remove_tab(int index)
    {
      auto item = tab_control.GetItem(index);

      auto current_index = TabCtrl_GetCurSel(tab_control);

      assert(::DestroyWindow(win32::hwnd_t(item->lParam)) == TRUE);
      TabCtrl_DeleteItem(tab_control, index);

      if (index == current_index)
      {
        auto count = TabCtrl_GetItemCount(tab_control);

        if (count > 0)
        {
          index = std::clamp<int>(index, 0, count - 1);

          TabCtrl_SetCurSel(tab_control, index);
          tab_control_tcn_sel_change(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGE });
        }
      }
    }

    void tab_control_nm_rclick(win32::tab_control sender, const NMHDR& notification)
    {
      POINT mouse_pos;
      if (GetCursorPos(&mouse_pos) && ScreenToClient(sender, &mouse_pos))
      {
        auto window_rect = sender.GetWindowRect();
        TCHITTESTINFO hit_test{
          .pt = mouse_pos,
          .flags = TCHT_ONITEMLABEL
        };

        if (auto index = TabCtrl_HitTest(sender, &hit_test); index != -1)
        {
          auto tab_rect = sender.GetItemRect(index);
          auto height = tab_rect->bottom - tab_rect->top;

          auto action = ::TrackPopupMenu(GetMenu(sender), TPM_CENTERALIGN | TPM_RETURNCMD, window_rect->left + tab_rect->left, window_rect->top + height, 0, *this, nullptr);

          if (action == 1)
          {
            remove_tab(index);
          }
        }
      }
    }

    BOOL tab_control_tcn_sel_changing(win32::tab_control sender, const NMHDR& notification)
    {
      auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);

      auto tab_item = tab_control.GetItem(current_index);
      ::ShowWindow(win32::hwnd_t(tab_item->lParam), SW_HIDE);
      return 0;
    }

    void tab_control_tcn_sel_change(win32::tab_control sender, const NMHDR& notification)
    {
      auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);

      if (current_index == -1)
      {
        return;
      }

      auto tab_item = tab_control.GetItem(current_index);


      if (tab_item->lParam == 0)
      {
        return;
      }

      auto temp_window = win32::window_ref(win32::hwnd_t(tab_item->lParam));

      temp_window.SetWindowPos(HWND_TOP);

      auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value); }).value().second;

      ShowWindow(win32::hwnd_t(tab_item->lParam), SW_SHOW);
    }

    void AddDefaultTab()
    {
      auto class_name = win32::type_name<default_view>();

      auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value); }).value().second;

      SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&tab_rect));

      auto child = win32::CreateWindowExW(::CREATESTRUCTW{
        .hwndParent = *this,
        .cy = tab_rect.bottom - tab_rect.top,
        .cx = tab_rect.right - tab_rect.left,
        .y = tab_rect.top,
        .x = tab_rect.left,
        .style = WS_CHILD | WS_CLIPCHILDREN,
        .lpszClass = class_name.c_str(),
      });
      child->SetPropW(L"TabIndestructible", TRUE);

      auto ref = child->ref();
      win32::apply_window_theme(ref);
      assert(child);

      auto index = tab_control.GetItemCount();

      std::wstring title = L"Supported Games";

      tab_control.InsertItem(index, TCITEMW{
                                      .mask = TCIF_TEXT | TCIF_PARAM,
                                      .pszText = title.data(),
                                      .lParam = win32::lparam_t(child->get()),
                                    });

      if (index == 0)
      {
        on_size(*this->GetClientSize());
      }

      tab_control_tcn_sel_changing(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGING });

      tab_control.SetCurrentSelection(index);

      tab_control_tcn_sel_change(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGE });

      on_size(*this->GetClientSize());
    }

    bool AddTabFromPath(std::filesystem::path file_path)
    {
      try
      {
        auto path_ref = file_path.c_str();
        win32::file file_to_read(file_path, GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

        auto mapping = file_to_read.CreateFileMapping(std::nullopt, PAGE_READONLY, {}, L"");

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
            .style = WS_CHILD | WS_CLIPCHILDREN,
            .lpszName = file_path.c_str(),
            .lpszClass = class_name.c_str(),
          });


          assert(child);

          auto ref = child->ref();
          win32::apply_window_theme(ref);

          COPYDATASTRUCT data{
            .dwData = (ULONG_PTR)mapping->get(),
            .cbData = DWORD(size),
            .lpData = view.get()
          };

          if (child->CopyData(*this, data))
          {
            auto index = tab_control.GetItemCount();

            tab_control.InsertItem(index, TCITEMW{
                                            .mask = TCIF_TEXT | TCIF_PARAM,
                                            .pszText = (wchar_t*)file_path.filename().c_str(),
                                            .lParam = win32::lparam_t(child->get()),
                                          });

            if (index == 0)
            {
              on_size(*this->GetClientSize());
            }

            tab_control_tcn_sel_changing(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGING });

            tab_control.SetCurrentSelection(index);

            tab_control_tcn_sel_change(win32::tab_control(tab_control.get()), NMHDR{ .hwndFrom = tab_control, .code = TCN_SELCHANGE });

            on_size(*this->GetClientSize());
          }
          else
          {
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

    BOOL dir_list_nm_dbl_click(win32::tree_view sender, const NMHDR& notification)
    {
      selected_file != files.end() && AddTabFromPath(*selected_file);
      return FALSE;
    }

    fs::path get_path_from_item(win32::tree_view& sender, HTREEITEM item)
    {
      fs::path result;

      SHSTOCKICONINFO icon{ .cbSize = sizeof(SHSTOCKICONINFO) };

      if (SHGetStockIconInfo(SIID_FOLDER, SHGSI_SYSICONINDEX, &icon) == S_OK)
      {
        std::array<wchar_t, 255> text;
        TVITEMW info{ .mask = TVIF_TEXT | TVIF_IMAGE, .hItem = item, .pszText = text.data(), .cchTextMax = text.size() };

        if (TreeView_GetItem(sender, &info) && icon.iSysImageIndex == info.iImage)
        {
          result = fs::path(text.data()) / result;
        }

        do {
          auto parent = TreeView_GetParent(sender, item);

          if (parent == nullptr)
          {
            break;
          }
          item = parent;
          info.hItem = item;

          if (TreeView_GetItem(sender, &info) && icon.iSysImageIndex == info.iImage)
          {
            result = fs::path(text.data()) / result;
          }


        } while (item != nullptr);
      }


      return result;
    }

    BOOL dir_list_nm_rclick(win32::tree_view sender, const NMHDR& notification)
    {
      POINT point{};

      if (::GetCursorPos(&point) && ::ScreenToClient(sender, &point))
      {
        TVHITTESTINFO info{
          .pt = point,
          .flags = TVHT_ONITEM
        };
        auto item = TreeView_HitTest(sender, &info);

        if (item && ::ClientToScreen(sender, &point))
        {
          auto action = ::TrackPopupMenu(GetMenu(sender), TPM_CENTERALIGN | TPM_RETURNCMD, point.x, point.y, 0, *this, nullptr);

          if (action == 1)
          {
            auto current_path = get_path_from_item(sender, item);
            ::ShellExecuteW(NULL, L"open", current_path.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
          }
        }
      }

      return FALSE;
    }

    void dir_list_tvn_sel_changed(win32::tree_view, const NMTREEVIEWW& notification)
    {
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
    }

    BOOL dir_list_tvn_item_expanding(win32::tree_view, const NMTREEVIEWW& notification)
    {
      return FALSE;
    }

    std::optional<LRESULT> wm_command(win32::menu_ref, int identifier) override
    {
      if (identifier == edit_theme_id)
      {
        theme_window = *win32::window_module_ref::current_module().CreateWindowExW(CREATESTRUCTW{
          .lpCreateParams = (HWND)this->get(),
          .hwndParent = *this,
          .cx = CW_USEDEFAULT,
          .x = CW_USEDEFAULT,
          .style = WS_OVERLAPPEDWINDOW,
          .lpszName = L"Preferences",
          .lpszClass = win32::type_name<preferences_view>().c_str() });

        auto ref = win32::window_ref(theme_window);
        win32::apply_window_theme(ref);
        ShowWindow(theme_window, SW_NORMAL);
      }

      if (identifier == open_workspace_id)
      {
        auto prop = win32::get_color_for_window(ref(), win32::properties::button::bk_color);
        auto dialog = win32::com::CreateFileOpenDialog();

        if (dialog)
        {
          auto open_dialog = *dialog;
          open_dialog->SetOptions(FOS_PICKFOLDERS);

          auto current_path = std::filesystem::current_path();

          ITEMIDLIST* path_id;
          if (SHParseDisplayName(current_path.c_str(), nullptr, &path_id, SFGAO_FOLDER, nullptr) == S_OK)
          {
            win32::com::com_ptr<IShellItem> shell_item;

            if (SHCreateItemFromIDList(path_id, IID_IShellItem, shell_item.put_void()) == S_OK)
            {
              open_dialog->SetFolder(shell_item.get());
            }

            ::CoTaskMemFree(path_id);
          }


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


          win32::com::com_ptr<IShellItem> folder_info;

          int file_count = 0;
          std::map<std::wstring, int> extension_counts;
          if (dialog.value()->GetFolder(folder_info.put()) == S_OK)
          {
            win32::com::com_string temp;
            if (folder_info->GetDisplayName(SIGDN_FILESYSPATH, temp.put()) == S_OK)
            {
              for (auto entry = fs::recursive_directory_iterator(std::wstring_view(temp));
                entry != fs::recursive_directory_iterator();
                ++entry)
              {
                if (file_count > 3000)
                {
                  break;
                }

                if (entry.depth() == 3)
                {
                  entry.disable_recursion_pending();
                }

                if (entry->is_regular_file())
                {
                  auto known_extension = extensions.find(entry->path().extension());

                  if (known_extension != extensions.end())
                  {
                    auto extension = entry->path().extension().wstring();

                    if (!extension_counts.contains(extension))
                    {
                      // helps place files in deeper folders lower down in the list
                      extension_counts[extension] = 1000 - entry.depth() * 100;
                    }
                    extension_counts[extension]++;
                  }
                }
                file_count++;
              }
            }
          }

          std::sort(filetypes.begin(), filetypes.end(), [&](const auto& a, const auto& b) {
            return extension_counts[a.pszName] > extension_counts[b.pszName];
          });

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

      if (identifier == about_id)
      {
        win32::DialogBoxIndirectParamW<about_view>(::GetModuleHandleW(nullptr),
          win32::default_dialog{ { .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 300, .cy = 400 } },
          ref());
      }

      return std::nullopt;
    }
  };
}// namespace siege::views

#endif