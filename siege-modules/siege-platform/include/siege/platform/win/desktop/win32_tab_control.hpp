#ifndef WIN32_TAB_CONTROL_HPP
#define WIN32_TAB_CONTROL_HPP

#include <siege/platform/win/desktop/win32_window.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct tab_control : window
    {
        using window::window;
        constexpr static auto class_name = WC_TABCONTROLW;

        [[maybe_unused]] inline wparam_t InsertItem(wparam_t index, TCITEMW info)
        {
            return SendMessageW(self, TCM_INSERTITEMW, index, std::bit_cast<lparam_t>(&info));
        }

        [[nodiscard]] inline wparam_t GetItemCount()
        {
            return SendMessageW(self, TCM_GETITEMCOUNT, 0, 0);
        }

        [[nodiscard]] inline std::optional<TCITEMW> GetItem(wparam_t index, std::uint32_t mask = TCIF_PARAM | TCIF_STATE)
        {
            TCITEMW result { .mask = mask };

            if (SendMessageW(self, TCM_GETITEM, index, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        inline RECT AdjustRect(bool dispay_to_window, RECT rect)
        {
            SendMessageW(self, TCM_ADJUSTRECT, dispay_to_window ? TRUE : FALSE, std::bit_cast<lparam_t>(&rect));
            return rect;
        }
    };
}

#endif