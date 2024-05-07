#ifndef WIN_CORE_MODULE_HPP
#define WIN_CORE_MODULE_HPP

#include <system_error>
#include <optional>
#include <siege/platform/win/auto_handle.hpp>
#include <wtypes.h>
#include <WinDef.h>
#include <libloaderapi.h>

namespace win32
{
	struct module_deleter
	{
		void operator()(HMODULE lib)
		{
			FreeLibrary(lib);
		}
	};

	struct module_no_deleter
	{
		void operator()(HMODULE lib)
		{
		}
	};

	struct module_ref : win32::auto_handle<HMODULE, module_no_deleter>
	{
		using base = win32::auto_handle<HMODULE, module_no_deleter>;
		using base::base;

		operator HMODULE() const
		{
			return get();
		}

		inline static module_ref current_module()
		{
			return module_ref(&current_module); 
		}

		explicit module_ref(void* func) : base([=]() {
				HMODULE temp = nullptr;

				::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
               (LPCWSTR)func, &temp);

				if (!temp)
				{
					throw std::system_error(std::error_code(GetLastError(),  std::system_category()));
				}

				return temp;
			}())
		{
		}

	};

	struct module : win32::auto_handle<HMODULE, module_deleter>
	{
		using base = win32::auto_handle<HMODULE, module_deleter>;
		using base::base;
	};
}

#endif // !WIN_CORE_MODULE_HPP
