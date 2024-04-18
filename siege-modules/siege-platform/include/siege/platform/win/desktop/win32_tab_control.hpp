#ifndef WIN32_TAB_CONTROL_HPP
#define WIN32_TAB_CONTROL_HPP

#include <siege/platform/win/gaming/win32_user32.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct tab_control
    {
        constexpr static auto class_name = WC_TABCONTROLW;

        [[maybe_unused]] static wparam_t InsertItem(hwnd_t self, wparam_t index, TCITEMW info)
        {
            return SendMessageW(self, TCM_INSERTITEMW, index, std::bit_cast<lparam_t>(&info));
        }

        [[nodiscard]] static wparam_t GetItemCount(hwnd_t self)
        {
            return SendMessageW(self, TCM_GETITEMCOUNT, 0, 0);
        }

        [[nodiscard]] static std::optional<TCITEMW> GetItem(hwnd_t self, wparam_t index, std::uint32_t mask = TCIF_PARAM | TCIF_STATE)
        {
            TCITEMW result { .mask = mask };

            if (SendMessageW(self, TCM_GETITEM, index, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        static RECT AdjustRect(hwnd_t self, bool dispay_to_window, RECT rect)
        {
            SendMessageW(self, TCM_ADJUSTRECT, dispay_to_window ? TRUE : FALSE, std::bit_cast<lparam_t>(&rect));
            return rect;
        }
    };
}

#endif