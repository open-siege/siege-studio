#ifndef DARKSTARDTSCONVERTER_JSON_BOOST_HPP
#define DARKSTARDTSCONVERTER_JSON_BOOST_HPP

#include <algorithm>
#include <boost/endian/arithmetic.hpp>
#include <nlohmann/json.hpp>

#define NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(Type, ...)  \
    inline void to_json(nlohmann::ordered_json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
    inline void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, __VA_ARGS__)) }

namespace nlohmann
{
    template<std::size_t Size>
    struct adl_serializer<std::array<char, Size>>
    {
        template<typename BasicJsonType>
        static void to_json(BasicJsonType &j, const std::array<char, Size> &opt)
        {
            j = std::string(&opt[0]);
        }

        template<typename BasicJsonType>
        static void from_json(const BasicJsonType &j, std::array<char, Size> &opt)
        {
            auto result = j.get<std::string>();
            auto end = result.size() < Size ? result.end()  : result.begin() + Size - 1;

            std::fill(opt.begin(), opt.end(), '\0');
            std::copy(result.begin(), end, opt.begin());
        }
    };

    template<boost::endian::order ByteOrder, typename IntType, std::size_t Size>
    struct adl_serializer<boost::endian::endian_arithmetic<ByteOrder, IntType, Size>>
    {
        using EndianType = boost::endian::endian_arithmetic<ByteOrder, IntType, Size>;

        template<typename BasicJsonType>
        static void to_json(BasicJsonType &j, const EndianType &opt)
        {
            j = static_cast<EndianType::value_type>(opt);
        }

        template<typename BasicJsonType>
        static void from_json(const BasicJsonType &j, EndianType &opt)
        {
            opt = j.get<EndianType::value_type>();
        }
    };
}

#endif //DARKSTARDTSCONVERTER_JSON_BOOST_HPP
