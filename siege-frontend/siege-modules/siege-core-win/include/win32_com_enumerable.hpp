#ifndef WIN32_COM_ENUMERABLE_HPP
#define WIN32_COM_ENUMERABLE_HPP

#include <expected>
#include <limits>
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include "win32_com.hpp"
#include "win32_com_variant.hpp"
#include <ocidl.h>
#include <olectl.h>

namespace win32::com
{   
    inline void CopyMember(const VARIANT& value, VARIANT& other)
    {
        other = value;
    }

    inline void CopyMember(IUnknown* value, IUnknown*&other)
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

    struct __declspec(uuid("00020400-0000-0000-C000-000000000046")) IEnumerable : IDispatch
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

        template<typename InterfaceType = IUnknown>
    struct Interface_EnumTraits
    {
        using ElemType = InterfaceType;
        using OutType = ElemType**;
    };

    template<typename StructType>
    struct Struct_EnumTraits
    {
        using ElemType = StructType;
        using OutType = ElemType*;
    };

    template<typename InterfaceType = IUnknown>
    struct IUnknown_EnumTraits : Interface_EnumTraits<InterfaceType>
    {
        using EnumType = IEnumUnknown;
        using OutType = IUnknown**;

        template <typename Iter>
        static void Copy(IUnknown **output, Iter iter)
        {
            (*iter)->QueryInterface<IUnknown>(output);
        }
    };

    struct IConnectionPoint_EnumTraits : Interface_EnumTraits<IConnectionPoint>
    {
        using EnumType = IEnumConnectionPoints;

        template <typename Iter>
        static void Copy(IConnectionPoint **output, Iter iter)
        {
            (*iter)->QueryInterface<IConnectionPoint>(output);
        }
    };
    
    template<typename TVariant = VARIANT>
    struct VARIANT_EnumTraits : Struct_EnumTraits<TVariant>
    {
        using EnumType = IEnumVARIANT;
        using OutType = VARIANT*;

        template <typename Iter>
        static void Copy(VARIANT *output, Iter iter)
        {
            VariantCopy(output, iter->get());
        }
    };

    template<typename TConnectData = CONNECTDATA>
    struct CONNECTDATA_EnumTraits : Struct_EnumTraits<TConnectData>
    {
        using EnumType = IEnumConnections;
        using OutType = CONNECTDATA*;

        template <typename Iter>
        static void Copy(CONNECTDATA *output, Iter iter)
        {
            output->dwCookie = iter->dwCookie;
            output->pUnk = iter->pUnk;
            output->pUnk->AddRef();
        }
    };

    struct String_EnumTraits
    {
        using EnumType = IEnumString;
        using ElemType = std::wstring;
        using OutType = wchar_t**;

        template <typename Iter>
        static void Copy(wchar_t **output, Iter iter)
        {
            *output = (wchar_t*)::CoTaskMemAlloc(iter->size() * sizeof(wchar_t));
            iter->copy(*output, iter->size());
        }
    };

    struct StringView_EnumTraits
    {
        using EnumType = IEnumString;
        using ElemType = std::wstring_view;
        using OutType = wchar_t**;

        template <typename Iter>
        static void Copy(wchar_t **output, Iter iter)
        {
            *output = (wchar_t*)::CoTaskMemAlloc(iter->size() * sizeof(wchar_t));
            iter->copy(*output, iter->size());
        }
    };

    template<typename TEnumTraits, typename TIter>
    struct RangeEnumerator : TEnumTraits::EnumType, ComObject
    {
        using ElemType = TEnumTraits::ElemType;
        using EnumType = TEnumTraits::EnumType;
        using OutType = TEnumTraits::OutType;

        IUnknown* owner;
        
        TIter begin;
        TIter current;
        TIter end;

        RangeEnumerator(TIter begin, TIter current, TIter end) : 
            owner(nullptr), 
            begin(begin), 
            current(current), 
            end(end)
        {
        }

        RangeEnumerator(IUnknown* owner, TIter begin, TIter current, TIter end) : 
            owner(owner), 
            begin(begin), 
            current(current), 
            end(end)
        {
        }

        void SetParent(IUnknown* owner)
        {
            this->owner = owner;
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            if (owner)
            {
                return owner->QueryInterface(riid, ppvObj);
            }

            return ComQuery<IUnknown, EnumType>(*this, riid, ppvObj)
                .or_else([&]() { return ComQuery<EnumType>(*this, riid, ppvObj); })
                .value_or(E_NOINTERFACE);
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            if (owner)
            {
                return refCount = owner->AddRef();
            }

            return ComObject::AddRef();
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            if (owner)
            {
                return refCount = owner->Release();
            }

            return ComObject::Release();
        }

        HRESULT __stdcall Clone(EnumType** other) noexcept override
        {
            auto temp = std::make_unique<RangeEnumerator<TEnumTraits, TIter>>(begin, current, end);
            *other = static_cast<EnumType*>(temp.release());

            return S_OK;
        }

        auto Clone()
        {
            auto* temp = new RangeEnumerator<TEnumTraits, TIter>(begin, current, end);
            return std::unique_ptr<
                RangeEnumerator<TEnumTraits, TIter>, 
                void(*)(RangeEnumerator<TEnumTraits, TIter>*)
            >
                (temp, [](auto* self) {
                        if (self)
                        {
                            delete self;
                        }
                    });
        }

        HRESULT __stdcall Reset() noexcept override
        {
            current = begin;
            return S_OK;
        }

        HRESULT __stdcall Skip(ULONG celt) noexcept override
        {
            auto currentDistance = std::distance(current, end);
            
            if (currentDistance < celt)
            {
                current = end;
                return S_FALSE;
            }

            std::advance(current, celt);
            return S_OK;
        }

        HRESULT __stdcall Next(ULONG celt, TEnumTraits::OutType rgVar, ULONG *pCeltFetched) noexcept override
        {
            if (rgVar == nullptr)
            {
                return S_FALSE;
            }

            auto index = std::distance(current, end);
            auto size = std::distance(begin, end);
            auto first = this->current;

            auto last = end;

            if (index + celt < size)
            {
                last = current;
                std::advance(last, celt);
            }

            auto count = 0;

            for (auto iter = first; iter != last; std::advance(iter, 1))
            {
                TEnumTraits::Copy(&rgVar[count], iter);
                count++;   
            }

            *pCeltFetched = ULONG(count);

            if (count == celt)
            {
                return S_OK;
            }

            return S_FALSE;
        }
    };

    
    template<typename TEnumTraits, typename TIter>
    auto make_unique_range_enumerator_from_traits(TIter begin, TIter current, TIter end)
    {
        auto* temp = new RangeEnumerator<TEnumTraits, TIter>(begin, current, end);
        return std::unique_ptr<
            RangeEnumerator<TEnumTraits, TIter>, 
            void(*)(RangeEnumerator<TEnumTraits, TIter>*)
        >
            (temp, [](auto* self) {
                    if (self)
                    {
                        delete self;
                    }
                });
    }

    template<typename TElement, typename TIter>
    auto make_unique_range_enumerator(TIter begin, TIter current, TIter end)
    {
        if constexpr(std::is_same_v<VARIANT, TElement> || std::is_base_of_v<VARIANT, TElement>)
        {
            return make_unique_range_enumerator_from_traits<VARIANT_EnumTraits<TElement>, TIter>(begin, current, end);
        }
        else if constexpr(std::is_same_v<CONNECTDATA, TElement> || std::is_base_of_v<CONNECTDATA, TElement>)
        {
            return make_unique_range_enumerator_from_traits<CONNECTDATA_EnumTraits<TElement>, TIter>(begin, current, end);
        }
        else if constexpr(std::is_same_v<std::wstring, TElement> || std::is_base_of_v<std::wstring, TElement>)
        {
            return make_unique_range_enumerator_from_traits<String_EnumTraits, TIter>(begin, current, end);
        }
        else if constexpr(std::is_same_v<std::wstring_view, TElement> || std::is_base_of_v<std::wstring_view, TElement>)
        {
            return make_unique_range_enumerator_from_traits<StringView_EnumTraits, TIter>(begin, current, end);
        }
        else
        {
            using element_type = typename TElement::element_type;

            if constexpr(std::is_same_v<IConnectionPoint, element_type>)
            {
                return make_unique_range_enumerator_from_traits<IConnectionPoint_EnumTraits, TIter>(begin, current, end);
            }
            else if constexpr(std::is_same_v<IUnknown, element_type> || std::is_base_of_v<IUnknown, element_type>)
            {
                return make_unique_range_enumerator_from_traits<IUnknown_EnumTraits<element_type>, TIter>(begin, current, end);
            }
            else
            {
                return make_unique_range_enumerator_from_traits<Struct_EnumTraits<TElement>, TIter>(begin, current, end);        
            }
        }
     }

    template<typename TEnum>
    struct VariantEnumeratorAdapter : IEnumVARIANT, ComObject
    {
        std::unique_ptr<TEnum, void(*)(TEnum*)> enumerator;
        
        VariantEnumeratorAdapter(std::unique_ptr<TEnum, void(*)(TEnum*)> enumerator) : enumerator(std::move(enumerator))
        {
            this->enumerator->SetParent(static_cast<IEnumVARIANT*>(this));
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComQuery<IUnknown, IEnumVARIANT>(*this, riid, ppvObj)
                .or_else([&]() { return ComQuery<IEnumVARIANT>(*this, riid, ppvObj); })
                .or_else([&]() { return ComQuery<TEnum::EnumType>(*enumerator.get(), riid, ppvObj); })
                .value_or(E_NOINTERFACE);
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            return ComObject::AddRef();
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            return ComObject::Release();
        }

        HRESULT __stdcall Clone(IEnumVARIANT** other) noexcept
        {
            auto inner = enumerator->Clone();

            auto temp = std::make_unique<VariantEnumeratorAdapter>(std::move(inner));
            *other = temp.release();

            return S_OK;
        }

        HRESULT __stdcall Next(ULONG celt, VARIANT *rgVar, ULONG *pCeltFetched) noexcept override
        {
            if (rgVar == nullptr)
            {
                return S_FALSE;
            }

            
            ULONG fetched = 0;
            HRESULT result = S_FALSE;

            if constexpr (std::is_abstract_v<TEnum::ElemType>)
            {
                std::vector<std::add_pointer_t<TEnum::ElemType>> temp(celt, nullptr);
                result = enumerator->Next(celt, reinterpret_cast<TEnum::OutType>(temp.data()), &fetched);

                for (auto i = 0; i < fetched; ++i)
                {
                    rgVar[i] = Variant(*temp[i]);

                    if constexpr(std::is_same_v<IUnknown, TEnum::ElemType> || std::is_base_of_v<IUnknown, TEnum::ElemType>)
                    {
                        temp[i]->Release();
                    }
                }
            }
            else if constexpr(std::is_same_v<TEnum::ElemType, std::wstring_view> || std::is_same_v<TEnum::ElemType, std::wstring>)
            {
                std::vector<wchar_t*> temp(celt);

                result = enumerator->Next(celt, temp.data(), &fetched);

                for (auto i = 0; i < fetched; ++i)
                {
                    rgVar[i] = Variant(std::wstring_view(temp[i]));
                    ::CoTaskMemFree(temp[i]);
                }
            }
            else
            {
                std::vector<TEnum::ElemType> temp(celt);
                result = enumerator->Next(celt, temp.data(), &fetched);

                for (auto i = 0; i < fetched; ++i)
                {
                    rgVar[i] = Variant(temp[i]);
                }
            }

            if (pCeltFetched)
            {
                *pCeltFetched = fetched;            
            }

            return result;
        }

        HRESULT __stdcall Skip(ULONG celt) noexcept override
        {
            return enumerator->Skip(celt);
        }

        HRESULT __stdcall Reset() noexcept override
        {
            return enumerator->Reset();
        }
    };
}

#endif