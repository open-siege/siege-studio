#ifndef DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
#define DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP

#include <variant>
#include <type_traits>
#include <boost/endian/arithmetic.hpp>
#include <nlohmann/json.hpp>
#include "json_boost.hpp"
#include "structures.hpp"

namespace nlohmann {
    using material_list = darkstar::dts::material_list_variant;
    using mesh = darkstar::dts::mesh_variant;
    using shape = darkstar::dts::shape_variant;


    template<typename... Type>
    struct adl_serializer<std::variant<Type...>>
    {
        template<typename BasicJsonType>
        static void to_json(BasicJsonType &j, const std::variant<Type...> &opt)
        {
            std::visit([&] (const auto& value)
            {
                j.emplace("version", std::remove_reference_t<decltype(value)>::version);
                darkstar::dts::to_json(j, value);
            }, opt);
        }

        template<typename BasicJsonType>
        static void from_json(const BasicJsonType &js, material_list &opt)
        {
            auto version = js.at("version");

            if (version == 3)
            {
                opt.emplace<darkstar::dts::material_list_v3>(js);
            }

            if (version == 2)
            {
                opt.emplace<darkstar::dts::material_list_v2>(js);
            }
        }

        template<typename BasicJsonType>
        static void from_json(const BasicJsonType &js, mesh &opt)
        {
            auto version = js.at("version");

            if (version == 3)
            {
                opt.emplace<darkstar::dts::mesh_v3>(js);
            }

            if (version == 2)
            {
                opt.emplace<darkstar::dts::mesh_v2>(js);
            }
        }

        template<typename BasicJsonType>
        static void from_json(const BasicJsonType &js, shape &opt)
        {
            auto version = js.at("version");

            if (version == 7)
            {
                opt.emplace<darkstar::dts::shape_v7>(js);
            }

            if (version == 6)
            {
                opt.emplace<darkstar::dts::shape_v6>(js);
            }

            if (version == 5)
            {
                opt.emplace<darkstar::dts::shape_v5>(js);
            }

            if (version == 2)
            {
                opt.emplace<darkstar::dts::shape_v2>(js);
            }
        }
    };
}

#endif //DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
