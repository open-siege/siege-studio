#ifndef DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
#define DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP

#include <variant>
#include <type_traits>
#include <boost/endian/arithmetic.hpp>
#include <nlohmann/json.hpp>
#include "json_boost.hpp"
#include "structures.hpp"

namespace nlohmann {
    using material_list = std::variant<darkstar::dts::material_list_v2, darkstar::dts::material_list_v3>;

    template<>
    struct adl_serializer<material_list>
    {
        template<typename BasicJsonType>
        static void to_json(BasicJsonType &j, const material_list &opt)
        {
            std::visit([&] (const auto& value)
            {
                j.emplace("version", std::remove_reference_t<decltype(value)>::version);
                darkstar::dts::to_json(j, value);
            }, opt);
        }

        template<typename BasicJsonType>
        static void from_json(const BasicJsonType &j, material_list &opt)
        {
            auto version = j.at("version");

            if (version == 3)
            {
                darkstar::dts::material_list_v3 value = j;
                opt = value;
            }

            if (version == 2)
            {
                darkstar::dts::material_list_v2 value = j;
                opt = value;
            }
        }
    };
}

#endif //DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
