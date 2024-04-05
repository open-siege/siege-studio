#ifndef WIN32_COM_SERVER_HPP
#define WIN32_COM_SERVER_HPP

#include "win32_com_variant.hpp"
#include "win32_com.hpp"
#include <array>
#include <atomic>
#include <optional>
#include <memory>
#include <type_traits>
#include <string_view>
#include <vector>
#include <expected>
#include <ocidl.h>
#include <olectl.h>

namespace win32::com
{
    struct ComAllocatorAware : IUnknown
    {
        std::atomic_uint refCount = 1;

        static bool IsHeapAllocated(void* object, std::size_t);
        static void* operator new(std::size_t count);
        static void operator delete(void* ptr, std::size_t sz);

        [[maybe_unused]] ULONG __stdcall IUnknown::AddRef() noexcept override;
        [[maybe_unused]] ULONG __stdcall IUnknown::Release() noexcept override;
    };

    template<typename TBase, typename TInterface, typename TObject>
    std::optional<HRESULT> ComCast(TObject& self, const GUID& riid, void** ppvObj)
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
    std::optional<HRESULT> ComCast(TObject& self, const GUID& riid, void** ppvObj)
    {
        if (IsEqualGUID(riid, __uuidof(TInterface)))
        {
            self.AddRef();
            *ppvObj = static_cast<TInterface*>(&self);
            return S_OK;
        }

        return std::nullopt;
    }

    template<typename InterfaceType = IUnknown>
    struct Interface_EnumTraits
    {
        using ElemType = InterfaceType;
        using PtrType = std::add_pointer_t<ElemType>;
        using OutType = std::add_pointer_t<PtrType>;
    };

    template<typename StructType>
    struct Struct_EnumTraits
    {
        using ElemType = StructType;
        using PtrType = std::add_pointer_t<ElemType>;
        using OutType = PtrType;
    };

    template<typename InterfaceType = IUnknown>
    struct IUnknown_EnumTraits : Interface_EnumTraits<InterfaceType>
    {
        using EnumType = IEnumUnknown;
        using OutType = std::add_pointer_t<IUnknown*>;

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
        using OutType = std::add_pointer_t<VARIANT>;

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
        using OutType = std::add_pointer_t<CONNECTDATA>;

        template <typename Iter>
        static void Copy(CONNECTDATA *output, Iter iter)
        {
            output->dwCookie = iter->dwCookie;
            output->pUnk = iter->pUnk;
            output->pUnk->AddRef();
        }
    };

    template<typename TEnumTraits, typename TIter>
    struct RangeEnumerator : TEnumTraits::EnumType, ComAllocatorAware
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

            return ComCast<IUnknown, EnumType>(*this, riid, ppvObj)
                .or_else([&]() { return ComCast<EnumType>(*this, riid, ppvObj); })
                .value_or(E_NOINTERFACE);
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            if (owner)
            {
                return refCount = owner->AddRef();
            }

            return ComAllocatorAware::AddRef();
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            if (owner)
            {
                return refCount = owner->Release();
            }

            return ComAllocatorAware::Release();
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
        else if constexpr(std::is_same_v<IConnectionPoint, TElement>)
        {
            return make_unique_range_enumerator_from_traits<IConnectionPoint_EnumTraits, TIter>(begin, current, end);
        }
        else if constexpr(std::is_same_v<IUnknown, TElement> || std::is_base_of_v<IUnknown, TElement>)
        {
            return make_unique_range_enumerator_from_traits<IUnknown_EnumTraits<TElement>, TIter>(begin, current, end);
        }
        else
        {
            return make_unique_range_enumerator_from_traits<Struct_EnumTraits<TElement>, TIter>(begin, current, end);        
        }
     }

    inline Variant ToVariant(IUnknown* src)
    {
        Variant dst;
        dst.vt = VT_UNKNOWN;
        src->AddRef();
        dst.punkVal = src;
        return dst;
    }

    inline Variant ToVariant(IDispatch* src)
    {
        Variant dst;
        dst.vt = VT_DISPATCH;
        src->AddRef();
        dst.pdispVal = src;

        return dst;
    }

    template<typename TEnum>
    struct VariantEnumeratorAdapter : IEnumVARIANT, ComAllocatorAware
    {
        std::unique_ptr<TEnum, void(*)(TEnum*)> enumerator;
        
        VariantEnumeratorAdapter(std::unique_ptr<TEnum, void(*)(TEnum*)> enumerator) : enumerator(std::move(enumerator))
        {
            this->enumerator->SetParent(static_cast<IEnumVARIANT*>(this));
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComCast<IUnknown, IEnumVARIANT>(*this, riid, ppvObj)
                .or_else([&]() { return ComCast<IEnumVARIANT>(*this, riid, ppvObj); })
                .or_else([&]() { return ComCast<TEnum::EnumType>(*enumerator.get(), riid, ppvObj); })
                .value_or(E_NOINTERFACE);
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            return ComAllocatorAware::AddRef();
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            return ComAllocatorAware::Release();
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
                    rgVar[i] = ToVariant(temp[i]);

                    if constexpr(std::is_same_v<IUnknown, TEnum::ElemType> || std::is_base_of_v<IUnknown, TEnum::ElemType>)
                    {
                        temp[i]->Release();
                    }
                }
            }
            else
            {
                std::vector<TEnum::ElemType> temp(celt);
                result = enumerator->Next(celt, temp.data(), &fetched);

                for (auto i = 0; i < fetched; ++i)
                {
                    rgVar[i] = ToVariant(temp[i]);
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

    struct ConnectData : ::CONNECTDATA
    {
        std::unique_ptr<IDispatch, void(*)(IDispatch*)> sink;

        ConnectData(std::unique_ptr<IDispatch, void(*)(IDispatch*)> sink, DWORD cookie) : 
            ::CONNECTDATA{static_cast<IUnknown*>(sink.get()), cookie}, sink(std::move(sink))
        {
        
        }
    };


    struct ConnectionPoint : IConnectionPoint, ComAllocatorAware
    {
        std::unique_ptr<IConnectionPointContainer, void(*)(IConnectionPointContainer*)> container; 
        std::vector<ConnectData> callbacks;

        ConnectionPoint(std::unique_ptr<IConnectionPointContainer, void(*)(IConnectionPointContainer*)> container) : container(std::move(container))
        {
            callbacks.reserve(4);
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComCast<IUnknown, IConnectionPoint>(*this, riid, ppvObj)
                .or_else([&]() { return ComCast<IConnectionPoint>(*this, riid, ppvObj); })
                .value_or(E_NOINTERFACE);
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            return ComAllocatorAware::AddRef();
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            return ComAllocatorAware::Release();
        }

        HRESULT Advise(IUnknown *pUnkSink, DWORD *pdwCookie) noexcept override
        {
            if (!(pUnkSink || pdwCookie))
            {
                return E_INVALIDARG;
            }

            IDispatch* dispatch = nullptr;

            if (pUnkSink->QueryInterface<IDispatch>(&dispatch) == S_OK)
            {
                DWORD cookie = callbacks.size() + 1;

                if (!callbacks.empty())
                {
                    cookie = callbacks.rbegin()->dwCookie + 1;
                }

                auto& back = callbacks.emplace_back(as_unique(dispatch), cookie);
                *pdwCookie = back.dwCookie;               
                return S_OK;
            }

            return CONNECT_E_CANNOTCONNECT;
        }

        HRESULT Unadvise(DWORD dwCookie) override
        {
            auto result = std::find_if(callbacks.begin(), callbacks.end(), [&](auto& callback) {
                    return callback.dwCookie == dwCookie;
            });

            if (result == callbacks.end())
            {
                return E_POINTER;
            }

            callbacks.erase(result);

            return S_OK;
        }

        HRESULT GetConnectionInterface(IID* pIID) override
        {
            if (!pIID)
            {
                return E_POINTER;
            }

            *pIID = __uuidof(IDispatch);

            return S_OK;
        }

        HRESULT GetConnectionPointContainer(IConnectionPointContainer** ppCPC) override
        {
            if (!ppCPC)
            {
                return E_POINTER;
            }

            container->AddRef();
            *ppCPC = container.get();

            return S_OK;
        }

        HRESULT EnumConnections(IEnumConnections** ppEnum) override
        {
            if (!ppEnum)
            {
                return E_POINTER;
            }

            auto enumerator = make_unique_range_enumerator<CONNECTDATA>(callbacks.begin(), callbacks.begin(), callbacks.end());
            *ppEnum = static_cast<IEnumConnections*>(enumerator.release());

            return S_OK;
        }
    };

    struct ConnectionPointContainer : IConnectionPointContainer, ComAllocatorAware
    {
        std::vector<std::unique_ptr<IConnectionPoint, void(*)(IConnectionPoint*)>> points;
    
        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComCast<IUnknown, IConnectionPointContainer>(*this, riid, ppvObj)
                .or_else([&]() { return ComCast<IConnectionPointContainer>(*this, riid, ppvObj); })
                .value_or(E_NOINTERFACE);
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            return ComAllocatorAware::AddRef();
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            return ComAllocatorAware::Release();
        }

        HRESULT EnumConnectionPoints(IEnumConnectionPoints **ppEnum) noexcept override
        {
            if (!ppEnum)
            {
                return E_POINTER;
            }

            auto enumerator = make_unique_range_enumerator<IConnectionPoint>(points.begin(), points.begin(), points.end());
            *ppEnum = static_cast<IEnumConnectionPoints*>(enumerator.release());

            return S_OK;
        }

        HRESULT FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP) noexcept override
        {
            if (!ppCP)
            {
                return E_POINTER;
            }

            auto result = std::find_if(points.begin(), points.end(), [&](auto& item) {
                IID temp;

                return item->GetConnectionInterface(&temp) == S_OK && IsEqualGUID(riid, temp);
            });

            if (result == points.end())
            {
                return CONNECT_E_NOCONNECTION;
            }

            return S_OK;
        }

    };

    template<typename TObject>
    struct VectorCollection : IDispatch, ComAllocatorAware
    {
        std::vector<std::unique_ptr<TObject, void(*)(TObject*)>> &items;

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

        VectorCollection(std::vector<std::unique_ptr<TObject, void(*)(TObject*)>> &items) : items(items)
        {
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComCast<IUnknown, IDispatch>(*this, riid, ppvObj)
                .or_else([&]() { return ComCast<IDispatch>(*this, riid, ppvObj); })
                .value_or(E_NOINTERFACE);
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            return ComAllocatorAware::AddRef();
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            return ComAllocatorAware::Release();
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
            TObject* temp = nullptr;

            if ((value.vt == VT_UNKNOWN || value.vt == VT_DISPATCH) && value.punkVal->QueryInterface<TObject>(&temp) == S_OK)
            {
               items.emplace_back(com::as_unique<TObject>(temp));
            }

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
                return ToVariant(items[index].get());
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
}

#endif