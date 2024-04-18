#ifndef WIN32_HEADER_HPP
#define WIN32_HEADER_HPP

#include <siege/platform/win/gaming/win32_user32.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct header
    {
        constexpr static auto class_name = WC_HEADERW;

        [[nodiscard]] static wparam_t GetItemCount(hwnd_t self)
        {
            return SendMessageW(self, HDM_GETITEMCOUNT, 0, 0);
        }

        [[maybe_unused]] wparam_t InsertItem(hwnd_t self, wparam_t index, HDITEMW info)
        {
            return SendMessageW(self, HDM_INSERTITEMW, index, std::bit_cast<win32::lparam_t>(&info));
        }
    };
}

#endif