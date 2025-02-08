#ifndef WINDOW_FACTORY_HPP
#define WINDOW_FACTORY_HPP

#include <vector>
#include <siege/platform/win/window_module.hpp>

namespace win32
{
	struct window_factory
	{
		std::vector<window_module_ref> modules;

        win32::window_ref parent;


        window_factory()
        {
            modules.emplace_back(win32::window_module_ref::current_module());
            modules.emplace_back(::GetModuleHandleW(L"comctl32.dll"));
        }

        window_factory(win32::window_ref parent) : window_factory()
        {
            this->parent = std::move(parent);
        }


		template <typename TControl = window>
        std::expected<TControl, DWORD> CreateWindowExW(CREATESTRUCTW params)
        {
            thread_local WNDCLASSEXW info{sizeof(WNDCLASSEXW)};

            std::wstring class_name = params.lpszClass ? params.lpszClass : L"";

            if (class_name.empty())
            {
                class_name = window_class_name<TControl>();
                params.lpszClass = class_name.c_str();
            }

            auto module_iter = std::find_if(modules.begin(), modules.end(), [&](auto& module) {
                    return module && module.GetClassInfoExW(class_name).has_value();
            });

            if (module_iter != modules.end())
            {
                params.hInstance = *module_iter;
            }
            else if (module_iter == modules.end())
            {
                module_iter = std::find_if(modules.begin(), modules.end(), [&](auto& module) {
                    return module && module.GetClassInfoExW(window_class_name<TControl>());
                });
                
                if (module_iter != modules.end())
                {
                    class_name = window_class_name<TControl>();
                    params.lpszClass = class_name.c_str();
                    params.hInstance = *module_iter;
                }
            }

            if (!params.hwndParent && this->parent)
            {
                params.hwndParent = this->parent;
            }

            return window_module_ref(params.hInstance).CreateWindowExW<TControl>(params);
        }
	};
}

#endif