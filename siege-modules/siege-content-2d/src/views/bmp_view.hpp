#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/shell.hpp>
#include <siege/platform/win/desktop/wic.hpp>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/storage_module.hpp>
#undef NDEBUG
#include <cassert>
#include <sstream>
#include <vector>
#include <memory>
#include <istream>
#include <spanstream>
#include "bmp_controller.hpp"
#include "pal_controller.hpp"

namespace siege::views
{
  // TODO complete palette selection feature
  // TODO complete frame selection feature
  // TODO complete icon loading
  struct bmp_view : win32::window_ref
  {
    win32::list_view palettes_list;
    win32::tool_bar bitmap_actions;
    win32::image_list bitmap_actions_icons;

    bmp_controller controller;
    win32::gdi::bitmap preview_bitmap;
    win32::gdi::bitmap current_frame;
    WICRect viewport;
    WICRect previous_viewport;
    std::size_t current_frame_index = 0;
    float scale = NAN;
    bool is_panning = false;
    std::optional<POINTS> last_mouse_position = std::nullopt;
    win32::static_control static_image;
    UINT_PTR pan_timer = 0;
    std::list<platform::storage_module> loaded_modules;


    bmp_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      std::filesystem::path app_path = std::filesystem::path(win32::module_ref::current_application().GetModuleFileName()).parent_path();
      loaded_modules = platform::storage_module::load_modules(std::filesystem::path(app_path));

      auto factory = win32::window_factory(ref());

      bitmap_actions = *factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

      bitmap_actions.InsertButton(-1, { .iBitmap = 0, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Zoom In" });

      bitmap_actions.InsertButton(-1, { .iBitmap = 1, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Zoom Out" });

      bitmap_actions.InsertButton(-1, { .iBitmap = 2, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECK, .iString = (INT_PTR)L"Pan" });

      bitmap_actions.InsertButton(-1, { .fsStyle = BTNS_SEP });

      bitmap_actions.InsertButton(-1, { .iBitmap = 3, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN, .iString = (INT_PTR)L"Frame" });

      bitmap_actions.InsertButton(-1, { .fsStyle = BTNS_SEP });
      bitmap_actions.InsertButton(-1, { .iBitmap = 4, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Save" });

      bitmap_actions.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);
      bitmap_actions.bind_nm_click(std::bind_front(&bmp_view::bitmap_actions_nm_click, this));

      std::wstring temp = L"menu.pal";

      palettes_list = [&] {
        auto palettes_list = *factory.CreateWindowExW<win32::list_view>(::CREATESTRUCTW{
          .style = WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | WS_CLIPSIBLINGS,
          .lpszName = L"Palettes" });

        palettes_list.SetView(win32::list_view::view_type::details_view);
        assert(palettes_list.EnableGroupView(true));
        assert(palettes_list.SetTileViewInfo(LVTILEVIEWINFO{
          .dwFlags = LVTVIF_FIXEDWIDTH,
          .sizeTile = SIZE{ .cx = this->GetClientSize()->cx, .cy = 50 },
        }));

        palettes_list.SetExtendedListViewStyle(0,
          LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS | LVS_EX_DOUBLEBUFFER);

        palettes_list.win32::list_view::InsertColumn(-1, LVCOLUMNW{ .pszText = const_cast<wchar_t*>(L"Available Palettes") });

        auto header = palettes_list.GetHeader();

        auto style = header.GetWindowStyle();
        header.SetWindowStyle(style | HDS_FLAT | HDS_NOSIZING | HDS_FILTERBAR);
        return palettes_list;
      }();

      static_image = *factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_BITMAP | SS_REALSIZEIMAGE | SS_CENTERIMAGE });

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }


    static VOID CALLBACK timer_callback(
      HWND hwnd,
      UINT message,
      UINT_PTR idTimer,
      DWORD dwTime)
    {
      auto* self = (bmp_view*)idTimer;
      self->resize_preview(true);
    }


    void set_is_panning(bool is_panning)
    {
      this->is_panning = is_panning;

      auto state = SendMessageW(bitmap_actions, TB_GETSTATE, 2, 0);

      if (state != -1)
      {
        SendMessageW(bitmap_actions, TB_SETSTATE, 2, is_panning ? MAKEWORD(state | TBSTATE_CHECKED, 0) : MAKEWORD(state & ~TBSTATE_CHECKED, 0));
      }

      if (is_panning && !pan_timer)
      {
        pan_timer = SetTimer(*this, (UINT_PTR)this, 150, timer_callback);
      }
      else if (pan_timer)
      {
        KillTimer(*this, (UINT_PTR)this);
        pan_timer = 0;
      }
    }

    BOOL bitmap_actions_nm_click(win32::tool_bar, const NMMOUSE& message)
    {
      if (message.dwItemSpec == 0)
      {
        this->viewport.Width -= 10;
        this->viewport.Height -= 10;

        resize_preview(true);
        return TRUE;
      }
      else if (message.dwItemSpec == 1)
      {
        this->viewport.Width += 10;
        this->viewport.Height += 10;

        resize_preview(true);
        return TRUE;
      }
      else if (message.dwItemSpec == 2)
      {
        set_is_panning(!is_panning);
        return TRUE;
      }
      return FALSE;
    }

    void recreate_image_list(std::optional<SIZE> possible_size)
    {

      SIZE icon_size = possible_size.or_else([this] {
                                      return bitmap_actions_icons.GetIconSize();
                                    })
                         .or_else([] {
                           return std::make_optional(SIZE{
                             .cx = ::GetSystemMetrics(SM_CXSIZE),
                             .cy = ::GetSystemMetrics(SM_CYSIZE) });
                         })
                         .value();

      if (bitmap_actions_icons)
      {
        bitmap_actions_icons.reset();
      }

      std::vector icons{
        win32::segoe_fluent_icons::zoom_in,
        win32::segoe_fluent_icons::zoom_out,
        win32::segoe_fluent_icons::pan_mode,
        win32::segoe_fluent_icons::picture,
        win32::segoe_fluent_icons::save,
      };

      bitmap_actions_icons = win32::create_icon_list(icons, icon_size);
    }

    auto wm_mouse_button_down(std::size_t wparam, POINTS mouse_position)
    {
      if (wparam & MK_RBUTTON)
      {
        set_is_panning(!is_panning);
      }

      return 0;
    }

    auto wm_mouse_move(std::size_t wparam, POINTS mouse_position)
    {
      if (is_panning)
      {
        if (wparam & MK_RBUTTON)
        {
          set_is_panning(false);
          return 0;
        }

        if (last_mouse_position && mouse_position.x > last_mouse_position->x)
        {
          viewport.X -= mouse_position.x - last_mouse_position->x;
        }
        else if (last_mouse_position && mouse_position.x < last_mouse_position->x)
        {
          viewport.X += last_mouse_position->x - mouse_position.x;
        }

        if (last_mouse_position && mouse_position.y > last_mouse_position->y)
        {
          viewport.Y -= mouse_position.y - last_mouse_position->y;
        }
        else if (last_mouse_position && mouse_position.y < last_mouse_position->y)
        {
          viewport.Y += last_mouse_position->y - mouse_position.y;
        }
        last_mouse_position = mouse_position;
      }
      return 0;
    }

    std::optional<win32::lresult_t> wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MINIMIZED || type == SIZE_MAXHIDE || type == SIZE_MAXSHOW)
      {
        return std::nullopt;
      }

      auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };

      recreate_image_list(bitmap_actions.GetIdealIconSize(SIZE{ .cx = client_size.cx / (LONG)bitmap_actions.ButtonCount(), .cy = top_size.cy }));
      SendMessageW(bitmap_actions, TB_SETIMAGELIST, 0, (LPARAM)bitmap_actions_icons.get());

      bitmap_actions.SetWindowPos(POINT{}, SWP_DEFERERASE | SWP_NOREDRAW);
      bitmap_actions.SetWindowPos(top_size, SWP_DEFERERASE);
      bitmap_actions.SetButtonSize(SIZE{ .cx = top_size.cx / (LONG)bitmap_actions.ButtonCount(), .cy = top_size.cy });

      auto left_size = SIZE{ .cx = (client_size.cx / 3) * 2, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy - top_size.cy };

      const auto top_left_width = left_size.cx / 6;

      static_image.SetWindowPos(POINT{ .y = top_size.cy });
      static_image.SetWindowPos(SIZE{ .cx = left_size.cx, .cy = left_size.cy - top_size.cy });

      palettes_list.SetWindowPos(POINT{ .x = left_size.cx, .y = top_size.cy });
      palettes_list.SetWindowPos(right_size);
      palettes_list.SetColumnWidth(0, right_size.cx);

      resize_preview();

      return 0;
    }

    void resize_preview(bool force = false)
    {
      if (scale != NAN && current_frame)
      {
        auto frame_size = controller.get_size(current_frame_index);

        if (this->viewport.Width > frame_size.width)
        {
          this->viewport.Width = frame_size.width;
        }

        if (this->viewport.Height > frame_size.height)
        {
          this->viewport.Height = frame_size.height;
        }

        if (this->viewport.Height <= 0)
        {
          this->viewport.Height += 10;
        }

        if (this->viewport.Width <= 0)
        {
          this->viewport.Width += 10;
        }

        if (this->viewport.Height <= 0)
        {
          this->viewport.Height += 10;
        }

        if (this->viewport.X < 0)
        {
          this->viewport.X = 0;
        }

        if (this->viewport.Y < 0)
        {
          this->viewport.Y = 0;
        }

        if (this->viewport.X > frame_size.width - viewport.Width)
        {
          this->viewport.X = frame_size.width - viewport.Width;
        }

        if (this->viewport.Y > frame_size.height - viewport.Height)
        {
          this->viewport.Y = frame_size.height - viewport.Height;
        }

        if (viewport.X + viewport.Width > frame_size.width)
        {
          viewport.Width = frame_size.width - viewport.X;
        }

        if (viewport.Y + viewport.Y > frame_size.height)
        {
          viewport.Height = frame_size.height - viewport.Y;
        }
        SIZE preview_size = { .cx = (LONG)(frame_size.width * scale), .cy = (LONG)(frame_size.height * scale) };

        auto existing_size = preview_bitmap.get_size();

        if (!(existing_size.cx == preview_size.cx && existing_size.cy == preview_size.cy))
        {
          preview_bitmap = win32::gdi::bitmap(preview_size, win32::gdi::bitmap::skip_shared_handle);

          win32::wic::bitmap(current_frame, win32::wic::alpha_channel_option::WICBitmapUseAlpha)
            .clip(viewport)
            .scale((std::uint32_t)preview_size.cx, (std::uint32_t)preview_size.cy, WICBitmapInterpolationModeFant)
            .copy_pixels(preview_size.cx * sizeof(std::uint32_t), preview_bitmap.get_pixels_as_bytes());

          static_image.SetImage(preview_bitmap.get());
        }
        else if (force && std::memcmp(&viewport, &previous_viewport, sizeof(viewport)) != 0)
        {
          win32::wic::bitmap(current_frame, win32::wic::alpha_channel_option::WICBitmapUseAlpha)
            .clip(viewport)
            .scale((std::uint32_t)preview_size.cx, (std::uint32_t)preview_size.cy, WICBitmapInterpolationModeFant)
            .copy_pixels(preview_size.cx * sizeof(std::uint32_t), preview_bitmap.get_pixels_as_bytes());

          static_image.SetImage(preview_bitmap.get());
        }
        previous_viewport = viewport;
      }
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(palettes_list);
        win32::apply_theme(bitmap_actions);
        win32::apply_theme(static_image);
        win32::apply_theme(*this);

        recreate_image_list(std::nullopt);
        SendMessageW(bitmap_actions, TB_SETIMAGELIST, 0, (LPARAM)bitmap_actions_icons.get());


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
          auto size = static_image.GetClientSize();
          auto frame_size = controller.get_size(current_frame_index);
          current_frame = win32::gdi::bitmap(SIZE{ .cx = frame_size.width, .cy = frame_size.height });
          controller.convert(0, frame_size, 32, current_frame.get_pixels_as_bytes());
          viewport = WICRect();
          viewport.Width = (INT)frame_size.width;
          viewport.Height = (INT)frame_size.height;

          scale = size->cx > size->cy ? size->cx / (float)frame_size.width : size->cy / (float)frame_size.height;

          SIZE preview_size = { .cx = (LONG)(frame_size.width * scale), .cy = (LONG)(frame_size.height * scale) };

          resize_preview();

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