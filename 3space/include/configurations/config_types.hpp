#ifndef SHARED_CONFIG_TYPES_HPP
#define SHARED_CONFIG_TYPES_HPP

#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <variant>
#include <cstdint>
#include <algorithm>

namespace studio::configurations
{
    struct key_type
    {
        inline key_type(std::string_view key) : data(std::array<std::string_view, 1>{key}) {}

        inline key_type(std::vector<std::string_view> key) : data(std::move(key)) {}

        inline key_type(std::initializer_list<std::string_view> key)
        {
            if (key.size() == 1)
            {
                data = std::array<std::string_view, 1>{*key.begin()};
            }
            else if (key.size() == 2)
            {
                data = std::array<std::string_view, 2>{*key.begin(), *std::next(key.begin())};
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
            }, data);
        }

        inline std::string_view at(std::size_t index) const 
        { 
            return std::visit([&](auto& a) -> std::string_view {
                if (index > a.size() - 1)
                {
                    return "";
                }

                return a[index];
            }, data);
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
                }, rhs.data);
            }, data);
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
            }, data);
        }

        std::variant<std::array<std::string_view, 1>, std::array<std::string_view, 2>, std::vector<std::string_view>> data;
    };

    struct config_value
    {
        inline explicit config_value(std::int8_t value) : data(value) {}
        inline explicit config_value(std::uint8_t value) : data(value) {}
        inline explicit config_value(std::uint16_t value) : data(value) {}
        inline explicit config_value(std::int16_t value) : data(value) {}
        inline explicit config_value(std::uint32_t value) : data(value) {}
        inline explicit config_value(std::int32_t value) : data(value) {}
        inline explicit config_value(std::uint64_t value) : data(value) {}
        inline explicit config_value(std::int64_t value) : data(value) {}
        inline explicit config_value(float value) : data(value) {}
        inline explicit config_value(double value) : data(value) {}
        inline explicit config_value(bool value) : data(value) {}
        inline explicit config_value(std::string value) : data(std::move(value)) {}

        inline explicit operator bool() const
        {
            if (std::holds_alternative<bool>(data))
            {
                return std::get<bool>(data);
            }

            return false;
        }

        inline bool operator==(const config_value& rhs) const
        { 
                    if (data.index() == rhs.data.index())
                    {
                        return data == rhs.data;
                    }

                    return std::visit([&](const auto& a) {
                        return std::visit([&](const auto& b) {

                            using AType = std::decay_t<decltype(a)>;
                            using BType = std::decay_t<decltype(b)>;
                            
                            if constexpr (std::is_integral_v<AType> && std::is_integral_v<BType>)
                            {
                                return a == b;
                            }

                            if constexpr (std::is_floating_point_v<AType> && std::is_floating_point_v<BType>)
                            {
                                return a == b;
                            }

                            return false;
                        }, rhs.data);
                    }, data);
        }

        template <typename StoredType> bool has() const
        {
            return std::holds_alternative<StoredType>(data);
        }

        template <typename StoredType> StoredType get() const
        {
            return std::get<StoredType>(data);
        }

        inline bool operator!=(const config_value& rhs) const
        { 
                    return !(*this == rhs); 
                
        }


        std::variant<std::string, std::int8_t, 
                                        std::uint8_t, 
                                        std::uint16_t, 
                                        std::int16_t, 
                                        std::uint32_t, 
                                        std::int32_t, 
                                        std::uint64_t, 
                                        std::int64_t, 
                                        float, 
                                        double, 
                                        bool> data;
    };
}

#endif