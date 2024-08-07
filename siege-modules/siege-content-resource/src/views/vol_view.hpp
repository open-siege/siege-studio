#ifndef VOL_VIEW_HPP
#define VOL_VIEW_HPP

#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/menu.hpp>
#include <siege/platform/content_module.hpp>
#include <siege/platform/shared.hpp>
#include <spanstream>
#include <map>
#include <set>
#include <siege/platform/win/desktop/theming.hpp>
#include "vol_controller.hpp"

namespace siege::views
{
  /*

  HIMAGELIST image_list = nullptr;
                  auto hresult = SHGetImageList(SHIL_SMALL, IID_IImageList, (void**)&image_list);

                  SHSTOCKICONINFO info{ .cbSize = sizeof(SHSTOCKICONINFO) };

                  if (hresult == S_OK)
                  {
                          dir_list.SetImageList(TVSIL_NORMAL, image_list);
                          hresult = SHGetStockIconInfo(SIID_FOLDER, SHGSI_SYSICONINDEX, &info);

                          if (hresult == S_OK)
                          {
                                  root[0].item.iImage = info.iSysImageIndex;
                          }
                  }
  */

  struct vol_view final : win32::window_ref
    , win32::list_view::notifications
    , win32::tool_bar::notifications
    , win32::header::notifications
  {
    using win32::list_view::notifications::wm_notify;
    using win32::tool_bar::notifications::wm_notify;
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

      table_settings = *factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });
      table_settings.LoadImages(IDB_VIEW_SMALL_COLOR);

      table_settings.InsertButton(-1, { .iBitmap = VIEW_DETAILS, .idCommand = LV_VIEW_DETAILS, .fsState = TBSTATE_ENABLED | TBSTATE_CHECKED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Details" });

      table_settings.InsertButton(-1, { .iBitmap = VIEW_LARGEICONS, .idCommand = LV_VIEW_ICON, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Large Icons" });

      table_settings.InsertButton(-1, { .iBitmap = VIEW_SMALLICONS, .idCommand = LV_VIEW_SMALLICON, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Small Icons" });

      table_settings.InsertButton(-1, { .iBitmap = VIEW_LIST, .idCommand = LV_VIEW_LIST, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"List" });

      table = *factory.CreateWindowExW<win32::list_view>(CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
      });

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
      header.SetWindowStyle(style | HDS_FILTERBAR);
      header.SetFilterChangeTimeout();

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };
      table_settings.SetWindowPos(top_size);
      table_settings.SetWindowPos(POINT{});
      table_settings.AutoSize();

      table.SetWindowPos(SIZE{ .cx = client_size.cx, .cy = client_size.cy - top_size.cy });
      table.SetWindowPos(POINT{ .y = top_size.cy });

      auto column_count = table.GetColumnCount();

      auto column_width = client_size.cx / column_count;

      for (auto i = 0u; i < column_count; ++i)
      {
        table.SetColumnWidth(i, column_width);
      }

      LVTILEVIEWINFO tileViewInfo = { 0 };

      tileViewInfo.cbSize = sizeof(tileViewInfo);
      tileViewInfo.dwFlags = LVTVIF_AUTOSIZE;
      tileViewInfo.dwMask = LVTVIM_COLUMNS;
      tileViewInfo.cLines = 2;

      ListView_SetTileViewInfo(table, &tileViewInfo);

      return 0;
    }

    inline static std::set<std::wstring> strings;

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        auto parent = this->GetParent();

        win32::apply_theme(*parent, table_settings);
        win32::apply_theme(*parent, table);
        auto header = table.GetHeader();
        win32::apply_theme(*parent, header);

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
            }
          }

          return TRUE;
        }

        return FALSE;
      }

      return FALSE;
    }

    auto wm_erase_background(win32::gdi_drawing_context_ref context)
    {
      static auto black_brush = ::CreateSolidBrush(0x00000000);

      auto rect = GetClientRect();
      context.FillRect(*rect, black_brush);

      return TRUE;
    }

    std::optional<win32::lresult_t> wm_notify(win32::list_view, const NMITEMACTIVATE& message) override
    {
      switch (message.hdr.code)
      {
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
            auto items = controller.get_contents();

            auto item = std::find_if(items.begin(), items.end(), [&](auto& content) {
              if (auto* file = std::get_if<siege::platform::file_info>(&content))
              {
                return std::wstring_view(file->filename.c_str()) == item_info->pszText;
              }

              return false;
            });

            if (item != items.end())
            {
              auto data = controller.load_content_data(*item);

              root->SetPropW(L"FilePath", temp.data());
              root->CopyData(*this, COPYDATASTRUCT{ .cbData = DWORD(data.size()), .lpData = data.data() });

              root->RemovePropW(L"FilePath");
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

    BOOL wm_notify(win32::tool_bar, const NMMOUSE& message) override
    {
      switch (message.hdr.code)
      {
      case NM_CLICK: {
        table.SetView(win32::list_view::view_type(message.dwItemSpec));
        return TRUE;
      }
      default: {
        return FALSE;
      }
      }
    }

    BOOL wm_notify(win32::header, NMHDFILTERBTNCLICK& message) override
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