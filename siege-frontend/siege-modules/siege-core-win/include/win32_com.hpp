#ifndef WIN32_COM_HPP
#define WIN32_COM_HPP

#include <memory>
#include <combaseapi.h>

namespace win32::com
{
	template<typename T>
	auto as_unique(T* value, void(*deleter)(T*))
	{
		return std::unique_ptr<T, void(*)(T*)>(value, deleter);
	}

	template<typename T>
	auto as_unique(T* value)
	{
		return std::unique_ptr<T, void(*)(T*)>(value, [](auto* self) { 
			if (self) 
			{
				self->Release();
			}
		});
	}

	HRESULT init_com()
	{
		thread_local HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		thread_local auto com_handle = as_unique<HRESULT>(&result, [](auto*){ CoUninitialize(); });
		return result;
	}
}

#endif