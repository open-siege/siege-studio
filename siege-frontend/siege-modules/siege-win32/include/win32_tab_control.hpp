#ifndef WIN32_TAB_CONTROL_HPP
#define WIN32_TAB_CONTROL_HPP

#include "win32_user32.hpp"
#include "CommCtrl.h"

namespace win32
{
    struct tab_control
    {
        constexpr static auto class_name = WC_TABCONTROLW;

        [[maybe_unused]] static wparam_t InsertItem(hwnd_t self, wparam_t index, TCITEMW info)
        {
            return SendMessageW(self, TCM_INSERTITEMW, index, std::bit_cast<lparam_t>(&info));
        }

        static RECT AdjustRect(hwnd_t self, bool dispay_to_window, RECT rect)
        {
            SendMessageW(self, TCM_ADJUSTRECT, dispay_to_window ? TRUE : FALSE, std::bit_cast<lparam_t>(&rect));
            return rect;
        }
    };
}

#endif