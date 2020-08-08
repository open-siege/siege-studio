#ifndef DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
#define DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP

#include <variant>
#include <type_traits>
#include <nlohmann/json.hpp>
#include "json_boost.hpp"
#include "structures.hpp"

namespace nlohmann
{
  using material_list = darkstar::dts::material_list_variant;
  using mesh = darkstar::dts::mesh_variant;
  using shape = darkstar::dts::shape_variant;


  template<typename... Type>
  struct adl_serializer<std::variant<Type...>>
  {
    template<typename BasicJsonType>
    static void to_json(BasicJsonType &j, const std::variant<Type...> &opt)
    {
      std::visit([&](const auto &value) {
        j.emplace("version", std::remove_reference_t<decltype(value)>::version);
        darkstar::dts::to_json(j, value);
      },
        opt);
    }

    template<typename Struct, typename BasicJsonType, typename Variant>
    static void emplace_variant(const BasicJsonType &js, Variant &opt, int version)
    {
      if (version == Struct::version)
      {
        opt.template emplace<Struct>(js);
      }
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType &js, material_list &opt)
    {
      auto version = js.at("version");

      emplace_variant<darkstar::dts::material_list_v3>(js, opt, version);
      emplace_variant<darkstar::dts::material_list_v2>(js, opt, version);
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType &js, mesh &opt)
    {
      auto version = js.at("version");

      emplace_variant<darkstar::dts::mesh_v3>(js, opt, version);
      emplace_variant<darkstar::dts::mesh_v2>(js, opt, version);
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType &js, shape &opt)
    {
      auto version = js.at("version");

      emplace_variant<darkstar::dts::shape_v7>(js, opt, version);
      emplace_variant<darkstar::dts::shape_v6>(js, opt, version);
      emplace_variant<darkstar::dts::shape_v5>(js, opt, version);
      emplace_variant<darkstar::dts::shape_v2>(js, opt, version);
    }
  };
}// namespace nlohmann

#endif//DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
