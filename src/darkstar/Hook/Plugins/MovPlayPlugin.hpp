//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_MOVPLAYPLUGIN_HPP
#define DARKSTARHOOK_MOVPLAYPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct MovPlayPlugin : GamePluginWrapper
    {
        void newMovPlay(bool shouldShow = true)
        {
            execute(plugin, 0, { "newMovPlay", toStringLower(shouldShow) });
        }

        void openMovie(bool shouldShow = true)
        {
            execute(plugin, 1, { "openMovie", toStringLower(shouldShow) });
        }

        void closeMovie(bool shouldShow = true)
        {
            execute(plugin, 2, { "closeMovie", toStringLower(shouldShow) });
        }

        void playMovie(bool shouldShow = true)
        {
            execute(plugin, 3, { "playMovie", toStringLower(shouldShow) });
        }

        void playMovieToComp(bool shouldShow = true)
        {
            execute(plugin, 4, { "playMovieToComp", toStringLower(shouldShow) });
        }

        void stopMovie(bool shouldShow = true)
        {
            execute(plugin, 5, { "stopMovie", toStringLower(shouldShow) });
        }

        void pauseMovie(bool shouldShow = true)
        {
            execute(plugin, 6, { "pauseMovie", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_MOVPLAYPLUGIN_HPP
