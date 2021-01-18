#ifndef DARKSTAR_HOOK_SHARED_HPP
#define DARKSTAR_HOOK_SHARED_HPP

#include <utility>
#include <optional>
#include <string>
#include "Hook/Plugins/GamePluginWrapper.hpp"

namespace Hook::Plugins
{
    constexpr const char* toString(bool someValue)
    {
        return someValue ? "True" : "False";
    }

    constexpr const char* toIntString(bool someValue)
    {
        return someValue ? "1" : "0";
    }

    constexpr const char* toStringLower(bool someValue)
    {
        return someValue ? "true" : "false";
    }

    constexpr bool toBool(std::string_view someValue)
    {
        return someValue == "True" || someValue == "true" || someValue == "TRUE";
    }

    constexpr bool toForcedBool(const char* someRawValue)
    {
        if (someRawValue == nullptr)
        {
            return true;
        }

        return toBool(std::string_view{someRawValue});
    }

    inline std::optional<bool> toOptionalBool(const char* someRawValue)
    {
        if (someRawValue == nullptr)
        {
            return std::nullopt;
        }

        return std::optional<bool>{toBool(std::string_view{someRawValue})};
    }

    template<std::size_t Size>
    const char* execute(Core::GamePlugin* plugin, std::int32_t callbackId, const char* (&&args)[Size])
    {
        return plugin->executeCallback(plugin->console, callbackId, Size, args);
    }

    inline const char* execute(Core::GamePlugin* plugin, std::int32_t callbackId, std::vector<const char*>& args)
    {
        return plugin->executeCallback(plugin->console, callbackId, args.size(), args.data());
    }
}


#endif //DARKSTAR_HOOK_SHARED_HPP
