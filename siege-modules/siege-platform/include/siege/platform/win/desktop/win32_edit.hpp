#ifndef WIN32_EDIT_HPP
#define WIN32_EDIT_HPP

#include <siege/platform/win/desktop/win32_window.hpp>

namespace win32
{
    struct edit : window
    {
        using window::window;
        constexpr static auto class_name = WC_EDITW;
        constexpr static std::uint16_t dialog_id = 0x0081;

       [[maybe_unused]] inline bool SetCueBanner(bool show_on_focus, std::wstring text)
       {
          return SendMessageW(*this, EM_SETCUEBANNER, show_on_focus ? TRUE : FALSE, std::bit_cast<lparam_t>(text.c_str()));
       }
    };

    struct ip_address
    {
        constexpr static auto class_name = WC_IPADDRESSW;
    };
}

#endif