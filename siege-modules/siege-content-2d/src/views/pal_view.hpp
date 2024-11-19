#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include "pal_controller.hpp"

namespace siege::views
{
  struct pal_view final : win32::window_ref
    , win32::static_control::notifications
    , win32::list_box::notifications
  {
    
    pal_controller controller;

    std::vector<win32::gdi::brush> brushes;

    win32::static_control render_view;
    win32::list_box selection;
    std::wstring buffer;

    pal_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
      buffer.reserve(64);
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | SS_OWNERDRAW });

      selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      selection.bind_lbn_sel_change(std::bind_front(&selection_lbn_sel_change, this));

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(selection);
        win32::apply_theme(*this);

        return 0;
      }

      return std::nullopt;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      auto left_size = SIZE{ .cx = (client_size.cx / 3) * 2, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy };

      render_view.SetWindowPos(left_size);
      render_view.SetWindowPos(POINT{});

      selection.SetWindowPos(right_size);
      selection.SetWindowPos(POINT{ .x = left_size.cx });

      return 0;
    }

    void selection_lbn_sel_change(win32::list_box, const NMHDR&)
    {
      auto selected = selection.GetCurrentSelection();
      auto& colours = controller.get_palette(selected);

      brushes.clear();
      brushes.reserve(colours.size());

      ::COLORREF temp;

      for (auto i = 0u; i < colours.size(); ++i)
      {
        auto temp_colour = colours[i].colour;
        temp_colour.flags = std::byte{};
        std::memcpy(&temp, &temp_colour, sizeof(temp));
        brushes.emplace_back(::CreateSolidBrush(temp));
      }

      auto rect = render_view.GetClientRect();

      if (rect)
      {
        ::InvalidateRect(render_view, &*rect, TRUE);
      }
    }


    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (controller.is_pal(stream))
      {
        auto size = controller.load_palettes(stream);

        if (size > 0)
        {
          auto& colours = controller.get_palette(0);

          brushes.reserve(colours.size());

          ::COLORREF temp;

          for (auto i = 0u; i < colours.size(); ++i)
          {
            auto temp_colour = colours[i].colour;
            temp_colour.flags = std::byte{};
            std::memcpy(&temp, &temp_colour, sizeof(temp));
            brushes.emplace_back(::CreateSolidBrush(temp));
          }

          for (auto i = 1u; i <= size; ++i)
          {
            selection.InsertString(-1, L"Palette " + std::to_wstring(i));
          }

          selection.SetCurrentSelection(0);
          return TRUE;
        }

        return FALSE;
      }

      return FALSE;
    }

    std::optional<win32::lresult_t> wm_draw_item(win32::static_control, DRAWITEMSTRUCT& item) override
    {
      if (item.hwndItem == render_view && item.itemAction == ODA_DRAWENTIRE)
      {
        auto context = win32::gdi::drawing_context_ref(item.hDC);

        auto total_width = item.rcItem.right - item.rcItem.left;
        auto total_height = item.rcItem.bottom - item.rcItem.top;
        auto total_area = total_width * total_height;
        auto best_size = (int)std::sqrt(double(total_area) / brushes.size());

        win32::rect pos{};
        pos.right = best_size;
        pos.bottom = best_size;

        for (auto i = 0; i < brushes.size(); ++i)
        {
          context.FillRect(pos, brushes[i]);
          pos.Offset(best_size, 0);

          if (pos.right > total_width)
          {
            pos.left = 0;
            pos.right = best_size;
            pos.Offset(0, best_size);
          }
        }
      }

      return TRUE;
    }
  };
}// namespace siege::views

#endif