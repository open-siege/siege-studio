#ifndef WIN32_COM_ENUMERABLE_HPP
#define WIN32_COM_ENUMERABLE_HPP

#include <expected>
#include <limits>
#include <string>
#include <memory>
#include <optional>
#include "win32_com.hpp"
#include "win32_com_variant.hpp"

namespace win32::com
{   
    void CopyMember(const VARIANT& value, VARIANT& other)
    {
        VariantCopy(&other, &value);
    }

    void CopyMember(IUnknown* value, IUnknown*&other)
    {
        if (!value)
        {
            other = nullptr;
            return;
        }

        value->AddRef();
        other = value;
    }

    template <typename TValue, typename TEnumerator, typename TEnumeratorContainer = std::unique_ptr<TEnumerator, void(*)(TEnumerator*)>> 
    struct EnumeratorIterator
    {
        TEnumeratorContainer enumerator;
        std::size_t position = 0;
        TValue temp;

        EnumeratorIterator(TEnumeratorContainer container, std::size_t position = 0) : enumerator(std::move(container)), position(position), temp{}
        {
        
        }

        EnumeratorIterator(const EnumeratorIterator& other) : enumerator(nullptr, [](TEnumerator* self) {
               if (self)
               {
                    self->Release();
               }
            }), position{other.position}, temp{}
        {
            IEnumVARIANT* temp;

            if (other.enumerator && other.enumerator->Clone(&temp) == S_OK)
            {
                enumerator.reset(temp);            
            }

            CopyMember(other.temp, temp);
        }

        EnumeratorIterator& operator++()
        {
            enumerator->Next(1, &temp, nullptr);

            position++;
            return *this;
        }

        EnumeratorIterator operator++(int)
        {
            EnumeratorIterator old = *this;
            operator++();
            return old; 
        }

        TValue& operator*()
        {
            return temp;
        }

        TValue* operator->()
        {
            return &temp;
        }

        friend inline bool operator< (const EnumeratorIterator& lhs, const EnumeratorIterator& rhs) { return lhs.position < rhs.position; }
        friend inline bool operator==(const EnumeratorIterator& lhs, const EnumeratorIterator& rhs) { return lhs.position == rhs.position; }

        friend inline bool operator> (const EnumeratorIterator& lhs, const EnumeratorIterator& rhs) { return rhs < lhs; }
        friend inline bool operator<=(const EnumeratorIterator& lhs, const EnumeratorIterator& rhs) { return !(lhs > rhs); }
        friend inline bool operator>=(const EnumeratorIterator& lhs, const EnumeratorIterator& rhs) { return !(lhs < rhs); }

        friend inline bool operator!=(const EnumeratorIterator& lhs, const EnumeratorIterator& rhs) { return !(lhs == rhs); }
    };

    struct IEnumerable : IDispatch
    {
        std::expected<DISPID, HRESULT> GetDispId(std::wstring& value)
        {
            wchar_t* data = value.data();

            DISPID id = 0;

            auto result = this->GetIDsOfNames(IID_NULL, &data, 1, LOCALE_USER_DEFAULT,  &id);
            
            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            return id;
        }

        template<typename IEnum = IEnumVARIANT>
        std::expected<std::unique_ptr<IEnumVARIANT, void(*)(IEnum*)>, HRESULT> NewEnum()  noexcept
        {
            DISPPARAMS dp = {nullptr, nullptr, 0, 0};
            Variant result;
            auto hresult = Invoke(DISPID_NEWENUM, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dp, &result, nullptr, nullptr);
            
            auto temp = std::unique_ptr<IEnum, void(*)(IEnum*)>(nullptr, [](auto* self) {
                    if (self)
                    {
                        self->Release();
                    }
                });

            if (hresult == S_OK && (result.vt == VT_DISPATCH || result.vt == VT_UNKNOWN))
            {
                IEnum* self = nullptr;

                hresult = result.punkVal->QueryInterface(__uuidof(IEnum), (void**)&self);

                if (hresult != S_OK)
                {
                    return std::unexpected(hresult);
                }

                temp.reset(self);
                return temp;
            }

            return std::unexpected(hresult);
        }

        template<typename IEnum = IEnumVARIANT>
        auto begin()
        {
            auto newEnum = NewEnum<IEnum>();

            return EnumeratorIterator<Variant, IEnum, decltype(newEnum)::value_type>(std::move(*newEnum));
        }

        template<typename IEnum = IEnumVARIANT>
        auto end()
        {
            auto newEnum = NewEnum<IEnum>();
            constexpr static auto max = std::numeric_limits<ULONG>::max();

            ULONG count = 0u;

            for (count = 0u; count < max; ++count)
            {
                if ((*newEnum)->Skip(1) == S_FALSE)
                {
                    break;
                }
            }

            return EnumeratorIterator<Variant, IEnum, decltype(newEnum)::value_type>(std::move(*newEnum), count);
        }
    };
}

#endif