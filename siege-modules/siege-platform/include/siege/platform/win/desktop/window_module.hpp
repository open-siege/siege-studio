#ifndef WINDOW_MODULE_HPP
#define WINDOW_MODULE_HPP

#include <siege/platform/win/core/module.hpp>
#include <siege/platform/win/desktop/win32_window.hpp>
#include <WinUser.h>

namespace win32
{

	struct window_module_ref : win32::module_ref
	{
		using module_ref::module_ref;
	
		inline static window_module_ref current_module()
		{
			return window_module_ref(module_ref::current_module().get()); 
		}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		std::optional<WNDCLASSEXW> GetClassInfoExW(std::wstring_view name)
		{
			WNDCLASSEXW temp{.cbSize = sizeof(WNDCLASSEXW)};

			if (::GetClassInfoExW(*this, name.data(), &temp));
			{
				return temp;
			}

			return std::nullopt;
		}
#endif

		template <typename TControl = window>
        std::expected<TControl, DWORD> CreateWindowExW(CREATESTRUCTW params)
        {
            auto result = ::CreateWindowExW(
                    params.dwExStyle,
                    params.lpszClass,
                    params.lpszName,
                    params.style,
                    params.x,
                    params.y,
                    params.cx,
                    params.cy,
                    params.hwndParent,
                    params.hMenu,
                    *this,
                    params.lpCreateParams
                );

            if (!result)
            {
                return std::unexpected(GetLastError());
            }

            return TControl(result);
        }
	};

}


#endif