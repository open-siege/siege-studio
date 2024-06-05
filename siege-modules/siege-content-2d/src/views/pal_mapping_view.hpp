#ifndef DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>

namespace siege::views
{
    struct pal_mapping_view : win32::window_ref
    {
        constexpr static auto formats = std::array<std::wstring_view, 1>{{L"palettes.settings.json"}};
    
        pal_mapping_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
	    {
	    }

        auto on_create(const win32::create_message&)
        {
            return 0;
        }

        auto on_size(win32::size_message sized)
	    {
		    return std::nullopt;
	    }
    };
}

#endif//DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP
