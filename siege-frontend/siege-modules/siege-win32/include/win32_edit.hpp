#ifndef WIN32_EDIT_HPP
#define WIN32_EDIT_HPP

#include "win32_user32.hpp"

namespace win32
{
    struct edit
    {
        constexpr static auto class_name = WC_EDITW;
        constexpr static std::uint16_t dialog_id = 0x0081;

       [[maybe_unused]] static bool SetCueBanner(hwnd_t self, bool show_on_focus, std::wstring text)
       {
          return SendMessageW(self, EM_SETCUEBANNER, show_on_focus ? TRUE : FALSE, std::bit_cast<lparam_t>(text.c_str()));
       }
    };

    struct ip_address
    {
        constexpr static auto class_name = WC_IPADDRESSW;
    };
}

#endif