#ifndef WIN32_BUTTON_HPP
#define WIN32_BUTTON_HPP

#include "win32_user32.hpp"
#include "CommCtrl.h"

namespace win32
{
    struct button
    {
        constexpr static auto class_name = WC_BUTTONW;
        constexpr static std::uint16_t dialog_id = 0x0080;

        [[nodiscard]] static std::optional<RECT> GetTextMargin(hwnd_t self)
        {
            RECT result;

            if (SendMessageW(self, BCM_GETTEXTMARGIN, 0, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        [[nodiscard]] static std::optional<SIZE> GetIdealSize(hwnd_t self, LONG ideal_width = 0)
        {
            SIZE result{.cx = ideal_width };

            if (SendMessageW(self, BCM_GETIDEALSIZE, 0, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }
    };

    struct link
    {
        constexpr static auto class_name = WC_LINK;
    };
}

#endif