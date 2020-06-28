//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_SIMGUICONSOLEPLUGIN_HPP
#define DARKSTARHOOK_SIMGUICONSOLEPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct SimGuiConsolePlugin : GamePluginWrapper
    {
        bool consoleEnable(std::optional<bool> shouldEnable)
        {
            if (shouldEnable.has_value()) {
                return toBool(execute(plugin, 3, { "consoleEnable", toString(*shouldEnable) }));
            }

            return execute(plugin, 3, { "consoleEnable" });
        }
    };
}

#endif //DARKSTARHOOK_SIMGUICONSOLEPLUGIN_HPP
