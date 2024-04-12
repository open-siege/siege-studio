#ifndef WIN32_COM_VARIANT_HPP
#define WIN32_COM_VARIANT_HPP

#include <string_view>
#include <string>
#include <memory>
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

        Variant(const std::wstring& other) : Variant()
        {
            this->vt = VT_BSTR;
            this->bstrVal = ::SysAllocStringLen(other.data(), other.size());
        }

        operator std::wstring() const
        {
            if (this->vt == VT_BSTR && this->bstrVal)
            {
                return std::wstring(this->bstrVal, ::SysStringLen(this->bstrVal));
            }

            if ((this->vt & VT_BSTR && this->vt & VT_BYREF) && this->pbstrVal)
            {
                return std::wstring(*this->pbstrVal, ::SysStringLen(*this->pbstrVal));
            }

            return L"";
        }

        Variant(std::wstring_view other) : Variant()
        {
            this->vt = VT_BSTR;
            this->bstrVal = ::SysAllocStringLen(other.data(), other.size());
        }

        operator std::wstring_view() const
        {
            if (this->vt == VT_BSTR && this->bstrVal)
            {
                return std::wstring_view(this->bstrVal, ::SysStringLen(this->bstrVal));
            }

            if ((this->vt & VT_BSTR && this->vt & VT_BYREF) && this->pbstrVal)
            {
                return std::wstring_view(*this->pbstrVal, ::SysStringLen(*this->pbstrVal));
            }

            return L"";
        }

        Variant(IUnknown& object)
        {
            this->vt = VT_UNKNOWN;
            this->punkVal = &object;
            object.AddRef();
        }

        Variant(std::unique_ptr<IUnknown, void(*)(IUnknown*)>& object) : Variant(*object)
        {
        }

        operator std::unique_ptr<IUnknown, void(*)(IUnknown*)>() const
        {
            std::unique_ptr<IUnknown, void(*)(IUnknown*)> temp(nullptr, [](IUnknown* self) { self->Release(); });

            if (this->vt == VT_UNKNOWN && this->punkVal)
            {
                temp.reset(this->punkVal);
                temp->AddRef();
                return temp;
            }

            if ((this->vt & VT_UNKNOWN && this->vt & VT_BYREF) && this->ppunkVal)
            {
                temp.reset(*this->ppunkVal);
                temp->AddRef();
                return temp;
            }

            return temp;
        }

        Variant(IDispatch& object)
        {
            this->vt = VT_DISPATCH;
            this->pdispVal = &object;
            object.AddRef();
        }

        Variant(std::unique_ptr<IDispatch, void(*)(IDispatch*)>& object) : Variant(*object)
        {
        }

        operator std::unique_ptr<IDispatch, void(*)(IDispatch*)>() const
        {
            std::unique_ptr<IDispatch, void(*)(IDispatch*)> temp(nullptr, [](IDispatch* self) { self->Release(); });

            if (this->vt == VT_DISPATCH && this->pdispVal)
            {
                temp.reset(this->pdispVal);
                temp->AddRef();
                return temp;
            }

            if ((this->vt & VT_DISPATCH && this->vt & VT_BYREF) && this->ppdispVal)
            {
                temp.reset(*this->ppdispVal);
                temp->AddRef();
                return temp;
            }

            return temp;
        }

        Variant(std::unique_ptr<IStream, void(*)(IStream*)>& object) : Variant(*object)
        {

        }

        operator std::unique_ptr<IStream, void(*)(IStream*)>() const
        {
            std::unique_ptr<IStream, void(*)(IStream*)> temp(nullptr, [](IStream* self) { self->Release(); });

            if (this->vt == VT_UNKNOWN && this->punkVal)
            {
                IStream* raw = nullptr;
                if (this->punkVal->QueryInterface<IStream>(&raw) == S_OK)
                {
                    temp.reset(raw);
                }

                return temp;
            }

            if ((this->vt & VT_UNKNOWN && this->vt & VT_BYREF) && this->ppunkVal)
            {
                IStream* raw = nullptr;

                if ((*this->ppunkVal)->QueryInterface<IStream>(&raw) == S_OK)
                {
                    temp.reset(raw);
                }

                return temp;
            }

            return temp;
        }
    };
}

#endif