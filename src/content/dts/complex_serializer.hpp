#ifndef DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
#define DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP

#include <variant>
#include <type_traits>
#include <nlohmann/json.hpp>
#include "content/json_boost.hpp"
#include "darkstar_structures.hpp"

namespace nlohmann
{
  using material_list = darkstar::dts::material_list_variant;
  using mesh = darkstar::dts::mesh_variant;
  using shape = darkstar::dts::shape_variant;


  template<typename... Type>
  struct adl_serializer<std::variant<Type...>>
  {
    template<typename BasicJsonType>
    static void to_json(BasicJsonType& json, const std::variant<Type...>& opt)
    {
      std::visit([&](const auto& value) {
        json.emplace("version", std::remove_reference_t<decltype(value)>::version);
        json.emplace("typeName", std::remove_reference_t<decltype(value)>::type_name);
        darkstar::dts::to_json(json, value);
      },
        opt);
    }

    template<typename Struct, typename BasicJsonType, typename Variant>
    static void emplace_variant(const BasicJsonType& js, Variant& opt, std::string_view type_name, int version)
    {
      if (version == Struct::version && type_name == Struct::type_name)
      {
        opt.template emplace<Struct>(js);
      }
    }

    template<typename Struct, typename BasicJsonType>
    static void raise_type_error(std::string_view type_name)
    {
      if (type_name != Struct::type_name)
      {
        std::stringstream msg;
        msg << "The type name " << type_name << " is a misspelled or unsupported type. ";

        throw BasicJsonType::other_error::create(int{}, msg.str());
      }
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType& js, material_list& opt)
    {
      using namespace darkstar::dts::material_list;
      auto version = js.at("version");
      auto type_name = js.at("typeName");

      raise_type_error<v2::material_list, BasicJsonType>(type_name);

      emplace_variant<v2::material_list>(js, opt, type_name, version);
      emplace_variant<v3::material_list>(js, opt, type_name, version);
      emplace_variant<v4::material_list>(js, opt, type_name, version);
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType& js, mesh& opt)
    {
      using namespace darkstar::dts::mesh;
      auto version = js.at("version");
      auto type_name = js.at("typeName");

      raise_type_error<v2::mesh, BasicJsonType>(type_name);

      emplace_variant<v2::mesh>(js, opt, type_name, version);
      emplace_variant<v3::mesh>(js, opt, type_name, version);
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType& js, shape& opt)
    {
      using namespace darkstar::dts::shape;
      auto version = js.at("version");
      auto type_name = js.at("typeName");

      raise_type_error<v2::shape, BasicJsonType>(type_name);

      emplace_variant<v2::shape>(js, opt, type_name, version);
      emplace_variant<v3::shape>(js, opt, type_name, version);
      emplace_variant<v5::shape>(js, opt, type_name, version);
      emplace_variant<v6::shape>(js, opt, type_name, version);
      emplace_variant<v7::shape>(js, opt, type_name, version);
      emplace_variant<v8::shape>(js, opt, type_name, version);
    }
  };
}// namespace nlohmann

#endif//DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
