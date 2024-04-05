#ifndef WIN32_COM_HPP
#define WIN32_COM_HPP

#include <memory>
#include <combaseapi.h>

namespace win32::com
{
	template <typename T>
	constexpr auto get_com_deleter()
	{
		return [](T* value) {
			if constexpr(std::is_same_v<IUnknown, T> || std::is_base_of_v<IUnknown, T>)
            {
				if (value)
				{
					value->Release();
				}
            }
		};
	}

	template<typename T>
	constexpr auto as_unique(T* value)
	{
		return std::unique_ptr<T, decltype(get_com_deleter<T>())>(value, get_com_deleter<T>());
	}

	template<typename T>
	auto as_unique(T* value, void(*deleter)(T*))
	{
		return std::unique_ptr<T, void(*)(T*)>(value, deleter);
	}

	HRESULT init_com();
}

#endif