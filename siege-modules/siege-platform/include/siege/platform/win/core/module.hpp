#ifndef WIN_CORE_MODULE_HPP
#define WIN_CORE_MODULE_HPP

#include <system_error>
#include <optional>
#include <filesystem>
#include <siege/platform/win/auto_handle.hpp>
#include <wtypes.h>
#include <WinDef.h>
#include <libloaderapi.h>

namespace win32
{
	template<typename TDeleter>
	struct module_base : win32::auto_handle<HMODULE, TDeleter>
	{
		using base = win32::auto_handle<HMODULE, TDeleter>;
		using base::base;

		template<typename TPointer = void*>
		auto GetProcAddress(std::string name)
		{
			static_assert(std::is_pointer_v<TPointer>);
			return reinterpret_cast<TPointer>(::GetProcAddress(*this, name.c_str()));
		}

		auto GetModuleFileNameW()
		{
			std::wstring result(256, L'\0');
			::GetModuleFileNameW(*this, result.data(), result.size());
			return result;
		}
	};

	struct module_no_deleter
	{
		void operator()(HMODULE lib)
		{
		}
	};

	struct module_ref : module_base<module_no_deleter>
	{
		using base = module_base<module_no_deleter>;
		using base::base;

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

		inline static module_ref current_module()
		{
			return module_ref((void*)&current_module); 
		}

		inline static module_ref current_application()
		{
			return module_ref(::GetModuleHandleW(nullptr));
		}
	};

	struct module_deleter
	{
		void operator()(HMODULE lib)
		{
			FreeLibrary(lib);
		}
	};

	struct module : module_base<module_deleter>
	{
		using base = module_base<module_deleter>;
		using base::base;

		explicit module(std::filesystem::path path, bool is_system = false) : base([=]() {
				if (!is_system && !std::filesystem::exists(path))
				{
					throw std::invalid_argument("path");
				}

				HMODULE temp = ::LoadLibraryW(path.c_str());

				if (!temp)
				{
					throw std::system_error(std::error_code(GetLastError(),  std::system_category()));
				}

				return temp;
			}())
		{
		}

		auto ref()
		{
			return module_ref(get());
		}
	};
}

#endif // !WIN_CORE_MODULE_HPP
