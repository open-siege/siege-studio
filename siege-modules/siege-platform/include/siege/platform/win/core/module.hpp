#ifndef WIN_CORE_MODULE_HPP
#define WIN_CORE_MODULE_HPP

#include <SDKDDKVer.h>
#include <system_error>
#include <optional>
#include <filesystem>
#include <siege/platform/win/auto_handle.hpp>
#include <wtypes.h>
#include <WinDef.h>
#include <libloaderapi.h>
#undef GetModuleFileName

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

		template<typename TChar = wchar_t>
		std::basic_string<TChar> GetModuleFileName()
		{
			if constexpr (sizeof(TChar) == sizeof(char16_t))
			{
				std::basic_string<TChar> result(256, '\0');
                ::GetModuleFileNameW(*this, reinterpret_cast<wchar_t*>(result.data()), result.size());
                
				result.erase(result.find(TChar('\0')));
				return result;
			}
			else if constexpr (sizeof(TChar) == sizeof(char8_t))
			{
                std::basic_string<TChar> result(256, '\0');
                ::GetModuleFileNameA(*this, reinterpret_cast<char*>(result.data()), result.size());

				result.erase(result.find(TChar('\0')));
                return result;
			}

			return std::basic_string<TChar>{};
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
