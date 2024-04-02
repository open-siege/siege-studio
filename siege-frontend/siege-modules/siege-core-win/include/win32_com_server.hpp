#ifndef WIN32_COM_SERVER_HPP
#define WIN32_COM_SERVER_HPP

#include "win32_com_variant.hpp"
#include <array>
#include <atomic>
#include <optional>
#include <memory>
#include <string_view>
#include <vector>

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

    template <typename Iter>
    void Copy(IUnknown **output, Iter iter)
    {
        (*iter)->QueryInterface<IUnknown>(output);
    }

    template <typename Iter>
    void Copy(VARIANT *output, Iter iter)
    {
        VariantCopy(output, iter->get());
    }

    template<typename TInterface, typename TOut, typename TIter>
    struct RangeEnumerator : TInterface, ComAllocatorAware
    {
        IUnknown* owner;
        
        TIter begin;
        TIter current;
        TIter end;

        using EnumeratorType = TInterface;
        using ElementType = TOut;
        using ElementPtrType = TOut*;

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

            return ComCast<IUnknown, TInterface>(*this, riid, ppvObj)
                .or_else([&]() { return ComCast<TInterface>(*this, riid, ppvObj); })
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

        HRESULT __stdcall Clone(TInterface** other) noexcept override
        {
            auto temp = std::make_unique<RangeEnumerator<TInterface, TOut, TIter>>(begin, current, end);
            *other = static_cast<TInterface*>(temp.release());

            return S_OK;
        }

        auto Clone()
        {
            auto* temp = new RangeEnumerator<TInterface, TOut, TIter>(begin, current, end);
            return std::unique_ptr<
                RangeEnumerator<TInterface, TOut, TIter>, 
                void(*)(RangeEnumerator<TInterface, TOut, TIter>*)
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

        HRESULT __stdcall Next(ULONG celt, TOut **rgVar, ULONG *pCeltFetched) noexcept override
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
                Copy(&rgVar[count], iter);
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

    
    template<typename TInterface, typename TOut, typename TIter>
    auto MakeRangeEnumerator(TIter begin, TIter current, TIter end)
    {
        auto* temp = new RangeEnumerator<TInterface, TOut, TIter>(begin, current, end);
        return std::unique_ptr<
            RangeEnumerator<TInterface, TOut, TIter>, 
            void(*)(RangeEnumerator<TInterface, TOut, TIter>*)
        >
            (temp, [](auto* self) {
                    if (self)
                    {
                        delete self;
                    }
                });
    }

    inline void ToVariant(IUnknown* src, VARIANT* dst)
    {
        dst->vt = VT_UNKNOWN;
        src->AddRef();
        dst->punkVal = src;
    }

    inline void ToVariant(IDispatch* src, VARIANT* dst)
    {
        dst->vt = VT_DISPATCH;
        src->AddRef();
        dst->pdispVal = src;
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
                .or_else([&]() { return ComCast<TEnum::EnumeratorType>(*enumerator.get(), riid, ppvObj); })
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

            std::vector<TEnum::ElementPtrType> temp(celt, nullptr);

            ULONG fetched = 0;

            auto result = enumerator->Next(celt, temp.data(), &fetched);

            for (auto i = 0; i < fetched; ++i)
            {
                VariantInit(&rgVar[i]);
                ToVariant(temp[i], &rgVar[i]);
                temp[i]->Release();
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

        constexpr static DISPID DispIdOf(std::wstring_view name, LCID lcid = LOCALE_USER_DEFAULT)
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

                *rgDispId = DispIdOf(temp);

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

            Variant temp;

            if (dispIdMember == DispIdOf(L"Add") && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto changed = VariantChangeType(&temp, pDispParams->rgvarg, 0, VT_UNKNOWN);

                if (changed != S_OK)
                {
                    *puArgErr = 0;
                    return changed;
                }

                return Add(temp, *pVarResult);
            }

            if (dispIdMember == DispIdOf(L"Remove") && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto changed = VariantChangeType(&temp, pDispParams->rgvarg, 0, VT_UI4);

                if (changed != S_OK)
                {
                    *puArgErr = 0;
                    return changed;
                }

                return Remove(temp, *pVarResult);
            }

            if (dispIdMember == DispIdOf(L"Count") && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
            {
                return Count(*pVarResult);
            }

            if (dispIdMember == DISPID_VALUE && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto changed = VariantChangeType(&temp, pDispParams->rgvarg, 0, VT_UI4);

                if (changed != S_OK)
                {
                    *puArgErr = 0;
                    return changed;
                }

                return Item(temp, *pVarResult);
            }

            if (dispIdMember == DISPID_NEWENUM && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
            {
                return NewEnum(*pVarResult);
            }

            return DISP_E_MEMBERNOTFOUND;
        }

        HRESULT Add(VARIANTARG& value, VARIANT& result) noexcept
        {
            result.vt = VT_EMPTY;
            TObject* temp = nullptr;

            if ((value.vt == VT_UNKNOWN || value.vt == VT_DISPATCH) && value.punkVal->QueryInterface<TObject>(&temp) == S_OK)
            {
               items.emplace_back(temp, [](TObject* self) { 
                   if (self)
                   {
                    self->Release();
                   }
                   
             });
            }

            return S_OK;
        }

        HRESULT Remove(VARIANTARG& value, VARIANT& result) noexcept
        {
            auto begin = items.begin() + result.uintVal;
            items.erase(begin);
            result.vt = VT_EMPTY;
            return S_OK;
        }

        HRESULT Count(VARIANT& result) noexcept
        {
            result.vt = VT_I4;
            result.intVal = int(items.size());
            return S_OK;
        }

        HRESULT Item(VARIANTARG& value, VARIANT& result) noexcept
        {
            std::size_t index = std::size_t(value.uintVal); 

            if (items.size() > index)
            {
                ToVariant(items[index].get(), &result);
            }

            return S_OK;
        }

        HRESULT NewEnum(VARIANT& result) noexcept
        {
            auto enumerator = MakeRangeEnumerator<IEnumUnknown, IUnknown>(items.begin(), items.begin(), items.end());
            auto item = std::make_unique<VariantEnumeratorAdapter<decltype(enumerator)::element_type>>(std::move(enumerator));

            result.vt = VT_UNKNOWN;
            result.punkVal = static_cast<IEnumVARIANT*>(item.release());

            return S_OK;
        }
    };
}

#endif