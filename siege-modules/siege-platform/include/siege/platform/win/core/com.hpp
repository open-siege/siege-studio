#ifndef WIN32_COM_HPP
#define WIN32_COM_HPP

#include <memory>
#include <optional>
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

	struct ComObject : IUnknown
    {
        std::atomic_uint refCount = 1;

        static bool IsHeapAllocated(void* object, std::size_t);
        static void* operator new(std::size_t count);
        static void operator delete(void* ptr, std::size_t sz);

        [[maybe_unused]] ULONG __stdcall IUnknown::AddRef() noexcept override;
        [[maybe_unused]] ULONG __stdcall IUnknown::Release() noexcept override;
    };

    template<typename TBase, typename TInterface, typename TObject>
    std::optional<HRESULT> ComQuery(TObject& self, const GUID& riid, void** ppvObj)
    {
        if (IsEqualGUID(riid, __uuidof(TBase)))
        {
            self.AddRef();
            *ppvObj = static_cast<TInterface*>(&self);
            return S_OK;
        }

        return std::nullopt;
    }

    template<typename TInterface, typename TObject>
    std::optional<HRESULT> ComQuery(TObject& self, const GUID& riid, void** ppvObj)
    {
        if (IsEqualGUID(riid, __uuidof(TInterface)))
        {
            self.AddRef();
            *ppvObj = static_cast<TInterface*>(&self);
            return S_OK;
        }

        return std::nullopt;
    }

	HRESULT init_com();
}

#endif