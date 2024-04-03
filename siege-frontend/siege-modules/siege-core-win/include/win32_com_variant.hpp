#ifndef WIN32_COM_VARIANT_HPP
#define WIN32_COM_VARIANT_HPP

#include <oaidl.h>
#include <cassert>

namespace win32::com
{
    struct Variant : ::VARIANT
    {
        Variant() noexcept
        {
            ::VariantInit(this);
        }

        Variant(const ::VARIANT& other) : Variant()
        {
            assert(::VariantCopy(this, &other) == S_OK);
        }

        Variant(const Variant& other) : Variant()
        {
            assert(::VariantCopy(this, &other) == S_OK);
        }

        ~Variant() noexcept
        {
            assert(::VariantClear(this) == S_OK);
        }
    };
}

#endif