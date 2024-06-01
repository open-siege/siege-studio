#ifndef WIN32_COM_PROP_VARIANT_HPP
#define WIN32_COM_PROP_VARIANT_HPP

#include <expected>
#include <memory>
#include <propsys.h>
#include <siege/platform/win/core/com/base.hpp>

namespace win32::com
{
    template <typename TCollection>
    std::expected<std::unique_ptr<TCollection, com_deleter<TCollection>>, HRESULT> PSCreateMemoryPropertyStore()
    {
        com_ptr<TCollection> result;
        auto hresult = ::PSCreateMemoryPropertyStore(__uuidof(TCollection), result.put_void());

        if (hresult != S_OK)
        {
            return std::unexpected(hresult);
        }

        return result;
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