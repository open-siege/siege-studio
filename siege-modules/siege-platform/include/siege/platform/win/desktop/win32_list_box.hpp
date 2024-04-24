#ifndef WIN32_LIST_BOX_HPP
#define WIN32_LIST_BOX_HPP

#include <siege/platform/win/desktop/win32_window.hpp>

namespace win32
{
    struct list_box : window
    {
        constexpr static auto class_name = WC_LISTBOXW;
        constexpr static std::uint16_t dialog_id = 0x0083;

        [[nodiscard]] inline lresult_t GetCount()
        {
            return SendMessageW(self, LB_GETCOUNT, 0, 0);
        }

        [[maybe_unused]] inline wparam_t AddString(wparam_t index, std::wstring_view text)
        {
            return SendMessageW(self, LB_ADDSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }

        [[maybe_unused]] inline wparam_t InsertString(wparam_t index, std::wstring_view text)
        {
            return SendMessageW(self, LB_INSERTSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }
    };
}

#endif