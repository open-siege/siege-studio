#ifndef WIN32_SINGULAR_CONTROLS_HPP
#define WIN32_SINGULAR_CONTROLS_HPP

#include <expected>
#include <siege/platform/win/desktop/win32_window.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct button : window
    {
        using window::window;
        constexpr static auto class_name = WC_BUTTONW;
        constexpr static std::uint16_t dialog_id = 0x0080;

        [[nodiscard]] inline std::optional<RECT> GetTextMargin(hwnd_t self)
        {
            RECT result;

            if (SendMessageW(*this, BCM_GETTEXTMARGIN, 0, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        [[nodiscard]] inline std::optional<SIZE> GetIdealSize(LONG ideal_width = 0)
        {
            SIZE result{.cx = ideal_width };

            if (SendMessageW(*this, BCM_GETIDEALSIZE, 0, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }
    };

    struct edit : window
    {
        using window::window;
        constexpr static auto class_name = WC_EDITW;
        constexpr static std::uint16_t dialog_id = 0x0081;

       [[maybe_unused]] inline bool SetCueBanner(bool show_on_focus, std::wstring text)
       {
          return SendMessageW(*this, EM_SETCUEBANNER, show_on_focus ? TRUE : FALSE, std::bit_cast<lparam_t>(text.c_str()));
       }
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

        [[nodiscard]] inline lresult_t GetCount()
        {
            return SendMessageW(*this, LB_GETCOUNT, 0, 0);
        }

        [[maybe_unused]] inline wparam_t AddString(wparam_t index, std::wstring_view text)
        {
            return SendMessageW(*this, LB_ADDSTRING, index, std::bit_cast<LPARAM>(text.data()));
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

        [[maybe_unused]] inline wparam_t AddString(wparam_t index, std::string_view text)
        {
            return SendMessageW(*this, CB_ADDSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }

        [[maybe_unused]] inline wparam_t InsertString(wparam_t index, std::string_view text)
        {
            return SendMessageW(*this, CB_INSERTSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }

        [[nodiscard]] inline std::expected<COMBOBOXINFO, DWORD> GetComboBoxInfo()
        {
            COMBOBOXINFO result;

            if (SendMessageW(*this, CB_GETCOMBOBOXINFO, 0, std::bit_cast<LPARAM>(&result)))
            {
                return result;
            }

            return std::unexpected(GetLastError());
        }
    };
}


#endif