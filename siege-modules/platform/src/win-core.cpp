#include <siege/platform/win/com.hpp>
#include <siege/platform/win/module.hpp>
#undef NDEBUG 
#include <cassert>
#include <set>
#include <comcat.h>

namespace win32::com
{
	static_assert(sizeof(std::uint32_t) == sizeof(ULONG));
	static_assert(sizeof(std::uint32_t) == sizeof(LCID));
	static_assert(sizeof(char16_t) == sizeof(wchar_t));

	HRESULT init_com(COINIT apartment_model)
	{
		thread_local HRESULT result = CoInitializeEx(nullptr, apartment_model);
		thread_local auto com_handle = std::unique_ptr<HRESULT, void(*)(HRESULT*)>(&result, [](auto*) { CoUninitialize(); });

		return result;
	}
}
