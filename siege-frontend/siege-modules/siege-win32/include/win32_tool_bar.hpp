#ifndef WIN32_TOOL_BAR_HPP
#define WIN32_TOOL_BAR_HPP

#include "win32_user32.hpp"
#include "CommCtrl.h"

namespace win32
{
    struct tool_bar
    {
        constexpr static auto class_name = TOOLBARCLASSNAMEW;

        enum extended_style : DWORD
        {
            mixed_buttons = TBSTYLE_EX_MIXEDBUTTONS,
            draw_drop_down_arrows = TBSTYLE_EX_DRAWDDARROWS
        };

        static void AutoSize(hwnd_t self)
        {
            SendMessageW(self, TB_AUTOSIZE, 0, 0);
        }

        [[maybe_unused]] static bool SetButtonWidth(hwnd_t self, std::array<int, 2> range)
        {
            auto [min, max] = range;

            return SendMessageW(self, TB_SETBUTTONWIDTH, 0, MAKELPARAM(min, max));
        }

        [[maybe_unused]] static bool SetButtonSize(hwnd_t self, SIZE size)
        {
            return SendMessageW(self, TB_SETBUTTONSIZE , 0, MAKELPARAM(size.cx, size.cy));
        }

        [[nodiscard]] static SIZE GetButtonSize(hwnd_t self)
        {
            auto result = SendMessageW(self, TB_GETBUTTONSIZE, 0, 0);

            return SIZE {.cx = LOWORD(result), .cy = HIWORD(result)};
        }

        [[nodiscard]] static std::optional<RECT> GetRect(hwnd_t self, wparam_t id)
        {
            RECT result;
            if (SendMessageW(self, TB_GETRECT, id, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        [[maybe_unused]] static extended_style SetExtendedStyle(hwnd_t self, lparam_t style)
        {
            return extended_style(SendMessageW(self, 
                TB_SETEXTENDEDSTYLE, 0, style));
        
        }

        [[maybe_unused]] static bool InsertButton(hwnd_t self, wparam_t index, TBBUTTON button)
        {
            SendMessageW(self, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
            return SendMessageW(self, TB_INSERTBUTTONW, index, 
                std::bit_cast<win32::lparam_t>(&button));
        }

        [[maybe_unused]] static bool AddButtons(hwnd_t self, std::span<TBBUTTON> buttons)
        {
            SendMessageW(self, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
            return SendMessageW(self, TB_ADDBUTTONSW, wparam_t(buttons.size()), 
                std::bit_cast<win32::lparam_t>(buttons.data()));
        }

        static std::span<TBBUTTON> GetChildButtons(hwnd_t self, std::span<TBBUTTON> buttons)
        {
            return buttons;
        }
    };
}

#endif