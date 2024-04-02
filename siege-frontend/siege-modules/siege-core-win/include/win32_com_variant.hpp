#ifndef WIN32_COM_VARIANT_HPP
#define WIN32_COM_VARIANT_HPP

#include <oaidl.h>

namespace win32::com
{
    struct Variant : ::VARIANT
    {
        Variant() noexcept
        {
            ::VariantInit(this);
        }

        ~Variant() noexcept
        {
            assert(::VariantClear(this) == S_OK);
        }
    };
}
}

#endif