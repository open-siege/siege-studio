#ifndef STARSIEGE_PLUGINS_HPP
#define STARSIEGE_PLUGINS_HPP

#include <vector>
#include "Hook/Plugins/SimGuiConsolePlugin.hpp"
#include "Hook/Plugins/GfxPlugin.hpp"
#include "Hook/Plugins/TerrainPlugin.hpp"
#include "Hook/Plugins/InteriorPlugin.hpp"
#include "Hook/Plugins/SkyPlugin.hpp"
#include "Hook/Plugins/NetPlugin.hpp"
#include "Hook/Plugins/SoundFxPlugin.hpp"
#include "Hook/Plugins/RedbookPlugin.hpp"
#include "Hook/Plugins/MovPlayPlugin.hpp"
#include "Hook/Plugins/SimInputPlugin.hpp"
#include "Hook/Plugins/SimGuiPlugin.hpp"
#include "Hook/Plugins/ToolPlugin.hpp"
#include "Hook/Plugins/SimTreePlugin.hpp"
#include "Hook/Plugins/MissionPlugin.hpp"
#include "Hook/Plugins/FearMissionPlugin.hpp"
#include "Hook/Plugins/EnginePlugins.hpp"

namespace Hook::Plugins
{
    class StarsiegePlugins
    {
		std::vector<Core::GamePlugin*> _plugins;

    public:
        StarsiegePlugins(std::vector<Core::GamePlugin*> plugins) : _plugins(std::move(plugins)) {}

        SimGuiConsolePlugin guiConsole()
        {
            return SimGuiConsolePlugin{_plugins[0]};
        }

        GfxPlugin gfx()
        {
            return GfxPlugin{_plugins[1]};
        }

        TerrainPlugin terrain()
        {
            return TerrainPlugin{_plugins[2]};
        }

        InteriorPlugin interior()
        {
            return InteriorPlugin{_plugins[3]};
        }

        SkyPlugin sky()
        {
            return SkyPlugin{_plugins[4]};
        }

        NetPlugin net()
        {
            return NetPlugin{_plugins[5]};
        }

        SoundFxPlugin soundFx()
        {
            return SoundFxPlugin{_plugins[6]};
        }

        RedbookPlugin redbook()
        {
            return RedbookPlugin{_plugins[7]};
        }

        MovPlayPlugin movPlay()
        {
            return MovPlayPlugin{_plugins[8]};
        }

        SimInputPlugin input()
        {
            return SimInputPlugin{_plugins[9]};
        }

        SimGuiPlugin gui()
        {
            return SimGuiPlugin{_plugins[10]};
        }

        ToolPlugin tool()
        {
            return ToolPlugin{_plugins[11]};
        }

        SimTreePlugin tree()
        {
            return SimTreePlugin{_plugins[12]};
        }

        MissionPlugin mission()
        {
            return MissionPlugin{_plugins[13]};
        }

        FearMissionPlugin fearMission()
        {
            return FearMissionPlugin{_plugins[14]};
        }
    };
}

#endif