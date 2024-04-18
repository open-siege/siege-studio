#ifndef WIN32_COM_PROP_VARIANT_HPP
#define WIN32_COM_PROP_VARIANT_HPP

#include <expected>
#include <memory>
#include <propsys.h>
#include <siege/platform/win/core/com.hpp>

namespace win32::com
{
    template <typename TCollection>
    std::expected<std::unique_ptr<TCollection, void(*)(TCollection*)>, HRESULT> PSCreateMemoryPropertyStore()
    {
        TCollection* result = nullptr;
        auto hresult = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&result));

        if (hresult != S_OK)
        {
            return std::unexpected(hresult);
        }

        return as_unique<TCollection>(result);
    }

    struct PropVariant : ::PROPVARIANT
    {
        PropVariant() noexcept
            : ::PROPVARIANT{}
        {
        }

        PropVariant(const PropVariant& other)
        {
            assert(::PropVariantCopy(this, &other) == S_OK);
        }

        ~PropVariant() noexcept
        {
            assert(::PropVariantClear(this) == S_OK);
        }
    };
}

#endif