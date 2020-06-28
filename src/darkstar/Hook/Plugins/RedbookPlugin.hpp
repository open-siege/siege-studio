//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_REDBOOKPLUGIN_HPP
#define DARKSTARHOOK_REDBOOKPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct RedbookPlugin : GamePluginWrapper
    {
        void newRedbook(bool shouldShow = true)
        {
            execute(plugin, 0, { "newRedbook", toStringLower(shouldShow) });
        }

        void rbOpen(bool shouldShow = true)
        {
            execute(plugin, 1, { "rbOpen", toStringLower(shouldShow) });
        }

        void rbClose(bool shouldShow = true)
        {
            execute(plugin, 2, { "rbClose", toStringLower(shouldShow) });
        }

        void rbEject(bool shouldShow = true)
        {
            execute(plugin, 3, { "rbEject", toStringLower(shouldShow) });
        }

        void rbRetract(bool shouldShow = true)
        {
            execute(plugin, 4, { "rbRetract", toStringLower(shouldShow) });
        }

        void rbGetStatus(bool shouldShow = true)
        {
            execute(plugin, 5, { "rbGetStatus", toStringLower(shouldShow) });
        }

        void rbGetTrackCount(bool shouldShow = true)
        {
            execute(plugin, 6, { "rbGetTrackCount", toStringLower(shouldShow) });
        }

        void rbGetTrackInfo(bool shouldShow = true)
        {
            execute(plugin, 7, { "rbGetTrackInfo", toStringLower(shouldShow) });
        }

        void rbGetTrackPosition(bool shouldShow = true)
        {
            execute(plugin, 8, { "rbGetTrackPosition", toStringLower(shouldShow) });
        }

        void rbPlay(bool shouldShow = true)
        {
            execute(plugin, 9, { "rbPlay", toStringLower(shouldShow) });
        }

        void rbStop(bool shouldShow = true)
        {
            execute(plugin, 10, { "rbStop", toStringLower(shouldShow) });
        }

        void rbPause(bool shouldShow = true)
        {
            execute(plugin, 11, { "rbPause", toStringLower(shouldShow) });
        }

        void rbResume(bool shouldShow = true)
        {
            execute(plugin, 12, { "rbResume", toStringLower(shouldShow) });
        }

        void rbSetVolume(bool shouldShow = true)
        {
            execute(plugin, 13, { "rbSetVolume", toStringLower(shouldShow) });
        }

        void rbGetVolume(bool shouldShow = true)
        {
            execute(plugin, 14, { "rbGetVolume", toStringLower(shouldShow) });
        }

        void rbSetPlayMode(bool shouldShow = true)
        {
            execute(plugin, 15, { "rbSetPlayMode", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_REDBOOKPLUGIN_HPP
