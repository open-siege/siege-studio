//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_SIMTREEPLUGIN_HPP
#define DARKSTARHOOK_SIMTREEPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct SimTreePlugin : GamePluginWrapper
    {
        void simTreeCreate(bool shouldShow = true)
        {
            execute(plugin, 0, { "simTreeCreate", toStringLower(shouldShow) });
        }

        void simTreeAddSet(bool shouldShow = true)
        {
            execute(plugin, 1, { "simTreeAddSet", toStringLower(shouldShow) });
        }

        void simTreeRegBitmaps(bool shouldShow = true)
        {
            execute(plugin, 2, { "simTreeRegBitmaps", toStringLower(shouldShow) });
        }

        void simTreeRegClass(bool shouldShow = true)
        {
            execute(plugin, 3, { "simTreeRegClass", toStringLower(shouldShow) });
        }

        void simTreeRegScript(bool shouldShow = true)
        {
            execute(plugin, 4, { "simTreeRegScript", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_SIMTREEPLUGIN_HPP
