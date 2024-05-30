#ifndef WIN32_COM_COLLECTION_HPP
#define WIN32_COM_COLLECTION_HPP

#include <siege/platform/win/core/com_enumerable.hpp>

namespace win32::com
{   
    struct __declspec(uuid("00020400-0000-0000-C000-000000000046")) IReadOnlyCollection : IEnumerable
    {
        std::expected<std::uint32_t, HRESULT> Count() noexcept
        {
            static auto count = std::wstring(L"Count");

            auto id = GetDispId(count);

            if (!id)
            {
                return std::unexpected(id.error());
            }
            
            DISPPARAMS args = {};

            Variant returnValue;

            auto result = this->Invoke(*id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &args, &returnValue, nullptr, nullptr);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            result = VariantChangeType(&returnValue, &returnValue, 0, VT_UI4);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            return returnValue.uintVal;
        }

        std::expected<Variant, HRESULT> Item(std::uint32_t index)  noexcept
        {
            DISPPARAMS args = {};

            Variant returnValue;

            auto result = this->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &args, &returnValue, nullptr, nullptr);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            return returnValue;
        }

        template<typename IEnum = IEnumVARIANT>
        auto end()
        {
            auto count = Count();

            if (count)
            {
                auto newEnum = NewEnum<IEnum>();
                return EnumeratorIterator<Variant, IEnum, typename decltype(newEnum)::value_type>(std::move(*newEnum), *count);
            }

            return IEnumerable::end<IEnum>();
            
        }
    };

    struct __declspec(uuid("00020400-0000-0000-C000-000000000046")) ICollection : IReadOnlyCollection
    {
        std::expected<Variant, HRESULT> Add(std::optional<Variant> newValue = std::nullopt)  noexcept
        {
            static auto count = std::wstring(L"Add");

            auto id = GetDispId(count);

            if (!id)
            {
                return std::unexpected(id.error());
            }
            
            DISPPARAMS args = {};

            if (newValue)
            {
                args.cArgs = 1;
                args.rgvarg = &*newValue;
            }

            Variant returnValue;

            auto result = this->Invoke(*id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &args, &returnValue, nullptr, nullptr);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            return returnValue;
        }

        std::expected<void, HRESULT> Remove(std::uint32_t)
        {
            static auto count = std::wstring(L"Remove");

            auto id = GetDispId(count);

            if (!id)
            {
                return std::unexpected(id.error());
            }
            
            DISPPARAMS args = {};

            Variant returnValue;

            auto result = this->Invoke(*id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &args, &returnValue, nullptr, nullptr);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            return std::expected<void, HRESULT>{};
        }
    };

    template<typename TObject, typename TContainer = std::vector<TObject>>
    struct ReadOnlyCollectionRef : IReadOnlyCollection, ComObject
    {
        TContainer& items;

        ReadOnlyCollectionRef(TContainer &items) : items(items)
        {
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComQuery<IUnknown, IDispatch>(*this, riid, ppvObj)
                .or_else([&]() { return ComQuery<IDispatch>(*this, riid, ppvObj); })
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

        HRESULT __stdcall GetIDsOfNames(const GUID& riid, wchar_t **rgszNames, UINT cNames, LCID lcid, DISPID  *rgDispId) noexcept override
        {
            assert(IsEqualGUID(riid, IID_NULL));

            if (cNames >= 0)
            {
                assert(rgszNames);

                std::wstring_view temp = rgszNames[0];

                constexpr static auto NewEnum = std::wstring_view(L"_NewEnum");

                if (CompareStringW(lcid, NORM_IGNORECASE, NewEnum.data(), NewEnum.size(), temp.data(), temp.size()) == CSTR_EQUAL)
                {    
                    *rgDispId = DISPID_NEWENUM;
                    return S_OK;
                }

                constexpr static auto Item = std::wstring_view(L"Item");
                
                if (CompareStringW(lcid, NORM_IGNORECASE, Item.data(), Item.size(), temp.data(), temp.size()) == CSTR_EQUAL)
                {
                    *rgDispId = DISPID_VALUE;
                    return S_OK;
                }

                constexpr static auto Count = std::wstring_view(L"Count");

                if (CompareStringW(lcid, NORM_IGNORECASE, Count.data(), Count.size(), temp.data(), temp.size()) == CSTR_EQUAL)
                {
                    *rgDispId = DISPID_VALUE + 1;
                    return S_OK;
                }

                *rgDispId = DISPID_UNKNOWN;
            }

            return DISP_E_UNKNOWNNAME;
        }

        HRESULT __stdcall GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) noexcept override
        {
            if (ppTInfo)
            {
                *ppTInfo = nullptr;
            }
            return DISP_E_BADINDEX;
        }

        HRESULT __stdcall GetTypeInfoCount(UINT *pctinfo) override
        {
            assert(pctinfo);
            *pctinfo = 0;
            return S_OK;
        }

        HRESULT __stdcall Invoke(DISPID dispIdMember, const GUID& riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override
        {
            assert(IsEqualGUID(riid, IID_NULL));

            if (pVarResult == nullptr)
            {
                return DISP_E_PARAMNOTOPTIONAL;
            }

            if (dispIdMember == DISPID_VALUE + 1 && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
            {
                auto result = Count();

                if (result)
                {
                    *pVarResult = *result;
                    return S_OK;
                }

                return result.error();
            }

            if (dispIdMember == DISPID_VALUE && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto result = Item(*pDispParams->rgvarg);

                if (result)
                {
                    *pVarResult = *result;
                    return S_OK;
                }

                return result.error();
            }

            if (dispIdMember == DISPID_NEWENUM && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
            {
                auto result = NewEnum();

                if (result)
                {
                    *pVarResult = *result;
                    return S_OK;
                }

                return result.error();
            }

            return DISP_E_MEMBERNOTFOUND;
        }

        std::expected<Variant, HRESULT> Count() noexcept
        {
            Variant result;
            result.vt = VT_I4;
            result.intVal = int(items.size());
            return result;
        }

        std::expected<Variant, HRESULT> Item(Variant value) noexcept
        {
            auto changed = VariantChangeType(&value, &value, 0, VT_UI4);

            if (changed != S_OK)
            {
                return std::unexpected(changed);
            }

            std::size_t index = std::size_t(value.uintVal); 

            if (items.size() > index)
            {
                return Variant(items[index]);
            }

            return Variant();
        }

        std::expected<Variant, HRESULT> NewEnum() noexcept
        {
            auto enumerator = make_unique_range_enumerator<TObject>(items.begin(), items.begin(), items.end());
         
            auto item = std::make_unique<VariantEnumeratorAdapter<decltype(enumerator)::element_type>>(std::move(enumerator));

            Variant result;
            result.vt = VT_UNKNOWN;
            result.punkVal = static_cast<IEnumVARIANT*>(item.release());

            return result;
        }
    };

    template<typename TObject, typename TContainer = std::vector<TObject>>
    struct CollectionRef : ICollection, ComObject
    {
        TContainer& items;

        constexpr static std::array<std::wstring_view, 4> names = {{
            std::wstring_view(L"Item"),  
            std::wstring_view(L"Count"),  
            std::wstring_view(L"Add"),  
            std::wstring_view(L"Remove"),  
        }};

        constexpr static DISPID DispIdOf(std::wstring_view name)
        {
            auto existingName = std::find(names.begin(), names.end(), name);

            if (existingName != names.end())
            {
                return static_cast<DISPID>(std::distance(names.begin(), existingName));
            }

            return DISPID_UNKNOWN;
        }

        constexpr static DISPID DispIdOf(std::wstring_view name, LCID lcid)
        {
            auto existingName = std::find_if(names.begin(), names.end(), [&](auto& value) {
                    return CompareStringW(lcid, NORM_IGNORECASE, value.data(), value.size(), name.data(), name.size()) == CSTR_EQUAL;
                 });

            if (existingName != names.end())
            {
                return static_cast<DISPID>(std::distance(names.begin(), existingName));
            }

            return DISPID_UNKNOWN;
        }

        CollectionRef(TContainer &items) : items(items)
        {
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComQuery<IUnknown, IDispatch>(*this, riid, ppvObj)
                .or_else([&]() { return ComQuery<IDispatch>(*this, riid, ppvObj); })
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

        HRESULT __stdcall GetIDsOfNames(const GUID& riid, wchar_t **rgszNames, UINT cNames, LCID lcid, DISPID  *rgDispId) noexcept override
        {
            assert(IsEqualGUID(riid, IID_NULL));

            if (cNames >= 0)
            {
                assert(rgszNames);

                std::wstring_view temp = rgszNames[0];

                constexpr static auto NewEnum = std::wstring_view(L"_NewEnum");

                if (CompareStringW(lcid, NORM_IGNORECASE, NewEnum.data(), NewEnum.size(), temp.data(), temp.size()) == CSTR_EQUAL)
                {    
                    *rgDispId = DISPID_NEWENUM;
                    return S_OK;
                }

                constexpr static auto Item = std::wstring_view(L"Item");
                
                if (CompareStringW(lcid, NORM_IGNORECASE, Item.data(), Item.size(), temp.data(), temp.size()) == CSTR_EQUAL)
                {
                    *rgDispId = DISPID_VALUE;
                    return S_OK;
                }

                *rgDispId = DispIdOf(temp, lcid);

                if (*rgDispId != DISPID_UNKNOWN)
                {
                    return S_OK;
                }
            }

            return DISP_E_UNKNOWNNAME;
        }

        HRESULT __stdcall GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) noexcept override
        {
            if (ppTInfo)
            {
                *ppTInfo = nullptr;
            }
            return DISP_E_BADINDEX;
        }

        HRESULT __stdcall GetTypeInfoCount(UINT *pctinfo) override
        {
            assert(pctinfo);
            *pctinfo = 0;
            return S_OK;
        }

        HRESULT __stdcall Invoke(DISPID dispIdMember, const GUID& riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override
        {
            assert(IsEqualGUID(riid, IID_NULL));

            if (pVarResult == nullptr)
            {
                return DISP_E_PARAMNOTOPTIONAL;
            }

            if (dispIdMember == DispIdOf(L"Add") && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto result = Add(*pDispParams->rgvarg);

                if (result)
                {
                    *pVarResult = *result;
                    return S_OK;
                }

                return result.error();
            }

            if (dispIdMember == DispIdOf(L"Remove") && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto result = Remove(*pDispParams->rgvarg);

                if (result)
                {
                    *pVarResult = *result;
                    return S_OK;
                }

                return result.error();
            }

            if (dispIdMember == DispIdOf(L"Count") && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
            {
                auto result = Count();

                if (result)
                {
                    *pVarResult = *result;
                    return S_OK;
                }

                return result.error();
            }

            if (dispIdMember == DISPID_VALUE && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto result = Item(*pDispParams->rgvarg);

                if (result)
                {
                    *pVarResult = *result;
                    return S_OK;
                }

                return result.error();
            }

            if (dispIdMember == DISPID_NEWENUM && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
            {
                auto result = NewEnum();

                if (result)
                {
                    *pVarResult = *result;
                    return S_OK;
                }

                return result.error();
            }

            return DISP_E_MEMBERNOTFOUND;
        }

        std::expected<Variant, HRESULT> Add(Variant value) noexcept
        {
            auto changed = VariantChangeType(&value, &value, 0, VT_UNKNOWN);

            if (changed != S_OK)
            {
                return std::unexpected(changed);
            }

            Variant result;
            result.vt = VT_EMPTY;

            items.emplace_back(value);

            return result;
        }

        std::expected<Variant, HRESULT> Remove(Variant value) noexcept
        {
            auto changed = VariantChangeType(&value, &value, 0, VT_UI4);

            if (changed != S_OK)
            {
                return std::unexpected(changed);
            }

            auto begin = items.begin() + value.uintVal;
            items.erase(begin);
            return Variant();
        }

        std::expected<Variant, HRESULT> Count() noexcept
        {
            Variant result;
            result.vt = VT_I4;
            result.intVal = int(items.size());
            return result;
        }

        std::expected<Variant, HRESULT> Item(Variant value) noexcept
        {
            auto changed = VariantChangeType(&value, &value, 0, VT_UI4);

            if (changed != S_OK)
            {
                return std::unexpected(changed);
            }

            std::size_t index = std::size_t(value.uintVal); 

            if (items.size() > index)
            {
                return Variant(items[index]);
            }

            return Variant();
        }

        std::expected<Variant, HRESULT> NewEnum() noexcept
        {
            auto enumerator = make_unique_range_enumerator<TObject>(items.begin(), items.begin(), items.end());
         
            auto item = std::make_unique<VariantEnumeratorAdapter<decltype(enumerator)::element_type>>(std::move(enumerator));

            Variant result;
            result.vt = VT_UNKNOWN;
            result.punkVal = static_cast<IEnumVARIANT*>(item.release());

            return result;
        }
    };

    template<typename TObject, typename TContainer = std::vector<TObject>>
    struct OwningCollection : public CollectionRef<TObject, TContainer>
    {
        TContainer value;

        OwningCollection() : CollectionRef<TObject, TContainer>(value)
        {
        
        }
    };
}

#endif