//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_SKYPLUGIN_HPP
#define DARKSTARHOOK_SKYPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct SkyPlugin : GamePluginWrapper
    {
        void globeLines()
        {
            execute(plugin, 0, { "globeLines" });
        }

        void showLowerStars(bool shouldShow = true)
        {
            execute(plugin, 1, { "showLowerStars", toStringLower(shouldShow) });
        }

        void setStarsVisibility(bool shouldShow = true)
        {
            execute(plugin, 2, { "setStarsVisibility", toStringLower(shouldShow) });
        }

        void setSkyMaterialListTag(int tag)
        {
            std::string tagStr = std::to_string(tag);
            execute(plugin, 3, { "setSkyMaterialListTag", tagStr.c_str() });
        }
    };
}

#endif //DARKSTARHOOK_SKYPLUGIN_HPP
