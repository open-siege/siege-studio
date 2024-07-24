#ifndef WIN32_SINGULAR_CONTROLS_HPP
#define WIN32_SINGULAR_CONTROLS_HPP

#include <expected>
#include <siege/platform/win/desktop/window.hpp>
#include <siege/platform/win/desktop/notifications.hpp>

namespace win32
{
  struct button : window
  {
    using window::window;
    constexpr static auto class_name = WC_BUTTONW;
    constexpr static std::uint16_t dialog_id = 0x0080;

    struct notifications
    {
      virtual std::optional<win32::lresult_t> wm_draw_item(win32::button, unsigned int, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }
    };
  };

  struct edit : window
  {
    using window::window;
    constexpr static auto class_name = WC_EDITW;
    constexpr static std::uint16_t dialog_id = 0x0081;
  };

  struct static_control : window
  {
    using window::window;
    constexpr static auto class_name = WC_STATICW;
    constexpr static std::uint16_t dialog_id = 0x0082;

    HBITMAP SetImage(HBITMAP image)
    {
      return HBITMAP(SendMessageW(*this, STM_SETIMAGE, IMAGE_BITMAP, lparam_t(image)));
    }
  };

  struct list_box : window
  {
    using window::window;
    constexpr static auto class_name = WC_LISTBOXW;
    constexpr static std::uint16_t dialog_id = 0x0083;

    [[nodiscard]] inline DWORD GetCount()
    {
      return ListBox_GetCount(*this);
    }

    [[nodiscard]] inline lresult_t GetCurrentSelection()
    {
      return SendMessageW(*this, LB_GETCURSEL, 0, 0);
    }

    [[nodiscard]] inline lresult_t GetTextLength(wparam_t index)
    {
      return SendMessageW(*this, LB_GETTEXTLEN, index, 0);
    }

    [[nodiscard]] inline lresult_t GetItemHeight(wparam_t index)
    {
      return SendMessageW(*this, LB_GETITEMHEIGHT, index, 0);
    }

    [[maybe_unused]] inline lresult_t GetText(wparam_t index, wchar_t* data)
    {
      return SendMessageW(*this, LB_GETTEXT, index, (LPARAM)data);
    }

    [[maybe_unused]] inline lresult_t SetCurrentSelection(wparam_t index)
    {
      return SendMessageW(*this, LB_SETCURSEL, index, 0);
    }

    [[maybe_unused]] inline wparam_t AddString(std::wstring_view text)
    {
      return SendMessageW(*this, LB_ADDSTRING, 0, std::bit_cast<LPARAM>(text.data()));
    }

    [[maybe_unused]] inline wparam_t InsertString(wparam_t index, std::wstring_view text)
    {
      return SendMessageW(*this, LB_INSERTSTRING, index, std::bit_cast<LPARAM>(text.data()));
    }
  };

  struct scroll_bar : window
  {
    using window::window;
    constexpr static auto class_name = WC_SCROLLBARW;
    constexpr static std::uint16_t dialog_id = 0x0084;
  };

  struct combo_box : window
  {
    using window::window;
    constexpr static auto class_name = WC_COMBOBOXW;
    constexpr static std::uint16_t dialog_id = 0x0085;
  };
}// namespace win32


#endif