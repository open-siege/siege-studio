#ifndef SHARED_CONFIG_HPP
#define SHARED_CONFIG_HPP

#include <vector>
#include <array>
#include <variant>
#include <utility>
#include <string>
#include <string_view>
#include <cstdint>
#include <functional>

namespace studio::configurations
{
    struct binary_game_config
    {

    }

    class text_game_config
    {
        public:
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

                inline bool operator==(const key_type& rhs) 
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

                inline bool operator!=(const key_type& rhs) 
                { 
                    return !(*this == rhs); 
                
                }

                template<std::size_t ArraySize>
                inline std::array<std::string_view, ArraySize> to_array()
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
                    });
                }

                std::variant<std::array<std::string_view, 1>, std::array<std::string_view, 2>, std::vector<std::string_view>> data;
            };

            struct config_line
            {
                std::string_view raw_line;
                key_type key_segments;
                std::string_view value;
            };

            text_game_config() = default;
            text_game_config(std::string&&, std::vector<config_line>&&);
            text_game_config(const text_game_config&);
            text_game_config(text_game_config&&) = delete;

            std::vector<key_type> keys() const;
            std::string_view find(key_type key) const;
            void emplace(key_type key, std::string_view value);
            void remove(key_type key);

            void persist(std::function<void(const std::vector<config_line>&)>);

        private:
            std::string raw_data;
            std::vector<config_line> line_entries;
    };

    struct little_endian_stream_reader
    {
        std::istream& stream;

        std::uint32_t read_uint32();
        std::uint8_t read_uint8();
    };

    struct little_endian_stream_writer
    {
        std::ostream& stream;
        void write(const std::uint32_t&);
        void write(const std::uint8_t&);
        void write(const std::string_view);
    };
}

#endif