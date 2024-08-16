#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/shell.hpp>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/storage_module.hpp>
#include <cassert>
#include <sstream>
#include <vector>
#include <memory>
#include <istream>
#include <spanstream>
#include <oleacc.h>
#include "bmp_controller.hpp"
#include "pal_controller.hpp"

namespace siege::views
{
  struct bmp_view : win32::window_ref
  {
    win32::static_control static_image;
    win32::list_view palettes_list;
    win32::button zoom;
    win32::button frame_selector;

    win32::static_control frame_label;
    win32::static_control frame_value;

    win32::static_control zoom_label;
    win32::static_control zoom_value;

    bmp_controller controller;

    win32::gdi::bitmap current_bitmap;

    std::list<platform::storage_module> loaded_modules;

    bmp_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      std::filesystem::path app_path = std::filesystem::path(win32::module_ref::current_application().GetModuleFileName()).parent_path();
      loaded_modules = platform::storage_module::load_modules(std::filesystem::path(app_path));

      auto factory = win32::window_factory(ref());

      std::wstring temp = L"menu.pal";

      frame_selector = *factory.CreateWindowExW<win32::button>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | BTNS_BUTTON,
        .lpszName = L"Frame Selector" });

      frame_label = *factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | SS_LEFT,
        .lpszName = L"Frame: " });

      frame_value = *factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | SS_LEFT,
        .lpszName = L"" });

      zoom = *factory.CreateWindowExW<win32::button>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | BTNS_BUTTON,
        .lpszName = L"Zoom" });

      zoom_label = *factory.CreateWindowExW<win32::static_control>(
        ::CREATESTRUCTW{
          .style = WS_VISIBLE | WS_CHILD | SS_LEFT,
          .lpszName = L"Zoom: ",
        });

      zoom_value = *factory.CreateWindowExW<win32::static_control>(
        ::CREATESTRUCTW{
          .style = WS_VISIBLE | WS_CHILD | SS_LEFT,
        });

      ::SendMessageW(zoom, UDM_SETBUDDY, win32::wparam_t(win32::hwnd_t(zoom_value)), 0);

      ::SendMessageW(zoom, UDM_SETRANGE, 0, MAKELPARAM(100, 1));

      palettes_list = [&] {
        auto palettes_list = *factory.CreateWindowExW<win32::list_view>(::CREATESTRUCTW{
          .style = WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
          .lpszName = L"Palettes" });

        palettes_list.SetView(win32::list_view::view_type::details_view);
        assert(palettes_list.EnableGroupView(true));
        assert(palettes_list.SetTileViewInfo(LVTILEVIEWINFO{
          .dwFlags = LVTVIF_FIXEDWIDTH,
          .sizeTile = SIZE{ .cx = this->GetClientSize()->cx, .cy = 50 },
        }));

        palettes_list.SetExtendedListViewStyle(0,
          LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

        palettes_list.win32::list_view::InsertColumn(-1, LVCOLUMNW{ .pszText = const_cast<wchar_t*>(L"Available Palettes") });

        auto header = palettes_list.GetHeader();
            
        auto style = header.GetWindowStyle();
        header.SetWindowStyle(style | HDS_FLAT | HDS_NOSIZING | HDS_FILTERBAR);
        return palettes_list;
      }();

      static_image = *factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | SS_BITMAP | SS_REALSIZECONTROL });

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    std::optional<win32::lresult_t> wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MINIMIZED || type == SIZE_MAXHIDE || type == SIZE_MAXSHOW)
      {
        return std::nullopt;
      }

      auto left_size = SIZE{ .cx = (client_size.cx / 3) * 2, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy };

      const auto top_left_height = left_size.cy / 20;
      const auto top_left_width = left_size.cx / 6;

      auto x = 0;

      frame_label.SetWindowPos(POINT{ .x = x });
      frame_label.SetWindowPos(SIZE{ .cx = left_size.cx / 6, .cy = top_left_height });
      x += top_left_width;

      frame_value.SetWindowPos(POINT{ .x = x });
      frame_value.SetWindowPos(SIZE{ .cx = left_size.cx / 6, .cy = top_left_height });
      x += top_left_width;

      frame_selector.SetWindowPos(POINT{ .x = x });
      frame_selector.SetWindowPos(SIZE{ .cx = left_size.cx / 6, .cy = top_left_height });
      x += top_left_width;

      zoom_label.SetWindowPos(POINT{ .x = x });
      zoom_label.SetWindowPos(SIZE{ .cx = left_size.cx / 6, .cy = top_left_height });
      x += top_left_width;

      zoom_value.SetWindowPos(POINT{ .x = x });
      zoom_value.SetWindowPos(SIZE{ .cx = left_size.cx / 6, .cy = top_left_height });
      x += top_left_width;

      zoom.SetWindowPos(POINT{ .x = x });
      zoom.SetWindowPos(SIZE{ .cx = left_size.cx / 6, .cy = top_left_height });

      static_image.SetWindowPos(POINT{ .y = top_left_height });
      static_image.SetWindowPos(SIZE{ .cx = left_size.cx, .cy = left_size.cy - top_left_height });

      palettes_list.SetWindowPos(POINT{ .x = left_size.cx });
      palettes_list.SetWindowPos(right_size);
      palettes_list.SetColumnWidth(0, right_size.cx);

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(palettes_list);
        win32::apply_theme(frame_label);
        win32::apply_theme(frame_value);
        win32::apply_theme(zoom_label);
        win32::apply_theme(zoom_value);
        //win32::apply_theme(*parent, zoom);
        //win32::apply_theme(*parent, frame_selector);

        return 0;
      }

      return std::nullopt;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (bmp_controller::is_bmp(stream))
      {
        std::unordered_map<std::filesystem::path, siege::platform::storage_module::context_ptr> loaded_contexts;

        auto task = controller.load_palettes_async(
          std::nullopt, [&](auto path) {
          
          siege::platform::storage_info info{
            .type = siege::platform::storage_info::file
          };
          info.info.path = path.c_str();

          auto resource_module = std::find_if(loaded_modules.begin(), loaded_modules.end(), [&](auto& module) {
            return module.stream_is_resource_reader(info);
          });

          if (resource_module == loaded_modules.end())
          {
            return std::set<std::filesystem::path>{};
          }

          auto context = resource_module->create_reader_context(info);

          if (!context)
          {
            return std::set<std::filesystem::path>{};
          }

          auto files = resource_module->get_reader_file_listing(*context.value(), path);

          if (!files)
          {
            return std::set<std::filesystem::path>{};
          }

          std::set<std::filesystem::path> results;

          for (auto& file : files.value())
          {
            auto is_pal = std::any_of(pal_controller::formats.begin(), pal_controller::formats.end(), [&](auto& ext) { return file.extension().c_str() == ext; });

            if (is_pal)
            {
              results.emplace(std::move(file));
            }
          }

          if (!results.empty())
          {
            loaded_contexts.emplace(std::move(path), std::move(context.value()));
          }

          return results; },

          [&](auto path) {
            auto storage_path = path.parent_path();

            auto existing = loaded_contexts.find(storage_path);

            if (existing != loaded_contexts.end())
            {
              // TODO create bound wrapper so that we don't have to find the module again
              auto result = loaded_modules.begin()->extract_file_contents(*existing->second.get(), path);

              if (result)
              {
                return std::move(result.value());
              }
            }

            return std::vector<char>{};
          });

        auto count = controller.load_bitmap(stream, task);

        if (count > 0)
        {
          ::SendMessageW(frame_selector, UDM_SETRANGE, 0, MAKELPARAM(count, 1));
          auto size = static_image.GetClientSize();
          BITMAPINFO info{
            .bmiHeader{
              .biSize = sizeof(BITMAPINFOHEADER),
              .biWidth = LONG(size->cx),
              .biHeight = LONG(size->cy),
              .biPlanes = 1,
              .biBitCount = 32,
              .biCompression = BI_RGB }
          };

          void* pixels = nullptr;
          current_bitmap.reset(::CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &pixels, nullptr, 0));

          controller.convert(0, std::make_pair(size->cx, size->cy), 32, std::span(reinterpret_cast<std::byte*>(pixels), size->cx * size->cy * 4));

          static_image.SetImage(current_bitmap.get());

          auto& palettes = task.get();
          auto selection = controller.get_selected_palette();
          auto p = 1u;
          std::vector<win32::list_view_group> groups;

          auto index = 0;
          auto selected_index = 0;

          for (auto& pal : palettes)
          {
            std::vector<win32::list_view_item> items;
            items.reserve(pal.children.size());

            auto c = 1u;

            for (auto& child : pal.children)
            {
              auto& child_item = items.emplace_back(win32::list_view_item(L"Palette " + std::to_wstring(c++)));

              if (selection.first->path == pal.path && selection.second == (c - 2))
              {
                selected_index = index;
              }

              index++;
            }

            auto& new_group = groups.emplace_back(pal.path.filename().wstring(), std::move(items));
            new_group.state = LVGS_COLLAPSIBLE;
          }

          palettes_list.InsertGroups(groups);

          ListView_SetCheckState(palettes_list, selected_index, TRUE);
          ListView_SetItemState(palettes_list, selected_index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

          return TRUE;
        }

        return FALSE;
      }

      return FALSE;
    }
  };
}// namespace siege::views

#endif