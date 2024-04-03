#ifndef WIN32_COM_COLLECTION_HPP
#define WIN32_COM_COLLECTION_HPP

#include "win32_com_enumerable.hpp"

namespace win32::com
{   
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
                return EnumeratorIterator<Variant, IEnum, decltype(newEnum)::value_type>(std::move(*newEnum), *count);
            }

            return IEnumerable::end<IEnum>();
            
        }
    };

    struct ICollection : IReadOnlyCollection
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

}

#endif