#ifndef WIN32_COM_PROP_VARIANT_HPP
#define WIN32_COM_PROP_VARIANT_HPP

#include <expected>
#include <memory>
#include <propsys.h>

namespace win32::com
{
    template <typename TCollection>
    std::unique_ptr<TCollection, void(*)(TCollection*)> PSCreateMemoryPropertyStore()
    {
        TCollection* result = nullptr;
        auto hresult = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&result));

        if (hresult != S_OK)
        {
            return std::unexpected(hresult);
        }

        return std::unique_ptr<TCollection, void(*)(TCollection*)>(result, [](auto* object) {
            assert(object->Release() >= 0);
        });
    }

    struct PropVariant : ::PROPVARIANT
    {
        PropVariant() noexcept
            : ::PROPVARIANT{}
        {
        }
        ~PropVariant() noexcept
        {
            assert(::PropVariantClear(this) == S_OK);
        }
    };
}

#endif