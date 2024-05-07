#ifndef WIN32_TRACK_BAR_HPP
#define WIN32_TRACK_BAR_HPP

#include <siege/platform/win/desktop/win32_window.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct track_bar : window
    {
        using window::window;
        constexpr static auto class_name = TRACKBAR_CLASSW;
    };
}

#endif