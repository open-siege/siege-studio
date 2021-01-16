//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTAR_HOOK_GAMEPLUGINWRAPPER_HPP
#define DARKSTAR_HOOK_GAMEPLUGINWRAPPER_HPP

#include "Core/EngineTypes.hpp"

namespace Hook::Plugins
{
    class GamePluginWrapper
    {
    protected:
        Core::GamePlugin* plugin;

    public:
        GamePluginWrapper(Core::GamePlugin* plugin) : plugin(plugin) {}
    };
}

#endif //DARKSTAR_HOOK_GAMEPLUGINWRAPPER_HPP
