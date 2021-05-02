#ifndef DARKSTARDTSCONVERTER_JSON_BOOST_HPP
#define DARKSTARDTSCONVERTER_JSON_BOOST_HPP

#include <algorithm>
#include <sstream>
#include <type_traits>
#include <nlohmann/json.hpp>
#include "endian_arithmetic.hpp"

namespace nlohmann
{
  template<std::size_t Size>
  struct adl_serializer<std::array<char, Size>>
  {
    template<typename BasicJsonType>
    static void to_json(BasicJsonType& json, const std::array<char, Size>& opt)
    {
      json = std::string(&opt[0]);
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType& js, std::array<char, Size>& opt)
    {
      auto result = js.template get<std::string>();
      constexpr auto actual_size = Size - 1;

      if (result.size() > actual_size)
      {
        std::stringstream msg;
        msg << "The value " << result << " is too long for the internal type it maps to. ";
        msg << "The maximum expected size is " << actual_size << " characters.";
        msg << "The size of " << result << " is " << result.size() << " characters.";

        throw BasicJsonType::other_error::create(int{}, msg.str());
      }

      auto end = result.size() < Size ? result.end() : result.begin() + actual_size;

      std::fill(opt.begin(), opt.end(), '\0');
      std::copy(result.begin(), end, opt.begin());
    }
  };

  template<boost::endian::order ByteOrder, typename IntType, std::size_t Size>
  struct adl_serializer<boost::endian::endian_arithmetic<ByteOrder, IntType, Size>>
  {
    using EndianType = boost::endian::endian_arithmetic<ByteOrder, IntType, Size>;

    template<typename BasicJsonType>
    static void to_json(BasicJsonType& j, const EndianType& opt)
    {
      j = static_cast<typename EndianType::value_type>(opt);
    }

    template<typename BasicJsonType>
    static void from_json(const BasicJsonType& j, EndianType& opt)
    {
      opt = j.template get<EndianType::value_type>();
    }
  };
}// namespace nlohmann

namespace studio::content
{
  template<typename T, typename = int>
  struct has_struct_keys : std::false_type
  {
  };

  template<typename T>
  struct has_struct_keys<T, decltype((void)T::keys, 0)> : std::true_type
  {
  };

  template<typename BasicJsonType, typename StringType, std::size_t Size, typename... Args>
  void from_json_impl(const BasicJsonType& json, std::array<StringType, Size> keys, Args&... args)
  {
    using key_type = typename BasicJsonType::object_t::key_type;
    std::size_t current_key = 0;
    (json.at(key_type(keys[current_key++])).get_to(args), ...);
  }

  template<typename BasicJsonType, typename StructType, typename = typename std::enable_if_t<has_struct_keys<StructType>::value>>
  void from_json(const BasicJsonType& json, StructType& raw)
  {
    if constexpr (StructType::keys.size() == 1)
    {
      auto& keys = StructType::keys;
      auto& [item0] = raw;

      from_json_impl(json, keys, item0);
    }

    if constexpr (StructType::keys.size() == 2)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1] = raw;

      from_json_impl(json, keys, item0, item1);
    }

    if constexpr (StructType::keys.size() == 3)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2] = raw;

      from_json_impl(json, keys, item0, item1, item2);
    }

    if constexpr (StructType::keys.size() == 4)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3);
    }

    if constexpr (StructType::keys.size() == 5)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4);
    }

    if constexpr (StructType::keys.size() == 6)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5);
    }

    if constexpr (StructType::keys.size() == 7)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6);
    }

    if constexpr (StructType::keys.size() == 8)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7);
    }

    if constexpr (StructType::keys.size() == 9)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8);
    }

    if constexpr (StructType::keys.size() == 10)
    {
      const auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9);
    }

    if constexpr (StructType::keys.size() == 11)
    {
      const auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10);
    }

    if constexpr (StructType::keys.size() == 12)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11);
    }

    if constexpr (StructType::keys.size() == 13)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12);
    }

    if constexpr (StructType::keys.size() == 14)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13);
    }

    if constexpr (StructType::keys.size() == 15)
    {
      const auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13, item14] = raw;

      from_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13, item14);
    }
  }

  template<typename BasicJsonType, typename StringType, std::size_t Size, typename... Args>
  void to_json_impl(BasicJsonType& json, std::array<StringType, Size> keys, Args&... args)
  {
    using key_type = typename BasicJsonType::object_t::key_type;
    std::size_t current_key = 0;
    (json.emplace(key_type(keys[current_key++]), args), ...);
  }


  template<typename BasicJsonType, typename StructType, typename = typename std::enable_if_t<has_struct_keys<StructType>::value>>
  void to_json(BasicJsonType& json, const StructType& raw)
  {
    if constexpr (StructType::keys.size() == 1)
    {
      auto& keys = StructType::keys;
      auto& [item0] = raw;
      to_json_impl(json, keys, item0);
    }

    if constexpr (StructType::keys.size() == 2)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1] = raw;
      to_json_impl(json, keys, item0, item1);
    }

    if constexpr (StructType::keys.size() == 3)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2] = raw;
      to_json_impl(json, keys, item0, item1, item2);
    }

    if constexpr (StructType::keys.size() == 4)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3);
    }

    if constexpr (StructType::keys.size() == 5)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4);
    }

    if constexpr (StructType::keys.size() == 6)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5);
    }

    if constexpr (StructType::keys.size() == 7)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6);
    }

    if constexpr (StructType::keys.size() == 8)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7);
    }

    if constexpr (StructType::keys.size() == 9)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8);
    }

    if constexpr (StructType::keys.size() == 10)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9);
    }

    if constexpr (StructType::keys.size() == 11)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10);
    }

    if constexpr (StructType::keys.size() == 12)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11);
    }

    if constexpr (StructType::keys.size() == 13)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12);
    }

    if constexpr (StructType::keys.size() == 14)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13);
    }

    if constexpr (StructType::keys.size() == 15)
    {
      auto& keys = StructType::keys;
      auto& [item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13, item14] = raw;
      to_json_impl(json, keys, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13, item14);
    }
  }
}// namespace studio::content

namespace studio::content::dts::darkstar
{
  namespace shape::v2
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace shape::v2

  namespace shape::v3
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace shape::v3

  namespace shape::v5
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace shape::v5

  namespace shape::v6
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace shape::v6

  namespace shape::v7
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace shape::v7

  namespace shape::v8
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace shape::v8

  namespace mesh::v1
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace mesh::v1

  namespace mesh::v2
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace mesh::v2

  namespace mesh::v3
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace mesh::v3

  namespace material_list::v2
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace material_list::v2

  namespace material_list::v3
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace material_list::v3

  namespace material_list::v4
  {
    using studio::content::to_json;
    using studio::content::from_json;
  }// namespace material_list::v4
}

#endif//DARKSTARDTSCONVERTER_JSON_BOOST_HPP
