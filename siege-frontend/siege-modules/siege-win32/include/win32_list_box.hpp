#ifndef WIN32_LIST_BOX_HPP
#define WIN32_LIST_BOX_HPP

#include "win32_user32.hpp"

namespace win32
{
    struct list_box
    {
        constexpr static auto class_name = WC_LISTBOXW;
        constexpr static std::uint16_t dialog_id = 0x0083;

        [[nodiscard]] static lresult_t GetCount(hwnd_t self)
        {
            return SendMessageW(self, LB_GETCOUNT, 0, 0);
        }

        [[maybe_unused]] static wparam_t AddString(hwnd_t self, wparam_t index, std::wstring_view text)
        {
            return SendMessageW(self, LB_ADDSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }

        [[maybe_unused]] static wparam_t InsertString(hwnd_t self, wparam_t index, std::wstring_view text)
        {
            return SendMessageW(self, LB_INSERTSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }
    };
}

#endif