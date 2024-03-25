#ifndef WIN32_COM_SERVER_HPP
#define WIN32_COM_SERVER_HPP

#include "win32_com_variant.hpp"
#include <array>
#include <atomic>

namespace win32::com
{
    template<typename T>
    struct BaseUnknown : IUnknown
    {
        std::array<std::byte, sizeof(T)> storage;
        T* object = nullptr;
        std::atomic_int refCount = 0;

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            if (refCount == 0 && object == nullptr)
            {
                storage.fill(std::byte{});
                object = new (storage.data()) T();
            }
            return ++refCount;
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            if (refCount == 0)
            {
                return 0;
            }

            --refCount;

            if (refCount == 0)
            {
                object->~T();
                storage.fill(std::byte{});
            }

            return refCount;
        }
    };

    struct VectorEnumerator : IEnumVARIANT, IEnumUnknown
    {
        std::function<std::vector<OleVariant>&()> getVector;
        std::size_t index;
        std::atomic_int refCount;

        VectorEnumerator(std::function<std::vector<OleVariant>&()> getVector,  std::size_t index = 0) :  getVector(getVector), index(index), refCount(0)
        {
                
        }

        ~VectorEnumerator()
        {
            Release();
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            if (IsEqualGUID(riid, __uuidof(IUnknown)))
            {
                AddRef();
                *ppvObj = this;
                return S_OK;
            }

            if (IsEqualGUID(riid, __uuidof(IEnumVARIANT)))
            {
                AddRef();
                *ppvObj = static_cast<IEnumVARIANT*>(this);
                return S_OK;
            }

            if (IsEqualGUID(riid, __uuidof(IEnumUnknown)))
            {
                AddRef();
                *ppvObj = static_cast<IEnumUnknown*>(this);
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            return ++refCount;
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            if (refCount == 0)
            {
                return 0;
            }
            
            --refCount;
            
            if (refCount == 0)
            {
                getVector = nullptr;
                index = 0;
            }
            
            return refCount;
        }

        HRESULT __stdcall Clone(IEnumVARIANT** other) noexcept
        {
            auto temp = std::make_unique<VectorEnumerator>(getVector, index);
            temp->AddRef();
            *other = temp.release();

            return S_OK;
        }

        HRESULT __stdcall Clone(IEnumUnknown** other) noexcept
        {
            auto temp = std::make_unique<VectorEnumerator>(getVector, index);
            temp->AddRef();
            *other = temp.release();

            return S_OK;
        }

        HRESULT __stdcall Next(ULONG celt, VARIANT *rgVar, ULONG *pCeltFetched) noexcept
        {
            if (rgVar == nullptr)
            {
                return S_FALSE;
            }

            if (getVector)
            {
                auto& vector = getVector();
                auto begin = vector.begin() + index;
                auto end = index + celt > vector.size() ? vector.end() : vector.begin() + index + celt;

                auto distance = std::distance(begin, end);

                for (auto iter = begin; iter != end; ++iter)
                {
                    assert(VariantCopy(rgVar + std::distance(iter, end), &iter->variant) == S_OK);
                }

                *pCeltFetched = ULONG(distance);

                if (distance == celt)
                {
                    return S_OK;
                }
            }

            return S_FALSE;
        }

        HRESULT __stdcall Next(ULONG celt, IUnknown **rgVar, ULONG *pCeltFetched) noexcept
        {
            if (rgVar == nullptr)
            {
                return S_FALSE;
            }

            if (getVector)
            {
                auto& vector = getVector();
                auto begin = vector.begin() + index;
                auto end = index + celt > vector.size() ? vector.end() : vector.begin() + index + celt;

                auto count = 0;

                for (auto iter = begin; iter != end; ++iter)
                {
                    if (iter->variant.vt == VT_UNKNOWN || iter->variant.vt == VT_DISPATCH)
                    {
                        assert(iter->variant.punkVal);
                        iter->variant.punkVal->AddRef();
                        rgVar[std::distance(iter, end)] = iter->variant.punkVal;
                        count++;
                    }
                }

                *pCeltFetched = ULONG(count);

                if (count == celt)
                {
                    return S_OK;
                }
            }

            return S_FALSE;
        }

        HRESULT __stdcall Reset() noexcept
        {
            index = 0;
            return S_OK;
        }

        HRESULT __stdcall Skip(ULONG celt) noexcept
        {
            if (getVector && getVector().size() < celt)
            {
                return S_FALSE;
            }

            index += index;
                    return S_OK;
        }
    };

    struct VectorCollection : BaseUnknown<std::vector<OleVariant>>, IDispatch
    {
        ATOM countAtom = 0;
        ATOM addAtom = 0;
        ATOM removeAtom = 0;

        VectorCollection()
        {
            AddRef();
        }

        ~VectorCollection()
        {
            Release();
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            if (IsEqualGUID(riid, __uuidof(IUnknown)))
            {
                AddRef();
                *ppvObj = this;
                return S_OK;
            }

            if (IsEqualGUID(riid, __uuidof(IDispatch)))
            {
                AddRef();
                *ppvObj = static_cast<IDispatch*>(this);
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
        {
            auto result = BaseUnknown::AddRef();
            countAtom  = AddAtomW(L"Count");
            addAtom = AddAtomW(L"Add");
            removeAtom = AddAtomW(L"Remove");
            return result;
        }

        [[maybe_unused]] ULONG __stdcall Release() noexcept override
        {
            DeleteAtom(countAtom);
            DeleteAtom(addAtom);
            DeleteAtom(removeAtom);
            return BaseUnknown::Release();
        }

        HRESULT __stdcall GetIDsOfNames(const GUID& riid, wchar_t **rgszNames, UINT cNames, LCID  lcid, DISPID  *rgDispId) noexcept override
        {
            assert(IsEqualGUID(riid, IID_NULL));

            if (cNames >= 0)
            {
                assert(rgszNames);

                auto atom = FindAtomW(rgszNames[0]);

                if (atom)
                {
                    *rgDispId = atom;
                    return S_OK;
                }
        
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

                *rgDispId = DISPID_UNKNOWN;
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

            OleVariant temp;

            if (dispIdMember == addAtom && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto changed = VariantChangeType(&temp.variant, pDispParams->rgvarg, 0, VT_UNKNOWN);

                if (changed != S_OK)
                {
                    *puArgErr = 0;
                    return changed;
                }

                return Add(temp.variant, *pVarResult);
            }

            if (dispIdMember == removeAtom && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto changed = VariantChangeType(&temp.variant, pDispParams->rgvarg, 0, VT_UI4);

                if (changed != S_OK)
                {
                    *puArgErr = 0;
                    return changed;
                }

                return Remove(temp.variant, *pVarResult);
            }

            if (dispIdMember == countAtom && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
            {
                return Count(*pVarResult);
            }

            if (dispIdMember == DISPID_VALUE && wFlags & DISPATCH_METHOD)
            {
                if (pDispParams->cArgs != 1)
                {
                    return DISP_E_BADPARAMCOUNT;
                }

                auto changed = VariantChangeType(&temp.variant, pDispParams->rgvarg, 0, VT_UI4);

                if (changed != S_OK)
                {
                    *puArgErr = 0;
                    return changed;
                }

                return Item(temp.variant, *pVarResult);
            }

            if (dispIdMember == DISPID_NEWENUM && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
            {
                return NewEnum(*pVarResult);
            }

            return DISP_E_MEMBERNOTFOUND;
        }

        HRESULT Add(VARIANTARG& value, VARIANT& result) noexcept
        {
            auto& newItem = object->emplace_back();
            result.vt = VT_EMPTY;
            return VariantCopy(&newItem.variant, &value);
        }

        HRESULT Remove(VARIANTARG& value, VARIANT& result) noexcept
        {
            auto begin = object->begin() + result.uintVal;
            object->erase(begin);
            result.vt = VT_EMPTY;
            return S_OK;
        }

        HRESULT Count(VARIANT& result) noexcept
        {
            result.vt = VT_I4;
            result.intVal = int(object->size());
            return S_OK;
        }

        HRESULT Item(VARIANTARG& value, VARIANT& result) noexcept
        {
            std::size_t index = std::size_t(value.uintVal); 

            if (object->size() > index)
            {
                return VariantCopy(&result, &object->operator[](index).variant);
            }

            return S_OK;
        }

        HRESULT NewEnum(VARIANT& result) noexcept
        {
            auto item = std::make_unique<VectorEnumerator>([this]() -> std::vector<OleVariant>& { return *object;});

            result.vt = VT_UNKNOWN;
            result.punkVal = static_cast<IEnumVARIANT*>(item.release());

            return S_OK;
        }
    };
}

#endif