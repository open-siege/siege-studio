#ifndef SIEGE_MAIN_WINDOW_HPP
#define SIEGE_MAIN_WINDOW_HPP

#include <dwmapi.h>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/file.hpp>
#include <siege/platform/presentation_module.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/layout.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/win/basic_window.hpp>
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
  struct siege_main_window final : win32::basic_window<siege_main_window>
  {
    win32::tree_view dir_list;
    win32::popup_menu dir_list_menu;
    win32::button separator;
    win32::tab_control tab_control;
    win32::window theme_window;

    std::list<platform::presentation_module> loaded_modules;
    std::map<std::wstring, std::int32_t> extensions;
    std::set<std::u16string> categories;

    std::list<fs::path> folders;
    std::list<fs::path> files;
    std::list<fs::path>::iterator selected_file;
    std::map<win32::lparam_t, fs::path> tab_working_directory_mapping;
    std::map<win32::lparam_t, fs::path>::iterator default_tab_mapping = tab_working_directory_mapping.end();

    bool updated_in_progress = false;
    bool is_resizing = false;
    win32::gdi::cursor_ref resize_cursor;
    HCURSOR previous_cursor = nullptr;
    SIZE tree_size{};

    std::wstring buffer;

    win32::menu main_menu;
    std::array<win32::popup_menu, 3> popup_menus;
    win32::popup_menu tab_context_menu;

    win32::image_list shell_images;

    inline static bool destroy_dialog = false;

    bool is_dark_mode = false;

    win32::gdi::bitmap menu_sizer = [] {
      win32::gdi::bitmap result(SIZE{ 1, ::GetSystemMetrics(SM_CYSIZE) * 2 }, win32::gdi::bitmap::skip_shared_handle);
      return result;
    }();

    siege_main_window(win32::hwnd_t self, CREATESTRUCTW& params) : basic_window(self, params), tab_control(nullptr)
    {
      params.dwExStyle = params.dwExStyle | WS_EX_ACCEPTFILES;

      ::SetWindowLongPtrW(self, GWL_EXSTYLE, params.dwExStyle);

      fs::path module_path = fs::path(win32::module_ref::current_module().GetModuleFileName()).parent_path();
      loaded_modules = platform::presentation_module::load_modules(module_path);

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

    void repopulate_tree_view(fs::path path)
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

      for (auto const& dir_entry : fs::directory_iterator{ current_path })
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
        win32::remove_window_subclass(hWnd, button_sub_class, uIdSubclass);
      }

      return win32::def_subclass_proc(hWnd, msg, wParam, lParam);
    }

    auto wm_create()
    {
      dir_list = *win32::CreateWindowExW<win32::tree_view>(CREATESTRUCTW{ .hMenu = dir_list_menu.get(), .hwndParent = *this, .style = WS_CHILD | WS_VISIBLE });
      dir_list.bind_tvn_sel_changed(std::bind_front(&siege_main_window::dir_list_tvn_sel_changed, this));
      dir_list.bind_tvn_item_expanding(std::bind_front(&siege_main_window::dir_list_tvn_item_expanding, this));
      dir_list.bind_nm_dbl_click(std::bind_front(&siege_main_window::dir_list_nm_dbl_click, this));
      dir_list.bind_nm_rclick(std::bind_front(&siege_main_window::dir_list_nm_rclick, this));
      dir_list_menu.AppendMenuW(MF_OWNERDRAW, 1, L"Open in File Explorer");

      separator = *win32::CreateWindowEx<win32::button>(CREATESTRUCTW{ .hwndParent = *this, .style = WS_CHILD | WS_VISIBLE | BS_FLAT });

      if (win32::set_window_subclass(separator, button_sub_class, (UINT_PTR)this, (DWORD_PTR)this))
      {
        TRACKMOUSEEVENT event{
          .cbSize = sizeof(TRACKMOUSEEVENT),
          .dwFlags = TME_LEAVE,
          .hwndTrack = separator,
          .dwHoverTime = HOVER_DEFAULT
        };
        assert(::TrackMouseEvent(&event) == TRUE);
      }

      repopulate_tree_view(fs::current_path());
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

      popup_menus[0].AppendMenuW(MF_OWNERDRAW, id, L"Open...");
      win32::set_window_command_subclass(*this, id++, [this](auto, auto, auto) -> std::optional<LRESULT> {
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
        return std::nullopt;
      });

      popup_menus[0].AppendMenuW(MF_OWNERDRAW, id, L"Open Folder as Workspace");
      win32::set_window_command_subclass(*this, id++, [this](auto, auto, auto) -> std::optional<LRESULT> {
        auto prop = win32::get_color_for_window(ref(), win32::properties::button::bk_color);
        auto dialog = win32::com::CreateFileOpenDialog();

        if (default_tab_mapping == tab_working_directory_mapping.end())
        {
          return std::nullopt;
        }

        auto new_path = win32::get_path_via_file_dialog({
          .lpstrInitialDir = default_tab_mapping->second.c_str(),
          .Flags = FOS_PICKFOLDERS,
        });

        if (new_path)
        {
          default_tab_mapping->second = *new_path;

          if (tab_control.GetCurrentSelection() == 0)
          {
            repopulate_tree_view(std::move(*new_path));
          }

          return 0;
        }

        return std::nullopt;
      });


      auto unpack_path = win32::find_binary_module(win32::search_context{
        .module_name = L"game-unpack.exe" });

      if (unpack_path)
      {
        popup_menus[0].AppendMenuW(MF_OWNERDRAW, id, L"Unpack Game Backup...");
        win32::set_window_command_subclass(*this, id++, [this, unpack_path = *unpack_path](auto, auto, auto) -> std::optional<LRESULT> {
          ::ShellExecuteW(NULL, L"open", unpack_path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
          return std::nullopt;
        });
      }

      popup_menus[0].AppendMenuW(MF_SEPARATOR | MF_OWNERDRAW, id++);

      popup_menus[0].AppendMenuW(MF_OWNERDRAW, id, L"Quit");
      win32::set_window_command_subclass(*this, id++, [this](auto, auto, auto) -> std::optional<LRESULT> {
        ::DestroyWindow(*this);
        return std::nullopt;
      });

      popup_menus[1].AppendMenuW(MF_OWNERDRAW, id, L"Preferences");
      win32::set_window_command_subclass(*this, id++, [this](auto, auto, auto) -> std::optional<LRESULT> {
        auto size = this->GetClientSize();
        auto pos = this->GetWindowRect();
        theme_window = *win32::window_module_ref(win32::module_ref::current_application()).CreateWindowExW(CREATESTRUCTW{
          .hwndParent = *this,
          .cy = size->cy,
          .cx = size->cx,
          .y = pos->top,
          .x = pos->left,
          .style = (LONG)(WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),
          .lpszName = L"Preferences",
          .lpszClass = win32::type_name<preferences_view>().c_str() });

        auto ref = win32::window_ref(theme_window);
        win32::apply_window_theme(ref);
        ShowWindow(theme_window, SW_NORMAL);
        return std::nullopt;
      });

      popup_menus[2].AppendMenuW(MF_OWNERDRAW, id, L"About");
      win32::set_window_command_subclass(*this, id++, [this](auto, auto, auto) -> std::optional<LRESULT> {
        win32::DialogBoxIndirectParamW<about_view>(::GetModuleHandleW(nullptr),
          win32::default_dialog{ { .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 350, .cy = 400 } },
          ref());
        return std::nullopt;
      });
      auto can_update = win32::module_ref::current_application().GetProcAddress<BOOL (*)()>("can_update");

      if (can_update && can_update())
      {
        static auto update_stable_id = (std::uint16_t)RegisterWindowMessageW(L"COMMAND_UPDATE_STABLE");
        static auto update_development_id = (std::uint16_t)RegisterWindowMessageW(L"COMMAND_UPDATE_DEVELOPMENT");

        popup_menus[2].AppendMenuW(MF_OWNERDRAW, update_stable_id, L"Update (Stable)");
        win32::set_window_command_subclass(*this, update_stable_id, [this](auto, auto, auto) -> std::optional<LRESULT> {
          trigger_update(update_stable_id);
          return std::nullopt;
        });
        popup_menus[2].AppendMenuW(MF_OWNERDRAW, update_development_id, L"Update (Development)");
        win32::set_window_command_subclass(*this, update_development_id, [this](auto, auto, auto) -> std::optional<LRESULT> {
          trigger_update(update_development_id);
          return std::nullopt;
        });
      }

      SetMenu(*this, main_menu.get());

      main_menu.AppendMenuW(MF_POPUP | MF_OWNERDRAW, (UINT_PTR)popup_menus[0].get(), L"File");
      main_menu.AppendMenuW(MF_POPUP | MF_OWNERDRAW, (UINT_PTR)popup_menus[1].get(), L"Edit");
      main_menu.AppendMenuW(MF_POPUP | MF_OWNERDRAW, (UINT_PTR)popup_menus[2].get(), L"Help");
      main_menu.AppendMenuW(MF_BITMAP, id++, std::wstring_view((wchar_t*)menu_sizer.get(), sizeof(void*)));

      tab_context_menu.AppendMenuW(MF_OWNERDRAW, 1, L"Close");

      AddDefaultTab();

      HKEY user_key = nullptr;
      HKEY main_key = nullptr;
      auto access = KEY_QUERY_VALUE | KEY_READ;

      if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, L"Software\\The Siege Hub\\Siege Studio", 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
      {
        DWORD preference = 0;
        DWORD size = sizeof(DWORD);
        DWORD type = REG_DWORD;

        if (::RegGetValueW(main_key, nullptr, L"UserThemePreference", RRF_RT_DWORD, &type, (BYTE*)&preference, &size) == ERROR_SUCCESS && (preference == 1 || preference == 2))
        {
          this->SetPropW(L"UserThemePreference", preference);
        }

        ::RegCloseKey(main_key);
        ::RegCloseKey(user_key);
      }

      SendMessageW(*this, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet");
      return 0;
    }

    std::optional<LRESULT> wm_destroy()
    {
      destroy_dialog = true;
      if (!updated_in_progress)
      {
        PostQuitMessage(0);
      }

      return 0;
    }

    void trigger_update(std::uint32_t update_type)
    {
      if (updated_in_progress)
      {
        return;
      }

      updated_in_progress = true;

      win32::queue_user_work_item([update_type, this] {
        auto has_update = win32::module_ref::current_application().GetProcAddress<BOOL (*)(std::uint32_t update_type)>("has_update");
        auto get_update_version = win32::module_ref::current_application().GetProcAddress<SIZE (*)(std::uint32_t update_type)>("get_update_version");
        auto is_updating = win32::module_ref::current_application().GetProcAddress<BOOL (*)()>("is_updating");
        auto detect_update = win32::module_ref::current_application().GetProcAddress<void (*)(std::uint32_t)>("detect_update");
        auto apply_update = win32::module_ref::current_application().GetProcAddress<void (*)(std::uint32_t, HWND)>("apply_update");

        static auto get_max_update_size = win32::module_ref::current_application().GetProcAddress<std::size_t (*)(std::uint32_t update_type)>("get_max_update_size");
        static auto get_current_update_size = win32::module_ref::current_application().GetProcAddress<std::size_t (*)(std::uint32_t update_type)>("get_current_update_size");

        if (has_update && get_update_version && is_updating && detect_update && apply_update)
        {
          detect_update(update_type);

          for (auto i = 0; i < 60; ++i)
          {
            if (!has_update(update_type))
            {
              ::Sleep(200);
            }
          }

          if (!has_update(update_type))
          {
            updated_in_progress = false;
            ::MessageBoxW(nullptr, L"Could not find an update for the selected channel", L"Could Not Update", MB_TASKMODAL | MB_ICONWARNING);
            return;
          }

          auto selection = ::MessageBoxW(nullptr, L"An update was detected, would you like to update?", L"Update Available", MB_TASKMODAL | MB_ICONINFORMATION | MB_YESNO);

          if (selection != IDYES)
          {
            updated_in_progress = false;
          }

          if (selection == IDYES)
          {
            struct handler
            {
              static HRESULT CALLBACK DialogCallback(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR update_type)
              {
                if (destroy_dialog && msg != TDN_DESTROYED)
                {
                  ::EndDialog(dialog, 0);
                  return S_OK;
                }

                if (msg == TDN_CREATED)
                {
                  win32::window_ref ref(dialog);
                  win32::apply_window_theme(ref);
                  ::SendMessageW(dialog, TDM_SET_MARQUEE_PROGRESS_BAR, get_max_update_size((std::uint32_t)update_type) == 0 ? TRUE : FALSE, 0);
                  ::SendMessageW(dialog, TDM_SET_PROGRESS_BAR_MARQUEE, get_max_update_size((std::uint32_t)update_type) == 0 ? TRUE : FALSE, 0);
                  ::SendMessageW(dialog, TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(0, 100));
                }

                if (msg == TDN_TIMER && (wParam > 600 && wParam <= 800))
                {
                  ::SendMessageW(dialog, TDM_SET_PROGRESS_BAR_MARQUEE, TRUE, 0);
                  ::SendMessageW(dialog, TDM_SET_MARQUEE_PROGRESS_BAR, TRUE, 0);
                  return S_OK;
                }

                if (msg == TDN_TIMER && get_max_update_size((std::uint32_t)update_type) > 0 && get_current_update_size)
                {
                  auto max = (float)get_max_update_size((std::uint32_t)update_type);
                  auto curr = (float)get_current_update_size((std::uint32_t)update_type);
                  auto result = (curr / max) * 100;

                  ::SendMessageW(dialog, TDM_SET_PROGRESS_BAR_POS, (WPARAM)result, 0);

                  if (result >= 100)
                  {
                    return S_OK;
                  }
                }
                else if (msg == TDN_TIMER)
                {
                  return S_OK;
                }
                return S_FALSE;
              }
            };
            destroy_dialog = false;
            apply_update(update_type, *this);

            TASKDIALOGCONFIG config{
              .cbSize = sizeof(config),
              .dwFlags = get_max_update_size && get_current_update_size ? TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER : TDF_SHOW_PROGRESS_BAR,
              .pszWindowTitle = L"Downloading and Applying Update",
              .pfCallback = handler::DialogCallback,
              .lpCallbackData = (LONG_PTR)update_type,

            };
            ::TaskDialogIndirect(&config, nullptr, nullptr, nullptr);
          }
        }
      });
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

      auto width = std::clamp<int>(total_width / count, fallback, right_size.cx > 25 ? right_size.cx - 25 : 0);// TODO figure out the correct sizing for the tab control

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

          tab_working_directory_mapping[win32::lparam_t(child->get())] = get_working_directory_for_path(*file_path);

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
          auto user_preference = this->GetPropW<int>(L"UserThemePreference");

          if (user_preference == 0 && RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS)
          {
            is_dark_mode = value == 0;
          }
          else if (user_preference != 0)
          {
            is_dark_mode = user_preference == 2;
          }

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
          set_color_for_class(L"#32770", properties::window::bk_color, bk_color);
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

    fs::path get_working_directory_for_path(const fs::path& file_path)
    {
      std::error_code last_error;
      if (fs::is_directory(file_path, last_error))
      {
        return file_path;
      }

      auto relative = fs::relative(file_path, last_error);

      if (relative != fs::path{} && (relative.begin() != relative.end() && *relative.begin() != ".."))
      {
        return fs::current_path();
      }


      // FFS! Comparing extensions is broken after a Visual Studio update.
      // if (file_path.has_extension() && (file_path.extension() == L".exe" || file_path.extension() == L".EXE"))
      // So I have to do this instead:

      if (std::wstring_view(file_path.c_str()).ends_with(L".exe"))
      {
        return file_path.parent_path();
      }

      if (std::wstring_view(file_path.c_str()).ends_with(L".EXE"))
      {
        return file_path.parent_path();
      }

      auto parent_path = file_path.parent_path();

      if (std::wstring(parent_path.c_str()).empty())
      {
        return fs::current_path();
      }

      constexpr static auto common_exe_paths = std::array<const char*, 3>{ { "bin",
        "bin32",
        "system" } };

      do
      {
        for (auto& entry : fs::directory_iterator{ parent_path })
        {
          if (!entry.is_directory() && entry.path().has_extension() && (entry.path().extension() == ".exe" || entry.path().extension() == ".EXE"))
          {
            return parent_path;
          }

          for (auto* common_path : common_exe_paths)
          {
            if (!fs::is_directory(parent_path / common_path, last_error))
            {
              continue;
            }

            for (auto& entry : fs::directory_iterator{ parent_path / common_path })
            {
              if (!entry.is_directory() && entry.path().has_extension() && (entry.path().extension() == ".exe" || entry.path().extension() == ".EXE"))
              {
                return parent_path;
              }
            }
          }
        }

        if (!parent_path.has_parent_path())
        {
          break;
        }

        parent_path = parent_path.parent_path();
      } while (parent_path != file_path.root_path());

      return file_path.parent_path();
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

      auto alt_path_iter = tab_working_directory_mapping.find(tab_item->lParam);

      if (alt_path_iter != tab_working_directory_mapping.end() && fs::current_path() != alt_path_iter->second)
      {
        fs::current_path(alt_path_iter->second);
        repopulate_tree_view(fs::current_path());
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

      default_tab_mapping = tab_working_directory_mapping.emplace(win32::lparam_t(child->get()), fs::current_path()).first;

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

    bool AddTabFromPath(fs::path file_path)
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

            tab_working_directory_mapping[win32::lparam_t(child->get())] = get_working_directory_for_path(file_path);

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

    LRESULT wm_drop_files(HDROP files)
    {
      auto count = ::DragQueryFileW(files, UINT_MAX, nullptr, 0);
      std::wstring buffer;
      buffer.reserve(1024);

      for (auto i = 0u; i < count; ++i)
      {
        buffer.resize(::DragQueryFileW(files, i, nullptr, 0) + 1);
        if (::DragQueryFileW(files, i, buffer.data(), buffer.size()))
        {
          AddTabFromPath(buffer);
        }
      }

      ::DragFinish(files);

      return 0;
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
        for (auto const& dir_entry : fs::directory_iterator{ *folder })
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

    inline static auto register_class(HINSTANCE module)
    {
      WNDCLASSEXW info{
        .cbSize = sizeof(info),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = basic_window::window_proc,
        .cbWndExtra = sizeof(void*),
        .hInstance = module,
        .hIcon = (HICON)::LoadImageW(module, L"AppIcon", IMAGE_ICON, 0, 0, 0),
        .hCursor = LoadCursorW(module, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszClassName = win32::type_name<siege::views::siege_main_window>().c_str(),
      };
      return ::RegisterClassExW(&info);
    }

    std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
    {
      static auto unpack_done_id = ::RegisterWindowMessageW(L"SIEGE_UNPACK_DONE");

      if (message == unpack_done_id)
      {
        struct handler
        {
          static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lparam)
          {
            ::SendMessageW(hwnd, (UINT)lparam, 0, 0);
            return TRUE;
          }
        };
        ::EnumChildWindows(*this, handler::EnumChildProc, unpack_done_id);
        return std::nullopt;
      }

      switch (message)
      {
      case WM_CREATE:
        return wm_create();
      case WM_DESTROY:
        return wm_destroy();
      case WM_SETTINGCHANGE:
        return wm_setting_change(win32::setting_change_message(wparam, lparam));
      case WM_SIZE:
        return (LRESULT)wm_size((std::size_t)wparam, SIZE(LOWORD(lparam), HIWORD(lparam)));
      case WM_COPYDATA:
        return (LRESULT)wm_copy_data(win32::copy_data_message<std::byte>(wparam, lparam));
      case WM_DROPFILES:
        return wm_drop_files((HDROP)wparam);
      default:
        return std::nullopt;
      }
    }
  };
}// namespace siege::views

#endif