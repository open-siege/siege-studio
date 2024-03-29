#ifndef WIN32_COM_CLIENT_HPP
#define WIN32_COM_CLIENT_HPP

#include <expected>
#include "win32_com_variant.hpp"


namespace win32::com
{   
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
            VARIANT result;
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
                VariantClear(&result);
                return temp;
            }

            return std::unexpected(hresult);

        }
    };


    struct IReadOnlyCollection : IEnumerable
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

            OleVariant returnValue;

            auto result = this->Invoke(*id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &args, &returnValue.variant, nullptr, nullptr);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            result = VariantChangeType(&returnValue.variant, &returnValue.variant, 0, VT_UI4);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            return returnValue.variant.uintVal;
        }

        std::expected<OleVariant, HRESULT> Item(std::uint32_t index)  noexcept
        {
            DISPPARAMS args = {};

            OleVariant returnValue;

            auto result = this->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &args, &returnValue.variant, nullptr, nullptr);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            return returnValue;
        }
    };


    struct ICollection : IReadOnlyCollection
    {
        std::expected<OleVariant, HRESULT> Add(std::optional<OleVariant> newValue = std::nullopt)  noexcept
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
                args.rgvarg = &newValue->variant;
            }

            OleVariant returnValue;

            auto result = this->Invoke(*id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &args, &returnValue.variant, nullptr, nullptr);

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

            OleVariant returnValue;

            auto result = this->Invoke(*id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &args, &returnValue.variant, nullptr, nullptr);

            if (result != S_OK)
            {
                return std::unexpected(result);
            }

            return std::expected<void, HRESULT>{};
        }
    };

}

#endif