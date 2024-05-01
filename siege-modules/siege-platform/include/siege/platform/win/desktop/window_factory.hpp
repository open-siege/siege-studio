#ifndef WINDOW_FACTORY_HPP
#define WINDOW_FACTORY_HPP

#include <vector>
#include <siege/platform/win/desktop/window_module.hpp>


namespace win32
{
	struct window_factory
	{
		std::vector<window_module_ref> modules;

        window_factory()
        {
            modules.emplace_back(win32::window_module_ref::current_module());
            modules.emplace_back(::GetModuleHandleW(L"comctrl32.dll"));
            modules.emplace_back(::GetModuleHandleW(L"siege-win-mfc.dll"));
        }

		template <typename TControl = window>
        std::expected<TControl, DWORD> CreateWindowExW(CREATESTRUCTW params)
        {
            thread_local WNDCLASSEXW info{sizeof(WNDCLASSEXW)};

            for (auto& module : modules)
            {
                if (module && ::GetClassInfoExW(module, params.lpszClass, &info))
                {
                    params.hInstance = module;
                    break;
                }   
            }

            return window_module_ref(params.hInstance).CreateWindowExW<TControl>(params);
        }
	};
}

#endif