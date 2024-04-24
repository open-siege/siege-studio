#ifndef WIN32_HEADER_HPP
#define WIN32_HEADER_HPP

#include <siege/platform/win/desktop/win32_window.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct header : window
    {
        using window::window;
        constexpr static auto class_name = WC_HEADERW;

        [[nodiscard]] inline wparam_t GetItemCount()
        {
            return SendMessageW(self, HDM_GETITEMCOUNT, 0, 0);
        }

        [[maybe_unused]] inline wparam_t InsertItem(wparam_t index, HDITEMW info)
        {
            return SendMessageW(self, HDM_INSERTITEMW, index, std::bit_cast<win32::lparam_t>(&info));
        }
    };
}

#endif