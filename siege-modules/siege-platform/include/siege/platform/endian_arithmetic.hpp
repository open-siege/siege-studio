#ifndef DARKSTARDTSCONVERTER_ENDIAN_ARITHMETIC_HPP
#define DARKSTARDTSCONVERTER_ENDIAN_ARITHMETIC_HPP

// Needed because warnings are treated as errors
// and there are some warnings from here.
// Will most likely need more pragmas for gcc as well.
#include <bit>
#include <array>
#include <cstdint>
#include <concepts>
#include <istream>
#include <ostream>
#include <string>

namespace siege::platform
{
    template<std::endian TByteOrder, std::integral TInt, std::size_t TIntSize = sizeof(TInt)>
    struct endian_int_t
    {
        using array_type = std::array<std::byte, TIntSize>;
        using value_type = TInt;
        using int_type = endian_int_t<TByteOrder, TInt, TIntSize>;
        array_type data;

        constexpr endian_int_t() noexcept : data{}
        {

        }

        constexpr endian_int_t(TInt value) noexcept
        {
            if constexpr (sizeof(TInt) == TIntSize)
            {
                if constexpr (std::endian::native != TByteOrder)
                {
                    data = std::bit_cast<array_type>(std::byteswap(value));
                    return;
                }

                data = std::bit_cast<array_type>(value);
            }
            else
            {
                using temp_array = std::array<std::byte, sizeof(TInt)>;
                temp_array temp;

                if constexpr (std::endian::native != TByteOrder)
                {
                    temp = std::bit_cast<temp_array>(std::byteswap(value));
                }
                else
                {
                    temp = std::bit_cast<temp_array>(value);
                }

                if constexpr (TByteOrder == std::endian::big)
                {
                    auto begin = temp.begin() + sizeof(TInt) - TIntSize;
                    std::copy(begin, begin + TIntSize, data.begin());
                }
                else
                {
                    std::copy(temp.begin(), temp.begin() + TIntSize, data.begin());
                }
            }
        }

        constexpr operator TInt() const noexcept
        {
            if constexpr (sizeof(TInt) == TIntSize)
            {
                if constexpr (std::endian::native != TByteOrder)
                {
                    return std::byteswap(std::bit_cast<TInt>(data));
                }

                return std::bit_cast<TInt>(data);
            }
            else
            {
                using temp_array = std::array<std::byte, sizeof(TInt)>;
                temp_array temp{};

                if constexpr (TByteOrder == std::endian::big)
                {
                    std::copy(data.begin(), data.end(), temp.begin() + sizeof(TInt) - TIntSize);
                }
                else
                {
                    std::copy(data.begin(), data.end(), temp.begin());
                }
                
                if constexpr (std::endian::native != TByteOrder)
                {
                    return std::byteswap(std::bit_cast<TInt>(temp));
                }

                return std::bit_cast<TInt>(temp);
            }
        }


        constexpr int_type& operator=(TInt value) noexcept
        {
            new (this) int_type(value);
            return *this;
            // if constexpr (std::endian::native != TByteOrder)
            // {
            //     data = std::bit_cast<array_type>(std::byteswap(value));
            //     return *this;
            // }

            // data = std::bit_cast<array_type>(value);
            // return *this;
        }

        constexpr TInt value() const noexcept
        {
            return TInt(*this);
        }


        template<std::integral OtherInt>
        constexpr operator OtherInt() const noexcept
        {
            return OtherInt(TInt(*this));
        }

        template<std::integral OtherInt>
        constexpr bool operator==(OtherInt b) const noexcept
        {
            return TInt(*this) == b;
        }

        template<std::integral OtherInt>
        constexpr bool operator<=(OtherInt b) const noexcept
        {
            return TInt(*this) <= b;
        }

        template<std::integral OtherInt>
        constexpr bool operator<(OtherInt b) const noexcept
        {
            return TInt(*this) < b;
        }

        template<std::integral OtherInt>
        friend constexpr bool operator<(OtherInt a, int_type b) noexcept
        {
            return a < TInt(b);
        }

        template<std::integral OtherInt>
        constexpr bool operator>=(OtherInt b) const noexcept
        {
            return TInt(*this) >= b;
        }

        template<std::integral OtherInt>
        friend constexpr bool operator>(OtherInt a, int_type b) noexcept
        {
            return a > TInt(b);
        }

        template<std::integral OtherInt>
        constexpr bool operator>(OtherInt b) const noexcept
        {
            return TInt(*this) > b;
        }

        constexpr int_type& operator++() noexcept
        {
            auto temp = TInt(*this);
            *this = ++temp;
            return *this;
        }

        constexpr int_type operator++(int) noexcept
        {
            TInt copy(*this);
            ++(*this);
            return copy;
        }

        constexpr int_type operator*(int_type b) const noexcept
        {
            return TInt(*this) * typename decltype (b)::value_type(b);
        }

        friend constexpr int_type operator*(int_type a, int_type b) noexcept
        {
            return typename decltype (b)::value_type(b) * TInt(b);
        }

        constexpr int_type operator+(int_type b) const noexcept
        {
            return TInt(*this) + typename decltype (b)::value_type(b);
        }

        friend constexpr int_type operator+(int_type a, int_type b) noexcept
        {
            return typename decltype (b)::value_type(b) + TInt(b);
        }

        constexpr int_type operator-(int_type b) const noexcept
        {
            return TInt(*this) - typename decltype (b)::value_type(b);
        }

        friend constexpr int_type operator-(int_type a, int_type b) noexcept
        {
            return typename decltype (b)::value_type(b) - TInt(b);
        }

        constexpr int_type operator/(int_type b) const noexcept
        {
            return TInt(*this) / typename decltype (b)::value_type(b);
        }

        friend constexpr int_type operator/(int_type a, int_type b) noexcept
        {
            return typename decltype (b)::value_type(b) / TInt(b);
        }

        template<std::integral OtherInt>
        constexpr OtherInt operator*(OtherInt b) const noexcept
        {
            return TInt(*this) * b;
        }

        template<std::integral OtherInt>
        friend constexpr OtherInt operator*(OtherInt a, int_type b) noexcept
        {
            return a * TInt(b);
        }


        template<std::integral OtherInt>
        constexpr OtherInt operator+(OtherInt b) const noexcept
        {
            return TInt(*this) + b;
        }

        template<std::integral OtherInt>
        friend constexpr OtherInt operator-(OtherInt a, int_type b) noexcept
        {
            return a - TInt(b);
        }

        template<std::integral OtherInt>
        constexpr OtherInt operator-(OtherInt b) const noexcept
        {
            return TInt(*this) - b;
        }

        template<std::integral OtherInt>
        friend constexpr OtherInt operator+(OtherInt a, int_type b) noexcept
        {
            return a + TInt(b);
        }

        template<std::integral OtherInt>
        constexpr OtherInt operator/(OtherInt b) const noexcept
        {
            return TInt(*this) / b;
        }

        template<std::integral OtherInt>
        friend constexpr OtherInt operator/(OtherInt a, int_type b) noexcept
        {
            return a / TInt(b);
        }

        friend constexpr std::string to_string(int_type value)
        {
            return std::to_string(TInt(value));
        }

        template <class charT, class traits>
      friend std::basic_ostream<charT, traits>&
        operator<<(std::basic_ostream<charT, traits>& os, const int_type& x)
        {
            os << TInt(x);
            return os;
        }

      // Stream extractor
      template <class charT, class traits>
      friend std::basic_istream<charT, traits>&
        operator>>(std::basic_istream<charT, traits>& is, int_type& x)
        {
            is >> TInt(x);
            return is;
        }
    };

    using little_int8_t = endian_int_t<std::endian::little, std::int8_t>;
    using little_uint8_t = endian_int_t<std::endian::little, std::uint8_t>;
    using little_int16_t = endian_int_t<std::endian::little, std::int16_t>;
    using little_uint16_t = endian_int_t<std::endian::little, std::uint16_t>;
    using little_int32_t = endian_int_t<std::endian::little, std::int32_t>;
    using little_uint32_t = endian_int_t<std::endian::little, std::uint32_t>;
    using little_int64_t = endian_int_t<std::endian::little, std::int64_t>;
    using little_uint64_t = endian_int_t<std::endian::little, std::uint64_t>;
    using little_int24_t = endian_int_t<std::endian::little, std::int32_t, 3>;
    using little_uint24_t = endian_int_t<std::endian::little, std::uint32_t, 3>;

    using big_int8_t = endian_int_t<std::endian::big, std::int8_t>;
    using big_uint8_t = endian_int_t<std::endian::big, std::uint8_t>;
    using big_int16_t = endian_int_t<std::endian::big, std::int16_t>;
    using big_uint16_t = endian_int_t<std::endian::big, std::uint16_t>;
    using big_int32_t = endian_int_t<std::endian::big, std::int32_t>;
    using big_uint32_t = endian_int_t<std::endian::big, std::uint32_t>;
    using big_int64_t = endian_int_t<std::endian::big, std::int64_t>;
    using big_uint64_t = endian_int_t<std::endian::big, std::uint64_t>;
    using big_int24_t = endian_int_t<std::endian::big, std::int32_t, 3>;
    using big_uint24_t = endian_int_t<std::endian::big, std::uint32_t, 3>;

    template<typename TEndianInt>
    consteval bool endian_int_t_test()
    {
        using RealInt = typename TEndianInt::value_type;
        static_assert(std::is_trivially_copyable_v<TEndianInt>);
        static_assert(sizeof(TEndianInt) == sizeof(RealInt));

        static_assert(TEndianInt(10) == 10);
        static_assert(TEndianInt(11) > 10);
        static_assert(TEndianInt(10) < 11);

        constexpr TEndianInt temp = 100;
        static_assert(temp.value() == 100);
        return true;
    }

    static_assert(endian_int_t_test<little_int8_t>());
    static_assert(endian_int_t_test<little_uint8_t>());
    static_assert(endian_int_t_test<little_int16_t>());
    static_assert(endian_int_t_test<little_uint16_t>());
    static_assert(endian_int_t_test<little_int32_t>());
    static_assert(endian_int_t_test<little_uint32_t>());
    static_assert(endian_int_t_test<little_int64_t>());
    static_assert(endian_int_t_test<little_uint64_t>());
    static_assert(endian_int_t_test<big_int8_t>());
    static_assert(endian_int_t_test<big_uint8_t>());
    static_assert(endian_int_t_test<big_int16_t>());
    static_assert(endian_int_t_test<big_uint16_t>());
    static_assert(endian_int_t_test<big_int32_t>());
    static_assert(endian_int_t_test<big_uint32_t>());
    static_assert(endian_int_t_test<big_int64_t>());
    static_assert(endian_int_t_test<big_uint64_t>());

    template<typename TEndianInt>
    consteval bool little_int24_t_test()
    {
        static_assert(std::is_trivially_copyable_v<TEndianInt>);

        constexpr static auto little_endian_10 = std::array<std::byte, 3>{ std::byte(0x0A), std::byte(0x00), std::byte(0x00) };
        static_assert(TEndianInt(10).data == little_endian_10);

        static_assert(TEndianInt(10) == 10);
        static_assert(TEndianInt(11) > 10);
        static_assert(TEndianInt(10) < 11);

        constexpr TEndianInt temp = 100;
        static_assert(temp.value() == 100);
        return true;
    }

    #ifndef _MSC_VER 
    static_assert(little_int24_t_test<little_int24_t>());
    static_assert(little_int24_t_test<little_uint24_t>());
    #endif


    template<typename TEndianInt>
    consteval bool big_int24_t_test()
    {
        static_assert(std::is_trivially_copyable_v<TEndianInt>);

        constexpr static auto big_endian_10 = std::array<std::byte, 3>{ std::byte(0x00), std::byte(0x00), std::byte(0x0A) };
        static_assert(TEndianInt(10).data == big_endian_10);

        static_assert(TEndianInt(10) == 10);
        static_assert(TEndianInt(11) > 10);
        static_assert(TEndianInt(10) < 11);

        constexpr TEndianInt temp = 100;
        static_assert(temp.value() == 100);
        return true;
    }

    #ifndef _MSC_VER 
    static_assert(big_int24_t_test<big_int24_t>());
    static_assert(big_int24_t_test<big_uint24_t>());
    #endif
}




#endif//DARKSTARDTSCONVERTER_ENDIAN_ARITHMETIC_HPP
