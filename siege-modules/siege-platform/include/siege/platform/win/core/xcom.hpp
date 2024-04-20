#include <iostream>
#include <functional>
#include <variant>
#include <string>
#include <array>
#include <system_error>
#include <type_traits>
#include <cstdint>

#if _WIN32 || _WIN64
    #if _WIN64
    #define stdcall
    #else
    #define stdcall __stdcall
    #endif
#endif

#if __GNUC__
    #if __x86_64__
    #define stdcall
    #else
    #define stdcall __attribute__((stdcall))
    #endif
#endif

namespace xcom
{
    union guid_t {
        std::array<std::byte, 16> as_array;
        struct {
            std::uint32_t header;
            std::uint16_t segment_a;
            std::uint16_t segment_b;
            std::uint64_t footer;
        } as_guid;

    } data;

    static_assert(sizeof(guid_t) == sizeof(std::array<std::byte, 16>));
    static_assert(std::is_trivially_copyable_v<guid_t>);
    static_assert(std::is_standard_layout_v<guid_t>);

    struct hresult_t
    {
        std::uint32_t code;

        hresult_t(std::uint32_t code) : code(code)
        {
        }

        //TODO implement
        //hresult_t(std::errc std_code)
        //{
        //}

        operator std::uint32_t() const
        {
            return code;
        }
    };

    static_assert(std::is_trivially_copyable_v<hresult_t>);
    static_assert(std::is_standard_layout_v<hresult_t>);
    static_assert(sizeof(std::uint32_t) == sizeof(hresult_t));

    struct IUnknown
    {
        //00000000-0000-0000-C000-000000000046
        constexpr static guid_t iid = {.as_guid { 0x00000000, 0x0000, 0x0000, 0x46000000000000C0 }};
        virtual hresult_t stdcall QueryInterface(const guid_t& id, void** out) = 0;
        virtual std::uint32_t stdcall AddRef() = 0;
        virtual std::uint32_t stdcall Release() = 0;
    };

    struct variant_t;
    struct DISPPARAMS;
    struct EXCEPTINFO;

    struct CONNECTDATA
    {
      IUnknown* sink;
      std::uint32_t cookie;
    };

    struct ITypeInfo;

    struct IDispatch : public IUnknown
    {
        virtual hresult_t stdcall GetTypeInfoCount(std::uint32_t *pctinfo) = 0;
        virtual hresult_t stdcall GetTypeInfo(std::uint32_t, std::uint32_t, ITypeInfo**) = 0;
        virtual hresult_t stdcall GetIDsOfNames(const guid_t&, char16_t**, std::uint32_t, std::uint32_t, std::uint32_t*) = 0;
        virtual hresult_t stdcall Invoke(std::uint32_t, const guid_t&, char16_t**, std::uint32_t, std::uint16_t, DISPPARAMS*, variant_t*, EXCEPTINFO*, std::uint32_t*) = 0;
    };

    enum class variant_bool : std::int16_t
    {
        variant_true = -1,
        variant_false = 0
    };

    struct IRecordInfo;

    struct variant_t
    {
        alignas(std::size_t) std::uint16_t type;
        union 
        {
            std::int8_t sbyte;
            std::int32_t int32;
            std::int16_t int16;
            std::int64_t int64;

            std::uint8_t byte;
            std::uint16_t uint16;
            std::uint32_t uint32;
            std::uint64_t uint64;
        
            float single_float;
            double double_float;
            variant_bool boolean;

            char16_t* string;
            IUnknown* unknown;
            IDispatch* dispatch;        
            void* void_ptr;
            struct {
                void* record_data;
                IRecordInfo* record_info;
            } record;
        } data;
    };

    void VariantInit(variant_t*);
    hresult_t VariantCopy(variant_t*, const variant_t*);
    hresult_t VariantClear(variant_t*);
    hresult_t VariantChangeType(variant_t*, const variant_t*, std::uint16_t, std::uint16_t);

    struct DISPPARAMS
    {
        variant_t* argv;
        std::uint16_t* named_argv;
        std::uint32_t argc;
        std::uint32_t named_argc;
    };

    struct EXCEPTINFO
    {
        alignas(std::uint32_t) std::uint16_t code;
        char16_t* source;
        char16_t* description;
        char16_t* help_file;
        std::uint32_t help_context;
        void* reserved;
        void* deferred_fill_in;
        std::int32_t scode;
    };

    static_assert(std::is_trivial_v<variant_t>);
    static_assert(std::is_standard_layout_v<variant_t>);
    static_assert(offsetof(variant_t, variant_t::data) == sizeof(std::size_t));
    static_assert(sizeof(variant_t) == sizeof(std::array<void*, 3>));

    struct IConnectionPoint;

    template<typename TOut>
    struct IEnum : public IUnknown
    {
        virtual hresult_t stdcall Clone(IEnum**) = 0;
        virtual hresult_t stdcall Next(std::uint32_t, TOut*, std::uint32_t*) = 0;
        virtual hresult_t stdcall Reset() = 0;
        virtual hresult_t stdcall Skip(std::uint32_t) = 0;
    };

    struct STATSTG;

    using IEnumvariant_t = IEnum<variant_t>;
    using IEnumString = IEnum<char16_t*>;
    using IEnumUnknown = IEnum<IUnknown*>;
    using IEnumConnections = IEnum<CONNECTDATA>;
    using IEnumConnectionPoints = IEnum<IConnectionPoint*>;
    using IEnumGUID = IEnum<guid_t>;
    using IEnumSTATSTG = IEnum<STATSTG>;

    struct IConnectionPoint : public IUnknown
    {
        virtual hresult_t stdcall Advise(IUnknown* sink, std::uint32_t* cookie) = 0;
        virtual hresult_t stdcall EnumConnections(IEnumConnections**) = 0;
        virtual hresult_t stdcall GetConnectionInterface(guid_t*) = 0;
        virtual hresult_t stdcall Unadvise(std::uint32_t cookie) = 0;
    };

    struct IConnectionPointContainer : public IUnknown
    {
        virtual hresult_t stdcall FindConnectionPoint(const guid_t&, IConnectionPoint**) = 0;
        virtual hresult_t stdcall EnumConnectionPoints(IEnumConnectionPoints**) = 0;
    };

    struct ILockBytes : public IUnknown
    {
      virtual hresult_t stdcall Flush() = 0;
      virtual hresult_t stdcall LockRegion() = 0;
      virtual hresult_t stdcall ReadAt() = 0;
      virtual hresult_t stdcall SetSize() = 0;
      virtual hresult_t stdcall Stat() = 0;
      virtual hresult_t stdcall UnlockRegion() = 0;
      virtual hresult_t stdcall WriteAt() = 0;
    };

    struct ISequentialStream : public IUnknown
    {
      virtual hresult_t stdcall Read(void*, std::uint32_t, std::uint32_t*) = 0;
      virtual hresult_t stdcall Write(const void*, std::uint32_t, std::uint32_t*) = 0;
    };

    struct IStream : public ISequentialStream
    {
      virtual hresult_t stdcall Clone(IStream*) = 0;
      virtual hresult_t stdcall Commit(std::uint32_t) = 0;
      virtual hresult_t stdcall CopyTo(IStream*, std::uint64_t, std::uint64_t*, std::uint64_t*) = 0;
      virtual hresult_t stdcall LockRegion(std::uint64_t, std::uint64_t, std::uint32_t) = 0;
      virtual hresult_t stdcall Revert() = 0;
      virtual hresult_t stdcall Seek(std::int64_t, std::uint32_t, std::uint64_t*) = 0;
      virtual hresult_t stdcall SetSize(std::uint64_t) = 0;
      virtual hresult_t stdcall Stat(STATSTG*, std::uint32_t) = 0;
      virtual hresult_t stdcall UnlockRegion(std::uint64_t, std::uint64_t, std::uint32_t) = 0;
    };

    struct IStorage : public IUnknown
    {
      virtual hresult_t stdcall Commit(std::uint32_t) = 0;
      virtual hresult_t stdcall CopyTo(std::uint32_t, const guid_t*, char16_t**, IStorage*) = 0;
      virtual hresult_t stdcall CreateStorage(char16_t**, std::uint32_t, std::uint32_t, std::uint32_t, IStorage**) = 0;
      virtual hresult_t stdcall CreateStream(char16_t**, std::uint32_t, std::uint32_t, std::uint32_t, IStream**) = 0;
      virtual hresult_t stdcall DestroyElement(char16_t*) = 0;
      virtual hresult_t stdcall EnumElements(std::uint32_t, void*, std::uint32_t, IEnumSTATSTG*) = 0;
      virtual hresult_t stdcall MoveElementTo(char16_t*, IStorage*, char16_t*, std::uint32_t) = 0;
      virtual hresult_t stdcall OpenStorage(char16_t*, IStorage*, std::uint32_t, char16_t, std::uint32_t, IStorage**) = 0;
      virtual hresult_t stdcall OpenStream(char16_t*, void*, std::uint32_t, std::uint32_t, IStream*) = 0;
      virtual hresult_t stdcall RenameElement(char16_t*, char16_t*) = 0;
      virtual hresult_t stdcall Revert() = 0;
      virtual hresult_t stdcall SetClass(const guid_t&) = 0;
      virtual hresult_t stdcall SetElementTimes(const char16_t, void*, void*, void*) = 0;
      virtual hresult_t stdcall SetStateBits(std::uint32_t, std::uint32_t) = 0;
      virtual hresult_t stdcall Stat(STATSTG*, std::uint32_t) = 0;
    };

    struct PROPERTYKEY;
    struct PROPVARIANT;

    struct IPropertyStore : public IUnknown
    {
      virtual hresult_t stdcall Commit() = 0;
      virtual hresult_t stdcall GetAt(std::uint32_t, PROPERTYKEY*) = 0;
      virtual hresult_t stdcall GetCount(std::uint32_t*) = 0;
      virtual hresult_t stdcall GetValue(PROPERTYKEY, PROPVARIANT*) = 0;
      virtual hresult_t stdcall SetValue(PROPERTYKEY&, PROPVARIANT&) = 0;
    };

    struct INamedPropertyStore : public IUnknown
    {
      virtual hresult_t stdcall GetNameAt(std::uint32_t, char16_t*) = 0;
      virtual hresult_t stdcall GetNameCount(std::uint32_t*) = 0;
      virtual hresult_t stdcall GetNamedValue(char16_t*, PROPVARIANT*) = 0;
      virtual hresult_t stdcall SetNamedValue(char16_t*, PROPVARIANT&) = 0;
    };

    hresult_t PSCreateMemoryPropertyStore(const guid_t&, void**);

}