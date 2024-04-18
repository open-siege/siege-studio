#ifndef WIN32_SCROLL_BAR_HPP
#define WIN32_SCROLL_BAR_HPP

#include <siege/platform/win/gaming/win32_user32.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct scroll_bar
    {
        constexpr static auto class_name = WC_SCROLLBARW;
        constexpr static std::uint16_t dialog_id = 0x0084;
    };

    struct page_scroller
    {
        constexpr static auto class_name = WC_PAGESCROLLERW;
    };
}

#endif