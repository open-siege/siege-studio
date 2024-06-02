#ifndef WIN32_COM_HPP
#define WIN32_COM_HPP

#include <memory>
#include <optional>
#include <combaseapi.h>

namespace win32::com
{
    template<typename TUnknown>
    struct com_deleter
    {
        void operator()(TUnknown* self)
        {
            if (self)
            {
                self->Release();
            }
        }
    
    };

    template<typename TUnknown>
    struct com_ptr : std::unique_ptr<TUnknown, com_deleter<TUnknown>>
    {
        using base = std::unique_ptr<TUnknown, com_deleter<TUnknown>>;
        using base::base;

        com_ptr(TUnknown* value) : base(value)
        {
        }

        com_ptr(const com_ptr& other) : base([&]() -> base {
                other->AddRef();
                return base(other.get());
            }())
        {
        }

        template <typename TOther>
        com_ptr<TOther> as()
        {
            static_assert(std::is_same_v<TUnknown, TOther> || std::is_base_of_v<TUnknown, TOther> || std::is_base_of_v<TOther, TUnknown>);

            if (this->get())
            {
                this->get()->AddRef();
            }
            return com_ptr<TOther>(static_cast<TOther*>(this->get()));
        }

        TUnknown** put()
        {
            static_assert(sizeof(base) == sizeof(void*));
            static_assert(std::is_standard_layout_v<com_ptr>);
            return reinterpret_cast<TUnknown**>(this);
        }

        void** put_void()
        {
            static_assert(sizeof(base) == sizeof(void*));
            static_assert(std::is_standard_layout_v<com_ptr>);
            return reinterpret_cast<void**>(this);
        }
    };

	struct ComObject : IUnknown
    {
        std::atomic_uint refCount = 1;

        static bool IsHeapAllocated(void* object, std::size_t);
        static void* operator new(std::size_t count);
        static void operator delete(void* ptr, std::size_t sz);

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override;
        [[maybe_unused]] ULONG __stdcall Release() noexcept override;
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

	HRESULT init_com(COINIT apartment_model = COINIT_MULTITHREADED);
}

#endif