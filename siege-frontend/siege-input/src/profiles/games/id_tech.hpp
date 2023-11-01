#ifndef OPEN_SIEGE_TECH_HPP
#define OPEN_SIEGE_TECH_HPP

#include <string_view>
#include <variant>
#include <array>
#include <utility>
#include <algorithm>

namespace common
{
    using namespace std::literals;    
}

namespace playstation
{
    using namespace std::literals;
    constexpr static auto cross = "cross"sv;
    constexpr static auto circle = "circle"sv;
    constexpr static auto square = "square"sv;
    constexpr static auto triangle = "triangle"sv;
    constexpr static auto l1 = "l1"sv;
    constexpr static auto r1 = "r1"sv;
    constexpr static auto l2 = "l2"sv;
    constexpr static auto r2 = "r2"sv;
    constexpr static auto l3 = "l3"sv;
    constexpr static auto r3 = "r3"sv;
    constexpr static auto start = "start"sv;
    constexpr static auto select = "select"sv;
}

namespace xbox
{
    using namespace std::literals;
    constexpr static auto a = "a"sv;
    constexpr static auto b = "b"sv;
    constexpr static auto x = "x"sv;
    constexpr static auto y = "y"sv;
    constexpr static auto lb = "lb"sv;
    constexpr static auto rb = "rb"sv;
    constexpr static auto lt = "lt"sv;
    constexpr static auto rt = "rt"sv;
    constexpr static auto ls = "ls"sv;
    constexpr static auto rs = "rs"sv;
    constexpr static auto start = "start"sv;
    constexpr static auto select = "select"sv;
}


constexpr static auto xbox_mapping = std::array<std::array<std::string_view, 2>>{{
    { xbox::a, playstation::cross },
    { xbob::b, playstation::circle },
    { xbox::x, playstation::square },
    { xbox::y, playstation::triangle },
    { xbox::lb, playstation::l1 },
    { xbox::rb, playstation::r1 },
    { xbox::lt, playstation::l2 },
    { xbox::rt, playstation::r2 },
    { xbox::lt+, playstation::l2+ },
    { xbox::lt-, playstation::l2- },
    { xbox::rt+, playstation::r2+ },
    { xbox::rt-, playstation::r2- },
    { xbox::ls, playstation::l3 },
    { xbox::rs, playstation::r3 }
}};


constexpr std::string_view to_xbox(std::string_view value)
{
    auto iter = std::find_if(xbox_mapping.begin(), xbox_mapping.end(), [&](auto& mapping) {
        return mapping[1] == value;
    });

    if (iter != xbox_mapping.end())
    {
        return iter->at(0);
    };


    return value;
}

constexpr std::string_view to_playstation(std::string_view value)
{
    auto iter = std::find_if(xbox_mapping.begin(), xbox_mapping.end(), [&](auto& mapping) {
        return mapping[0] == value;
    });

    if (iter != xbox_mapping.end())
    {
        return iter->at(1);
    };


    return value;
}

#endif