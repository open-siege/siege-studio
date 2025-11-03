#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/menu.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/presentation_module.hpp>
#include <siege/platform/resource.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/win/basic_window.hpp>
#include <spanstream>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <versionhelpers.h>
#include <siege/platform/win/theming.hpp>
#include "vol_shared.hpp"

namespace siege::views
{
  namespace fs = std::filesystem;
  std::optional<fs::path> create_self_extracting_resource(std::any& self,
    fs::path unvol_exe_path,
    std::optional<fs::path> output_path,
    std::optional<std::vector<std::string>> post_extract_commands);

  struct vol_view final : win32::basic_window<vol_view>
  {
    std::any shared_state;

    win32::tool_bar table_settings;
    win32::list_view table;

    std::list<platform::presentation_module> modules;

    std::set<std::u16string> all_categories;
    std::map<std::u16string, std::set<std::wstring>> category_extensions;
    std::map<std::u16string, win32::wparam_t> categories_to_groups;
    std::unordered_map<std::wstring_view, std::u16string> extensions_to_categories;
    std::unordered_map<siege::platform::file_info*, win32::wparam_t> file_indices;
    std::u16string filter_value;

    win32::popup_menu table_menu;
    win32::popup_menu table_settings_menu;

    SHSTOCKICONINFO default_icon{ .cbSize = sizeof(SHSTOCKICONINFO) };
    std::map<std::u16string_view, SHSTOCKICONINFO> category_icons;

    std::set<int> selected_table_items;

    bool has_console = GetConsoleWindow() != nullptr;

    std::optional<bool> has_saved = std::nullopt;
    win32::image_list image_list;

    constexpr static int extract_selected_id = 10;

    constexpr static std::u16string_view generic_category = u"All Generic Data Files";

    vol_view(win32::hwnd_t self, CREATESTRUCTW& params) : basic_window(self, params)
    {
    }

    ~vol_view()
    {
      stop_loading(shared_state);
    }

    auto wm_create()
    {
      auto app_path = std::filesystem::path(win32::module_ref().current_module().GetModuleFileName());
      modules = platform::presentation_module::load_modules(app_path.parent_path());

      for (const auto& module : modules)
      {
        auto categories = module.get_supported_format_categories();
        categories.insert(std::u16string(generic_category));

        for (auto& category : categories)
        {
          auto& stored_category = *all_categories.insert(std::move(category)).first;

          if (category_icons[stored_category].cbSize != sizeof(sizeof(SHSTOCKICONINFO)))
          {
            auto& info = category_icons[stored_category];
            info.cbSize = sizeof(SHSTOCKICONINFO);
            ::SHGetStockIconInfo((SHSTOCKICONID)module.get_default_file_icon(), SHGSI_SYSICONINDEX, &info);
          }

          auto extensions = module.get_supported_extensions_for_category(stored_category);

          if (category == generic_category)
          {
            extensions.insert(L".bin");
            extensions.insert(L".dat");
            extensions.insert(L".raw");
            extensions.insert(L"");
          }

          auto existing = category_extensions.find(stored_category);

          if (existing == category_extensions.end())
          {
            existing = category_extensions.emplace(stored_category, std::move(extensions)).first;
          }
          else
          {
            std::transform(extensions.begin(), extensions.end(), std::inserter(existing->second, existing->second.end()), [&](auto& ext) {
              return std::move(ext);
            });
          }

          for (auto& item : existing->second)
          {
            extensions_to_categories.emplace(item, stored_category);
          }
        }
      }

      table_settings = *win32::CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .hwndParent = *this, .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

      table_settings.bind_nm_click(std::bind_front(&vol_view::table_settings_nm_click, this));
      table_settings.bind_tbn_dropdown(std::bind_front(&vol_view::table_settings_tbn_dropdown, this));

      table_settings.InsertButton(-1, { .iBitmap = 0, .idCommand = LV_VIEW_DETAILS, .fsState = TBSTATE_ENABLED | TBSTATE_CHECKED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Details" }, false);

      table_settings.InsertButton(-1, { .iBitmap = 1, .idCommand = LV_VIEW_TILE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Tiles" }, false);

      table_settings.InsertButton(-1, { .fsStyle = BTNS_SEP }, false);
      table_settings.InsertButton(-1, { .iBitmap = 2, .idCommand = extract_selected_id, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Extract" }, false);

      table_settings.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);

      table_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 1, L"Open in New Tab");
      table_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 2, L"Extract");
      table_settings_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 1, L"Extract All");
      table_settings_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 2, L"Create Self-Extracting EXE");

      table = *win32::CreateWindowExW<win32::list_view>(CREATESTRUCTW{
        .hMenu = table_menu,
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
      });

      table.bind_nm_rclick(std::bind_front(&vol_view::table_nm_rclick, this));
      table.bind_nm_dbl_click(std::bind_front(&vol_view::table_nm_dbl_click, this));
      table.bind_lvn_item_changed(std::bind_front(&vol_view::table_lvn_item_changed, this));

      table.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);

      table.InsertGroup(-1, LVGROUP{
                              .pszHeader = const_cast<wchar_t*>(L"Hidden"),
                              .iGroupId = 1,
                              .state = LVGS_HIDDEN | LVGS_NOHEADER | LVGS_COLLAPSED,
                            });

      int id = 2;
      for (auto& item : category_extensions)
      {
        auto index = table.InsertGroup(-1, LVGROUP{
                                             .pszHeader = (wchar_t*)const_cast<char16_t*>(item.first.data()),
                                             .iGroupId = id,
                                             .state = LVGS_COLLAPSIBLE,
                                           });

        if (index != -1)
        {
          categories_to_groups.emplace(item.first, id++);
        }
      }

      if (!categories_to_groups.empty())
      {
        table.EnableGroupView(true);
      }

      table.InsertColumn(-1, LVCOLUMNW{
                               .pszText = const_cast<wchar_t*>(L"Filename"),
                             });

      table.InsertColumn(-1, LVCOLUMNW{
                               .pszText = const_cast<wchar_t*>(L"Path"),
                             });

      table.InsertColumn(-1, LVCOLUMNW{
                               .pszText = const_cast<wchar_t*>(L"Size (in bytes)"),
                             });

      table.InsertColumn(-1, LVCOLUMNW{
                               .pszText = const_cast<wchar_t*>(L"Compression Method"),
                             });

      table.SetExtendedListViewStyle(LVS_EX_HEADERINALLVIEWS | LVS_EX_FULLROWSELECT, LVS_EX_HEADERINALLVIEWS | LVS_EX_FULLROWSELECT);

      auto header = table.GetHeader();

      auto style = header.GetWindowStyle();

      header.SetWindowStyle(style | HDS_NOSIZING | HDS_FILTERBAR | HDS_FLAT);
      header.SetFilterChangeTimeout();
      header.bind_hdn_filter_btn_click(std::bind_front(&vol_view::table_filter_hdn_filter_btn_click, this));
      header.bind_hdn_filter_change(std::bind_front(&vol_view::table_filter_change, this));
      header.bind_hdn_end_filter_edit(std::bind_front(&vol_view::table_filter_change, this));

      HIMAGELIST image_list = nullptr;
      auto hresult = SHGetImageList(SHIL_LARGE, IID_IImageList, (void**)&image_list);

      if (hresult == S_OK)
      {
        table.SetImageList(LVSIL_NORMAL, image_list);
      }

      hresult = SHGetImageList(SHIL_SMALL, IID_IImageList, (void**)&image_list);

      if (hresult == S_OK)
      {
        table.SetImageList(LVSIL_SMALL, image_list);
      }

      hresult = SHGetStockIconInfo(SIID_MIXEDFILES, SHGSI_SYSICONINDEX, &default_icon);

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    void recreate_image_list(std::optional<SIZE> possible_size)
    {

      SIZE icon_size = possible_size.or_else([this] {
                                      return image_list.GetIconSize();
                                    })
                         .or_else([] {
                           return std::make_optional(SIZE{
                             .cx = ::GetSystemMetrics(SM_CXSIZE),
                             .cy = ::GetSystemMetrics(SM_CYSIZE) });
                         })
                         .value();

      if (image_list)
      {
        image_list.reset();
      }

      std::vector icons{ win32::segoe_fluent_icons::group_list, win32::segoe_fluent_icons::grid_view, win32::segoe_fluent_icons::folder_open };
      image_list = win32::create_icon_list(icons, icon_size);
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MINIMIZED)
      {
        return 0;
      }

      auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };

      recreate_image_list(table_settings.GetIdealIconSize(SIZE{ .cx = client_size.cx / table_settings.ButtonCount(), .cy = top_size.cy }));
      SendMessageW(table_settings, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());

      table_settings.SetWindowPos(POINT{}, SWP_DEFERERASE | SWP_NOREDRAW);
      table_settings.SetWindowPos(top_size, SWP_DEFERERASE);
      table_settings.SetButtonSize(SIZE{ .cx = client_size.cx / table_settings.ButtonCount(), .cy = top_size.cy });
      table.SetWindowPos(POINT{ .y = top_size.cy }, SWP_DEFERERASE | SWP_NOREDRAW);
      table.SetWindowPos(SIZE{ .cx = top_size.cx, .cy = client_size.cy - top_size.cy }, SWP_DEFERERASE);

      auto column_count = table.GetColumnCount();
      auto table_size = table.GetClientSize();

      auto padding = Header_GetBitmapMargin(table.GetHeader());
      auto column_width = (table_size->cx / column_count);

      for (auto i = 0u; i < column_count; ++i)
      {
        table.SetColumnWidth(i, column_width);
      }

      auto header_size = table.GetHeader().GetClientSize();

      table.GetHeader().SetWindowPos(SIZE{ .cx = top_size.cx, .cy = header_size->cy });

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        recreate_image_list(std::nullopt);
        SendMessageW(table_settings, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());

        return 0;
      }

      return std::nullopt;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (is_vol(stream))
      {
        std::optional<std::filesystem::path> path = win32::get_path_from_handle((HANDLE)message.data_type);

        auto count = load_volume(shared_state, stream, path, [&, table = table.get()](siege::platform::resource_reader::content_info& content) mutable {
          if (!::IsWindow(table))
          {
            stop_loading(shared_state);
            return;
          }

          if (auto* file = std::get_if<siege::platform::file_info>(&content))
          {
            win32::list_view_item item{ file->filename };

            auto extension = platform::to_lower(file->filename.extension().wstring());
            auto category = extensions_to_categories.find(extension);

            if (category != extensions_to_categories.end())
            {
              item.iImage = default_icon.iSysImageIndex;

              if (auto icon = category_icons.find(category->second); icon != category_icons.end())
              {
                item.iImage = icon->second.iSysImageIndex;
              }

              item.iGroupId = categories_to_groups[category->second];
              item.sub_items = {
                std::filesystem::relative(file->folder_path, file->archive_path).wstring(),
                std::to_wstring(file->size)
              };

              item.mask = item.mask | LVIF_GROUPID | LVIF_IMAGE;

              auto index = win32::list_view(table).InsertRow(item);
              file_indices.emplace(file, index);

              UINT columns[2] = { 2, 3 };
              int formats[2] = { LVCFMT_LEFT, LVCFMT_LEFT };
              LVTILEINFO item_info{ .cbSize = sizeof(LVTILEINFO), .iItem = (int)index, .cColumns = 2, .puColumns = columns, .piColFmt = formats };

              ListView_SetTileInfo(table, &item_info);
            }
          }
        });

        if (count > 0)
        {
          auto client_size = this->GetClientSize();
          wm_size(SIZE_RESTORED, *client_size);

          return TRUE;
        }

        return FALSE;
      }

      return FALSE;
    }

    // extracting of files may use external console applications,
    // so we a console window if it doesn't already exist
    void alloc_console()
    {
      if (!has_console)
      {
        if (AllocConsole())
        {
          has_console = true;
          ShowWindow(GetConsoleWindow(), SW_HIDE);
        }
      }
    }

    std::optional<std::filesystem::path> get_destination_directory()
    {
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

            if (path)
            {
              return *path;
            }
          }
        }
      }

      return std::nullopt;
    }

    void create_self_extracting_exe()
    {
      auto unvol_path = win32::find_binary_module({ "unvol.exe" });

      if (!unvol_path)
      {
        return;
      }

      decltype(unvol_path) new_unvol_path;

      auto dialog = win32::com::CreateFileSaveDialog();

      decltype(unvol_path) output_path;
      std::vector<std::string> commands;

      if (dialog)
      {
        win32::com::com_ptr<IFileDialogCustomize> customize = nullptr;

        if (dialog->get()->QueryInterface(customize.put()) == S_OK)
        {
          customize->StartVisualGroup(10, L"Enter the default output path:");

          if (auto vol_path = get_original_path(shared_state); vol_path)
          {
            auto new_path = L"C:\\Games\\" + vol_path->stem().wstring();
            customize->AddEditBox(11, new_path.c_str());
          }
          else
          {
            customize->AddEditBox(11, L"C:\\Games\\");
          }

          customize->EndVisualGroup();

          customize->StartVisualGroup(20, L"Specify commands to run:");
          customize->AddEditBox(21, L"");
          customize->AddEditBox(22, L"");
          customize->AddEditBox(23, L"");
          customize->AddEditBox(24, L"");
          customize->AddEditBox(25, L"");
          customize->AddEditBox(26, L"");
          customize->EndVisualGroup();
        }

        dialog->get()->SetDefaultExtension(L".exe");

        struct events : IFileDialogEvents
        {
          HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObject) override
          {
            if (riid == IID_IFileDialogEvents || riid == IID_IUnknown)
            {
              *ppvObject = this;
              return S_OK;
            }
            return E_NOINTERFACE;
          }

          ULONG __stdcall AddRef(void) override
          {
            return 1;
          }

          ULONG __stdcall Release(void) override
          {
            return 1;
          }

          std::function<void()> callback;

          HRESULT __stdcall OnFileOk(IFileDialog* pfd) override
          {
            callback();
            return S_OK;
          }

          HRESULT __stdcall OnFolderChanging(IFileDialog* pfd, IShellItem* psiFolder) override
          {
            return E_NOTIMPL;
          }

          HRESULT __stdcall OnFolderChange(IFileDialog* pfd) override
          {
            return E_NOTIMPL;
          }

          HRESULT __stdcall OnSelectionChange(IFileDialog* pfd)
          {
            return E_NOTIMPL;
          }

          HRESULT __stdcall OnShareViolation(IFileDialog* pfd, IShellItem* psi, FDE_SHAREVIOLATION_RESPONSE* pResponse) override
          {
            return E_NOTIMPL;
          }

          HRESULT __stdcall OnTypeChange(IFileDialog* pfd)
          {
            return E_NOTIMPL;
          }

          HRESULT __stdcall OnOverwrite(IFileDialog* pfd, IShellItem* psi, FDE_OVERWRITE_RESPONSE* pResponse) override
          {
            return E_NOTIMPL;
          }
        } callbacks;

        callbacks.callback = [&]() {
          win32::com::com_string edit_text;

          if (customize && customize->GetEditBoxText(11, edit_text.put()) == S_OK && !std::wstring_view(edit_text).empty())
          {
            output_path = edit_text;
          }

          if (customize)
          {
            std::string ascii;
            for (auto i = 21; i < 26; ++i)
            {
              win32::com::com_string temp;

              if (customize->GetEditBoxText(i, temp.put()) == S_OK && !std::wstring_view(temp).empty())
              {
                auto temp_str = std::wstring_view(temp);
                ascii.resize(temp_str.size());

                std::transform(temp_str.begin(), temp_str.end(), ascii.begin(), [](auto c) { return (char)c; });
                commands.emplace_back(ascii);
              }
            }
          }
        };

        DWORD token = 0;
        dialog->get()->Advise(&callbacks, &token);
        auto result = dialog->get()->Show(nullptr);
        dialog->get()->Unadvise(token);

        if (result == S_OK)
        {
          new_unvol_path = dialog->GetResult().value().GetFileSysPath().value();
        }
      }

      if (!new_unvol_path)
      {
        return;
      }

      fs::copy_file(*unvol_path, *new_unvol_path);

      auto final_path = create_self_extracting_resource(shared_state, *new_unvol_path, output_path, std::move(commands));

      if (!final_path)
      {
        ::MessageBoxW(*this, L"An error occurred creating the self-extracting archive", L"Could Not Create Self-Extracting Archive", MB_ICONERROR);
      }
    }

    void extract_all_files()
    {
      alloc_console();

      auto items = get_contents(shared_state);

      if (has_saved == false)
      {
        return;
      }

      auto path = get_destination_directory();

      if (!path)
      {
        return;
      }

      // TODO fix crash when extracting lots of files with our internal zip code
      if (auto archive_path = get_original_path(shared_state); IsWindows10OrGreater() && archive_path && (archive_path->extension().string() == ".pk3" || archive_path->extension().string() == ".PK3" || archive_path->extension().string() == ".zip" || archive_path->extension().string() == ".ZIP"))
      {
        std::stringstream command;
        command << "cd " << *path << " && tar -xf " << *archive_path << " && explorer .";
        std::system(command.str().c_str());
        return;
      }

      win32::queue_user_work_item([path = std::move(path), this, items]() {
        has_saved = false;

        std::for_each(items.begin(), items.end(), [this, path](auto& item) {
          if (auto* file_info = std::get_if<siege::platform::file_info>(&item.get()); file_info)
          {
            auto child_path = std::filesystem::relative(file_info->folder_path, file_info->archive_path);

            std::error_code code;
            std::filesystem::create_directories(*path / child_path, code);
            std::ofstream extracted_file(*path / child_path / file_info->filename, std::ios::trunc | std::ios::binary);
            auto raw_data = load_content_data(shared_state, item.get());

            extracted_file.write(raw_data.data(), raw_data.size());
          }
        });

        win32::launch_shell_process(*path);
        has_saved = true;
      });
    }

    void extract_selected_files()
    {
      alloc_console();

      auto path = get_destination_directory();

      if (!path)
      {
        return;
      }

      auto items = get_contents(shared_state);

      std::vector<siege::platform::resource_reader::content_info> files_to_extract;
      files_to_extract.reserve(selected_table_items.size());

      std::wstring temp;

      for (auto item_index : selected_table_items)
      {
        temp.assign(255, L'\0');

        LVITEMW item_info{
          .mask = LVIF_TEXT,
          .iItem = item_index,
          .pszText = temp.data(),
          .cchTextMax = (int)temp.size()
        };

        ListView_GetItem(table, &item_info);

        temp.resize(temp.find(L'\0'));

        auto item = std::find_if(items.begin(), items.end(), [&](auto& info) {
          siege::platform::file_info* file = std::get_if<siege::platform::file_info>(&info.get());

          return file && file->filename.wstring() == temp;
        });

        if (item != items.end())
        {
          files_to_extract.emplace_back(*item);
        }
      }

      for (auto& item : files_to_extract)
      {
        auto& file_info = std::get<siege::platform::file_info>(item);
        std::ofstream extracted_file(*path / file_info.filename, std::ios::trunc | std::ios::binary);
        auto raw_data = load_content_data(shared_state, item);

        extracted_file.write(raw_data.data(), raw_data.size());
      }

      win32::launch_shell_process(*path);
    }

    [[maybe_unused]] bool open_new_tab_for_item(LVITEMW item_info, win32::window_ref root)
    {
      auto items = get_contents(shared_state);

      auto item = std::find_if(items.begin(), items.end(), [&](auto& content) {
        if (auto* file = std::get_if<siege::platform::file_info>(&content.get()))
        {
          return std::wstring_view(file->filename.c_str()) == item_info.pszText;
        }

        return false;
      });

      if (item != items.end())
      {
        auto data = load_content_data(shared_state, *item);

        root.CopyData(*this, COPYDATASTRUCT{ .dwData = ::AddAtomW(item_info.pszText), .cbData = DWORD(data.size()), .lpData = data.data() });

        return true;
      }

      return false;
    }

    void table_nm_rclick(win32::list_view, const NMITEMACTIVATE& message)
    {
      auto point = message.ptAction;

      if (ClientToScreen(table, &point))
      {
        auto result = table_menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, point, ref());

        if (result == 1)
        {
          auto root = this->GetAncestor(GA_ROOT);

          if (root)
          {
            std::wstring temp;

            for (auto item : selected_table_items)
            {
              temp.assign(255, L'\0');
              auto item_info = table.GetItem(LVITEMW{
                .mask = LVIF_TEXT,
                .iItem = item,
                .pszText = temp.data(),
                .cchTextMax = 256 });

              if (item_info)
              {
                open_new_tab_for_item(*item_info, root->ref());
              }
            }
          }
        }
        if (result == 2)
        {
          extract_selected_files();
        }
      }
    }

    void table_nm_dbl_click(win32::list_view, const NMITEMACTIVATE& message)
    {
      auto root = this->GetAncestor(GA_ROOT);

      if (root)
      {
        std::array<wchar_t, 256> temp{};

        auto item_info = table.GetItem(LVITEMW{
          .mask = LVIF_TEXT,
          .iItem = message.iItem,
          .pszText = temp.data(),
          .cchTextMax = 256 });

        if (item_info)
        {
          open_new_tab_for_item(*item_info, root->ref());
        }
      }
    }

    void table_lvn_item_changed(win32::list_view, const NMLISTVIEW& message)
    {
      if (message.uNewState & LVIS_SELECTED)
      {
        selected_table_items.emplace(message.iItem);
      }
      else if (message.uOldState & LVIS_SELECTED)
      {
        selected_table_items.emplace(message.iItem);
      }
    }

    LRESULT table_settings_tbn_dropdown(win32::tool_bar, const NMTOOLBARW& message)
    {
      POINT point{ .x = message.rcButton.left, .y = message.rcButton.top };

      if (ClientToScreen(table, &point))
      {
        auto result = table_settings_menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, point, ref());

        if (result == 1)
        {
          extract_all_files();
        }
        else if (result == 2)
        {
          create_self_extracting_exe();
        }

        return TBDDRET_DEFAULT;
      }

      return TBDDRET_NODEFAULT;
    }

    BOOL table_settings_nm_click(win32::tool_bar, const NMMOUSE& message)
    {
      if (message.dwItemSpec == extract_selected_id)
      {
        extract_selected_files();
        return TRUE;
      }

      table.SetView(win32::list_view::view_type(message.dwItemSpec));

      return TRUE;
    }

    BOOL table_filter_hdn_filter_btn_click(win32::header, const NMHDFILTERBTNCLICK& message)
    {
      return FALSE;
    }

    // HDN_FILTERCHANGE + HDN_ENDFILTEREDIT
    void table_filter_change(win32::header, const NMHEADERW& message)
    {
      if (message.iItem == 0)
      {
        filter_value.clear();
        filter_value.resize(255, L'\0');
        HD_TEXTFILTERW string_filter{
          .pszText = (wchar_t*)filter_value.data(),
          .cchTextMax = (int)filter_value.capacity(),
        };

        auto header_item = table.GetHeader().GetItem(0, { .mask = HDI_FILTER, .type = HDFT_ISSTRING, .pvFilter = &string_filter });

        filter_value.resize(filter_value.find(u'\0'));

        if (header_item)
        {
          for (auto& item : categories_to_groups)
          {
            table.SetGroupInfo(item.second, {
                                              .mask = LVGF_STATE,
                                              .stateMask = LVGS_HIDDEN | LVGS_NOHEADER | LVGS_COLLAPSED,
                                              .state = 0,
                                            });
          }

          auto category_iter = categories_to_groups.find(filter_value);

          if (category_iter == categories_to_groups.end() && !filter_value.empty())
          {
            auto extension_iter = extensions_to_categories.find(std::wstring_view((wchar_t*)filter_value.data(), filter_value.size()));

            if (extension_iter != extensions_to_categories.end())
            {
              category_iter = categories_to_groups.find(extension_iter->second);
            }
          }

          if (category_iter != categories_to_groups.end())
          {
            for (auto& item : categories_to_groups)
            {
              if (item.first != category_iter->first)
              {
                table.SetGroupInfo(item.second, {
                                                  .mask = LVGF_STATE,
                                                  .stateMask = LVGS_HIDDEN | LVGS_NOHEADER | LVGS_COLLAPSED,
                                                  .state = LVGS_HIDDEN | LVGS_NOHEADER | LVGS_COLLAPSED,
                                                });
              }
            }
          }

          for (auto& file_index : file_indices)
          {
            if (!file_index.first)
            {
              continue;
            }

            if (siege::platform::to_lower(file_index.first->filename.u16string()).find(siege::platform::to_lower(filter_value)) == std::u16string_view::npos)
            {
              table.SetItem({
                .mask = LVIF_GROUPID,
                .iItem = int(file_index.second),
                .iGroupId = 1,
              });
            }
            else
            {
              auto extension = platform::to_lower(file_index.first->filename.extension().wstring());
              auto category = extensions_to_categories.find(extension);

              if (category != extensions_to_categories.end())
              {
                table.SetItem({
                  .mask = LVIF_GROUPID,
                  .iItem = int(file_index.second),
                  .iGroupId = int(categories_to_groups[category->second]),
                });
              }
            }
          }
        }
      }
    }

    std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
    {
      switch (message)
      {
      case WM_CREATE:
        return wm_create();
      case WM_SETTINGCHANGE:
        return wm_setting_change(win32::setting_change_message(wparam, lparam));
      case WM_SIZE:
        return (LRESULT)wm_size((std::size_t)wparam, SIZE(LOWORD(lparam), HIWORD(lparam)));
      case WM_COPYDATA:
        return (LRESULT)wm_copy_data(win32::copy_data_message<char>(wparam, lparam));
      default:
        return std::nullopt;
      }
    }
  };

  ATOM register_vol_view(win32::window_module_ref module)
  {
    WNDCLASSEXW info{
      .cbSize = sizeof(info),
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = win32::basic_window<vol_view>::window_proc,
      .cbWndExtra = sizeof(void*),
      .hInstance = module,
      .lpszClassName = win32::type_name<vol_view>().c_str(),
    };
    return ::RegisterClassExW(&info);
  }
}// namespace siege::views