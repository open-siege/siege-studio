#ifndef SHARED_CONFIG_TYPES_HPP
#define SHARED_CONFIG_TYPES_HPP

#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <variant>
#include <cstdint>
#include <algorithm>

namespace siege::configuration
{
  struct key_type
  {
    inline key_type(std::string_view key) : data(std::array<std::string_view, 1>{ key }) {}

    inline key_type(std::vector<std::string_view> key) : data(std::move(key)) {}

    inline key_type(std::initializer_list<std::string_view> key)
    {
      if (key.size() == 1)
      {
        data = std::array<std::string_view, 1>{ *key.begin() };
      }
      else if (key.size() == 2)
      {
        data = std::array<std::string_view, 2>{ *key.begin(), *std::next(key.begin()) };
      }
      else
      {
        data = std::vector(key);
      }
    }

    inline bool operator==(std::string_view rhs) const
    {
      return std::visit([&](auto& a) {
        if (a.size() > 0)
        {
          return a[0] == rhs;
        }

        return false;
      },
        data);
    }

    inline bool empty() const
    {
      return std::visit([&](auto& a) -> bool {
        return a.empty();
      },
        data);
    }

    inline std::string_view at(std::size_t index) const
    {
      return std::visit([&](auto& a) -> std::string_view {
        if (a.empty())
        {
          return "";
        }

        if (index > a.size() - 1)
        {
          return "";
        }

        return a[index];
      },
        data);
    }

    inline bool operator==(const key_type& rhs) const
    {
      if (data.index() == rhs.data.index())
      {
        return data == rhs.data;
      }

      return std::visit([&](auto& a) {
        return std::visit([&](auto& b) {
          if (a.size() == b.size())
          {
            return std::equal(a.begin(), a.end(), b.begin());
          }

          return false;
        },
          rhs.data);
      },
        data);
    }

    inline bool operator!=(const key_type& rhs) const
    {
      return !(*this == rhs);
    }

    template<std::size_t ArraySize>
    inline std::array<std::string_view, ArraySize> to_array() const
    {
      if (std::holds_alternative<std::array<std::string_view, ArraySize>>(data))
      {
        return std::get<std::array<std::string_view, ArraySize>>(data);
      }

      return std::visit([](auto& value) {
        std::array<std::string_view, ArraySize> result;

        if (value.size() > ArraySize)
        {
          std::copy_n(value.begin(), ArraySize, result.begin());
        }
        else
        {
          std::copy_n(value.begin(), value.size(), result.begin());
        }

        return result;
      },
        data);
    }

    std::variant<std::array<std::string_view, 1>, std::array<std::string_view, 2>, std::vector<std::string_view>> data;
  };
}// namespace siege::configuration

#endif