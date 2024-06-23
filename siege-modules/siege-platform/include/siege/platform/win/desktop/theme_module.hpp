#ifndef OPEN_SIEGE_THEME_MODULE_HPP
#define OPEN_SIEGE_THEME_MODULE_HPP

#include <siege/platform/win/core/module.hpp>
#include <filesystem>
#include <span>
#include <uxtheme.h>

namespace win32
{
	struct theme_module : private win32::module
	{		
		theme_module() : win32::module("uxtheme.dll", true)
		{
			set_window_theme = this->GetProcAddress<decltype(set_window_theme)>("SetWindowTheme");
		
			if (set_window_theme == nullptr)
			{
				throw std::exception("Could not load theme window functions");
			}
		}

		HRESULT SetWindowTheme(HWND window, const wchar_t* app_name, const wchar_t* id)
		{
			return set_window_theme(window, app_name, id);
		}

	private:
		std::add_pointer_t<decltype(::SetWindowTheme)> set_window_theme;
	};
}

#endif