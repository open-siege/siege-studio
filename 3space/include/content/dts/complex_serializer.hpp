#ifndef DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
#define DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP

#include <variant>
#include <type_traits>
#include <nlohmann/json.hpp>
#include "content/json_boost.hpp"
#include "darkstar_structures.hpp"
#include "3space.hpp"

namespace nlohmann
{
  using material_list = studio::content::dts::darkstar::material_list_variant;
  using mesh = studio::content::dts::darkstar::mesh_variant;
  using shape = studio::content::dts::darkstar::shape_variant;

  using three_space_shape = studio::content::dts::three_space::v1::shape_item;

  template<typename... Type>
  struct adl_serializer<std::variant<Type...>>
  {
    template<typename BasicJsonType>
    static void to_json(BasicJsonType& json, const std::variant<Type...>& opt)
    {
      std::visit([&](const auto& value) {
        json.emplace("version", std::remove_reference_t<decltype(value)>::version);
        json.emplace("typeName", std::remove_reference_t<decltype(value)>::type_name);
        studio::content::to_json(json, value);
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
      using namespace studio::content::dts::darkstar::material_list;
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
      using namespace studio::content::dts::darkstar::mesh;
      auto version = js.at("version");
      auto type_name = js.at("typeName");

      raise_type_error<v2::mesh, BasicJsonType>(type_name);

      emplace_variant<v2::mesh>(js, opt, type_name, version);
      emplace_variant<v3::mesh>(js, opt, type_name, version);
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType& js, shape& opt)
    {
      using namespace studio::content::dts::darkstar::shape;
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

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType& js, three_space_shape& opt)
    {
      using namespace studio::content::dts::three_space::v1;
      auto version = js.at("version");
      auto type_name = js.at("typeName");

      //raise_type_error<v2::shape, BasicJsonType>(type_name);

      emplace_variant<an_shape>(js, opt, type_name, version);
      emplace_variant<an_cyclic_sequence>(js, opt, type_name, version);
      emplace_variant<an_sequence>(js, opt, type_name, version);
      emplace_variant<an_anim_list>(js, opt, type_name, version);
      emplace_variant<base_part>(js, opt, type_name, version);
      emplace_variant<part_list>(js, opt, type_name, version);
      emplace_variant<bitmap_part>(js, opt, type_name, version);
      emplace_variant<detail_part>(js, opt, type_name, version);
      emplace_variant<bsp_part>(js, opt, type_name, version);
      emplace_variant<group>(js, opt, type_name, version);
      emplace_variant<bsp_group>(js, opt, type_name, version);
      emplace_variant<poly>(js, opt, type_name, version);
      emplace_variant<shaded_poly>(js, opt, type_name, version);
      emplace_variant<texture_for_poly>(js, opt, type_name, version);
      emplace_variant<solid_poly>(js, opt, type_name, version);
      emplace_variant<gouraud_poly>(js, opt, type_name, version);
    }
  };
}// namespace nlohmann

#endif//DARKSTARDTSCONVERTER_COMPLEX_SERIALIZER_HPP
