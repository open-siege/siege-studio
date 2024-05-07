#ifndef WIN32_SCROLL_BAR_HPP
#define WIN32_SCROLL_BAR_HPP

#include <siege/platform/win/desktop/win32_window.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct scroll_bar : window
    {
        using window::window;
        constexpr static auto class_name = WC_SCROLLBARW;
        constexpr static std::uint16_t dialog_id = 0x0084;
    };

    struct page_scroller : window
    {
        using window::window;
        constexpr static auto class_name = WC_PAGESCROLLERW;
    };
}

#endif