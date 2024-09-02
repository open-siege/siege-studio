#ifndef VOL_VIEW_HPP
#define VOL_VIEW_HPP

#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/menu.hpp>
#include <siege/platform/win/desktop/shell.hpp>
#include <siege/platform/content_module.hpp>
#include <siege/platform/shared.hpp>
#include <spanstream>
#include <map>
#include <set>
#include <algorithm>
#include <future>
#include <siege/platform/win/desktop/theming.hpp>
#include "vol_controller.hpp"

namespace siege::views
{
  struct vol_view final : win32::window_ref
    , win32::list_view::notifications
    , win32::tool_bar::notifications
    , win32::header::notifications
  {
    vol_controller controller;

    win32::tool_bar table_settings;
    win32::list_view table;

    std::list<platform::content_module> modules;

    std::set<std::u16string> all_categories;
    std::map<std::u16string, std::set<std::wstring>> category_extensions;
    std::map<std::u16string, win32::wparam_t> categories_to_groups;
    std::map<std::wstring_view, std::u16string> extensions_to_categories;
    std::map<siege::platform::file_info*, win32::wparam_t> file_indices;
    std::u16string filter_value;

    win32::popup_menu table_menu;
    win32::popup_menu table_settings_menu;

    SHSTOCKICONINFO default_icon{ .cbSize = sizeof(SHSTOCKICONINFO) };
    std::map<std::u16string_view, SHSTOCKICONINFO> category_icons;

    std::set<int> selected_table_items;

    bool has_console = GetConsoleWindow() != nullptr;

    std::future<void> pending_save;
    std::optional<bool> has_saved = std::nullopt;

    constexpr static int extract_selected_id = 10;

    vol_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      auto app_path = std::filesystem::path(win32::module_ref().current_application().GetModuleFileName());
      modules = platform::content_module::load_modules(app_path.parent_path());

      for (const auto& module : modules)
      {
        auto categories = module.get_supported_format_categories();

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

      auto factory = win32::window_factory(ref());

      table_settings = *factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });
      table_settings.LoadImages(IDB_VIEW_SMALL_COLOR);

      table_settings.InsertButton(-1, { .iBitmap = VIEW_DETAILS, .idCommand = LV_VIEW_DETAILS, .fsState = TBSTATE_ENABLED | TBSTATE_CHECKED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Details" }, false);

      table_settings.InsertButton(-1, { .iBitmap = VIEW_LARGEICONS, .idCommand = LV_VIEW_ICON, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Large Icons" }, false);

      table_settings.InsertButton(-1, { .iBitmap = VIEW_SMALLICONS, .idCommand = LV_VIEW_SMALLICON, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Small Icons" }, false);

      table_settings.InsertButton(-1, { .iBitmap = VIEW_LIST, .idCommand = LV_VIEW_LIST, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"List" }, false);
      table_settings.InsertButton(-1, { .fsStyle = BTNS_SEP }, false);
      table_settings.InsertButton(-1, { .iBitmap = VIEW_PARENTFOLDER, .idCommand = extract_selected_id, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Extract" }, false);

      table_settings.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);

      table_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 1, L"Open in New Tab");
      table_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 2, L"Extract");
      table_settings_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 1, L"Extract All");

      table = *factory.CreateWindowExW<win32::list_view>(CREATESTRUCTW{
        .hMenu = table_menu,
        .style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
      });

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

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MINIMIZED)
      {
        return 0;
      }

      auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };
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

    inline static std::set<std::wstring> strings;

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(table_settings);
        win32::apply_theme(table);
        auto header = table.GetHeader();
        win32::apply_theme(header);
        win32::apply_theme(*this);

        return 0;
      }

      return std::nullopt;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (vol_controller::is_vol(stream))
      {
        std::optional<std::filesystem::path> path;

        if (wchar_t* filename = this->GetPropW<wchar_t*>(L"FilePath"); filename)
        {
          path = filename;
        }

        auto count = controller.load_volume(stream, path);

        if (count > 0)
        {
          auto contents = controller.get_contents();

          for (auto& content : contents)
          {
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
                  std::filesystem::relative(file->archive_path, file->folder_path).wstring(),
                  std::to_wstring(file->size)
                };

                auto index = table.InsertRow(item);
                file_indices.emplace(file, index);

                UINT columns[2] = { 2, 3 };
                int formats[2] = { LVCFMT_LEFT, LVCFMT_LEFT };
                LVTILEINFO item_info{ .cbSize = sizeof(LVTILEINFO), .iItem = (int)index, .cColumns = 2, .puColumns = columns, .piColFmt = formats };

                ListView_SetTileInfo(table, &item_info);
              }
              auto client_size = this->GetClientSize();
              wm_size(SIZE_RESTORED, *client_size);
            }
          }

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

    void extract_all_files()
    {
      alloc_console();
      auto items = controller.get_contents();

      if (has_saved == false)
      {
        return;
      }

      auto path = get_destination_directory();

      if (!path)
      {
        return;
      }

      pending_save = std::async(
        std::launch::async, [path = std::move(path), this](auto items) {
          has_saved = false;

          for (auto& item : items)
          {
            if (auto* file_info = std::get_if<siege::platform::file_info>(&item); file_info)
            {
              std::ofstream extracted_file(*path / file_info->filename, std::ios::trunc | std::ios::binary);
              auto raw_data = controller.load_content_data(item);

              extracted_file.write(raw_data.data(), raw_data.size());
            }
          }

          has_saved = true;
        },
        items);
    }

    void extract_selected_files()
    {
      alloc_console();
      
      
      auto path = get_destination_directory();

      if (!path)
      {
        return;
      }
      
      auto items = controller.get_contents();

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
          siege::platform::file_info* file = std::get_if<siege::platform::file_info>(&info);

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
        auto raw_data = controller.load_content_data(item);

        extracted_file.write(raw_data.data(), raw_data.size());
      }
    }

    [[maybe_unused]] bool open_new_tab_for_item(LVITEMW item_info, win32::window_ref root)
    {
      auto items = controller.get_contents();

      auto item = std::find_if(items.begin(), items.end(), [&](auto& content) {
        if (auto* file = std::get_if<siege::platform::file_info>(&content))
        {
          return std::wstring_view(file->filename.c_str()) == item_info.pszText;
        }

        return false;
      });

      if (item != items.end())
      {
        auto data = controller.load_content_data(*item);

        root.SetPropW(L"FilePath", item_info.pszText);
        root.CopyData(*this, COPYDATASTRUCT{ .cbData = DWORD(data.size()), .lpData = data.data() });

        root.RemovePropW(L"FilePath");
        return true;
      }

      return false;
    }

    std::optional<win32::lresult_t> wm_notify(win32::list_view, const NMITEMACTIVATE& message) override
    {
      switch (message.hdr.code)
      {
      case NM_RCLICK: {
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

        return 0;
      }
      case NM_DBLCLK: {
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
        return 0;
      }
      default: {
        return std::nullopt;
      }
      }
    }

    std::optional<win32::lresult_t> wm_notify(win32::list_view, const NMLISTVIEW& message) override
    {
      switch (message.hdr.code)
      {
      case LVN_ITEMCHANGED: {
        if (message.uNewState & LVIS_SELECTED)
        {
          selected_table_items.emplace(message.iItem);
        }
        else if (message.uOldState & LVIS_SELECTED)
        {
          selected_table_items.emplace(message.iItem);
        }
        return 0;
      }
      default: {
        return FALSE;
      }
      }
    }

    std::optional<win32::lresult_t> wm_notify(win32::tool_bar, const NMTOOLBARW& message) override
    {
      switch (message.hdr.code)
      {
      case TBN_DROPDOWN: {
        POINT point{ .x = message.rcButton.left, .y = message.rcButton.top };

        if (ClientToScreen(table, &point))
        {
          auto result = table_settings_menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, point, ref());

          if (result == 1)
          {
            extract_all_files();
          }

          return TBDDRET_DEFAULT;
        }

        return TBDDRET_NODEFAULT;
      }
      default: {
        return FALSE;
      }
      }
    }

    std::optional<BOOL> wm_notify(win32::tool_bar, const NMMOUSE& message) override
    {
      switch (message.hdr.code)
      {
      case NM_CLICK: {
        if (message.dwItemSpec == extract_selected_id)
        {
          extract_selected_files();
          return TRUE;
        }

        table.SetView(win32::list_view::view_type(message.dwItemSpec));
        return TRUE;
      }
      default: {
        return FALSE;
      }
      }
    }

    std::optional<win32::lresult_t> wm_notify(win32::header, NMHDFILTERBTNCLICK& message) override
    {
      return FALSE;
    }

    std::optional<win32::lresult_t> wm_notify(win32::header, NMHEADERW& message) override
    {
      switch (message.hdr.code)
      {
      case HDN_FILTERCHANGE:
      case HDN_ENDFILTEREDIT: {
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

            auto contents = controller.get_contents();

            for (auto& content : contents)
            {
              if (auto* file = std::get_if<siege::platform::file_info>(&content))
              {
                auto index_iter = file_indices.find(file);

                if (index_iter == file_indices.end())
                {
                  continue;
                }

                if (file->filename.u16string().find(filter_value) == std::u16string_view::npos)
                {
                  table.SetItem({
                    .mask = LVIF_GROUPID,
                    .iItem = int(index_iter->second),
                    .iGroupId = 1,
                  });
                }
                else
                {
                  auto extension = platform::to_lower(file->filename.extension().wstring());
                  auto category = extensions_to_categories.find(extension);

                  if (category != extensions_to_categories.end())
                  {
                    table.SetItem({
                      .mask = LVIF_GROUPID,
                      .iItem = int(index_iter->second),
                      .iGroupId = int(categories_to_groups[category->second]),
                    });
                  }
                }
              }
            }
          }
        }
        return 0;
      }
      default: {
        return std::nullopt;
      }
      }
    }
  };

}// namespace siege::views

#endif