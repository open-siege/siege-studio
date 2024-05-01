#ifndef WIN_CORE_MODULE_HPP
#define WIN_CORE_MODULE_HPP

#include <system_error>
#include <siege/platform/win/auto_handle.hpp>
#include <libloaderapi.h>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <wtypes.h>
#include <WinDef.h>
#include <WinUser.h>
#endif

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

				GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
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

	};

	struct module : win32::auto_handle<HMODULE, module_deleter>
	{
		using base = win32::auto_handle<HMODULE, module_deleter>;
		using base::base;
	};

	
}

#endif // !WIN_CORE_MODULE_HPP
