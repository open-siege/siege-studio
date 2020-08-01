#ifndef DARKSTARDTSCONVERTER_JSON_BOOST_HPP
#define DARKSTARDTSCONVERTER_JSON_BOOST_HPP

#include <algorithm>
#include <type_traits>
#include <boost/endian/arithmetic.hpp>
#include <nlohmann/json.hpp>

#define NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(Type, ...)  \
    inline void to_json(nlohmann::ordered_json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) }

namespace nlohmann {
    template<std::size_t Size>
    struct adl_serializer<std::array<char, Size>> {
        template<typename BasicJsonType>
        static void to_json(BasicJsonType &j, const std::array<char, Size> &opt) {
            j = std::string(&opt[0]);
        }

        template<typename BasicJsonType>
        static void from_json(const BasicJsonType &j, std::array<char, Size> &opt) {
            auto result = j.get<std::string>();
            auto end = result.size() < Size ? result.end() : result.begin() + Size - 1;

            std::fill(opt.begin(), opt.end(), '\0');
            std::copy(result.begin(), end, opt.begin());
        }
    };

    template<boost::endian::order ByteOrder, typename IntType, std::size_t Size>
    struct adl_serializer<boost::endian::endian_arithmetic<ByteOrder, IntType, Size>> {
        using EndianType = boost::endian::endian_arithmetic<ByteOrder, IntType, Size>;

        template<typename BasicJsonType>
        static void to_json(BasicJsonType &j, const EndianType &opt) {
            j = static_cast<EndianType::value_type>(opt);
        }

        template<typename BasicJsonType>
        static void from_json(const BasicJsonType &j, EndianType &opt) {
            opt = j.get<EndianType::value_type>();
        }
    };
}

namespace darkstar {
    template<std::size_t Size>
    constexpr std::array<std::string_view, Size> make_keys(const char *(&&keys)[Size]) {
        std::array<std::string_view, Size> result;
        for (auto i = 0; i < Size; i++) {
            result[i] = keys[i];
        }
        return result;
    }


}

namespace darkstar::dts {
    template<typename T, typename = int>
    struct has_struct_keys : std::false_type {
    };

    template<typename T>
    struct has_struct_keys<T, decltype((void) T::keys, 0)> : std::true_type {
    };

    template<typename StructType, typename = typename std::enable_if<has_struct_keys<StructType>::value, bool>::type>
    void to_json(nlohmann::ordered_json &j, const StructType &raw) {
        if constexpr (StructType::keys.size() == 2) {
            auto &keys = StructType::keys;
            auto&[item0, item1] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1}};
        }

        if constexpr (StructType::keys.size() == 3) {
            auto &keys = StructType::keys;
            auto&[item0, item1, item2] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1},
                                       {keys[2], item2}};
        }

        if constexpr (StructType::keys.size() == 4) {
            auto &keys = StructType::keys;
            auto&[item0, item1, item2, item3] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1},
                                       {keys[2], item2},
                                       {keys[3], item3}};
        }

        if constexpr (StructType::keys.size() == 5) {
            auto &keys = StructType::keys;
            auto&[item0, item1, item2, item3, item4] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1},
                                       {keys[2], item2},
                                       {keys[3], item3},
                                       {keys[4], item4}
            };
        }

        if constexpr (StructType::keys.size() == 6) {
            auto &keys = StructType::keys;
            auto&[item0, item1, item2, item3, item4, item5] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1},
                                       {keys[2], item2},
                                       {keys[3], item3},
                                       {keys[4], item4},
                                       {keys[5], item5}
            };
        }

        if constexpr (StructType::keys.size() == 7) {
            auto &keys = StructType::keys;
            auto&[item0, item1, item2, item3, item4, item5, item6] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1},
                                       {keys[2], item2},
                                       {keys[3], item3},
                                       {keys[4], item4},
                                       {keys[5], item5},
                                       {keys[6], item6}
            };
        }

        if constexpr (StructType::keys.size() == 8) {
            auto &keys = StructType::keys;
            auto&[item0, item1, item2, item3, item4, item5, item6, item7] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1},
                                       {keys[2], item2},
                                       {keys[3], item3},
                                       {keys[4], item4},
                                       {keys[5], item5},
                                       {keys[6], item6},
                                       {keys[7], item7}
            };
        }

        if constexpr (StructType::keys.size() == 9) {
            auto &keys = StructType::keys;
            auto&[item0, item1, item2, item3, item4, item5, item6, item7, item8] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1},
                                       {keys[2], item2},
                                       {keys[3], item3},
                                       {keys[4], item4},
                                       {keys[5], item5},
                                       {keys[6], item6},
                                       {keys[7], item7},
                                       {keys[8], item8}
            };
        }

        if constexpr (StructType::keys.size() == 10) {
            auto &keys = StructType::keys;
            auto&[item0, item1, item2, item3, item4, item5, item6, item7, item8, item9] = raw;
            j = nlohmann::ordered_json{{keys[0], item0},
                                       {keys[1], item1},
                                       {keys[2], item2},
                                       {keys[3], item3},
                                       {keys[4], item4},
                                       {keys[5], item5},
                                       {keys[6], item6},
                                       {keys[7], item7},
                                       {keys[8], item8},
                                       {keys[9], item9}
            };
        }
    }

    namespace shape::v7 {
        template<typename StructType, typename = typename std::enable_if<has_struct_keys<StructType>::value, bool>::type>
        void to_json(nlohmann::ordered_json &j, const StructType &raw) {
            darkstar::dts::to_json(j, raw);
        }
    }

    namespace mesh::v3 {
        template<typename StructType, typename = typename std::enable_if<has_struct_keys<StructType>::value, bool>::type>
        void to_json(nlohmann::ordered_json &j, const StructType &raw) {
            darkstar::dts::to_json(j, raw);
        }
    }

    namespace material_list::v3 {
        template<typename StructType, typename = typename std::enable_if<has_struct_keys<StructType>::value, bool>::type>
        void to_json(nlohmann::ordered_json &j, const StructType &raw) {
            darkstar::dts::to_json(j, raw);
        }
    }
}

#endif //DARKSTARDTSCONVERTER_JSON_BOOST_HPP
