#ifndef WIN32_STATIC_HPP
#define WIN32_STATIC_HPP

#include <siege/platform/win/desktop/win32_window.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct static_control : window
    {
        using window::window;
        constexpr static auto class_name = WC_STATICW;
        constexpr static std::uint16_t dialog_id = 0x0082;
    };

    struct native_font : window
    {
        using window::window;
        constexpr static auto class_name = WC_NATIVEFONTCTLW;
    };
}

#endif