//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_SIMINPUTPLUGIN_HPP
#define DARKSTARHOOK_SIMINPUTPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct SimInputPlugin : GamePluginWrapper
    {
        void newInputManager(bool shouldShow = true)
        {
            execute(plugin, 0, { "newInputManager", toStringLower(shouldShow) });
        }

        void listInputDevices(bool shouldShow = true)
        {
            execute(plugin, 1, { "listInputDevices", toStringLower(shouldShow) });
        }

        void getInputDeviceInfo(bool shouldShow = true)
        {
            execute(plugin, 2, { "getInputDeviceInfo", toStringLower(shouldShow) });
        }

        void saveInputDeviceInfo(bool shouldShow = true)
        {
            execute(plugin, 3, { "saveInputDeviceInfo", toStringLower(shouldShow) });
        }

        void inputOpen(bool shouldShow = true)
        {
            execute(plugin, 4, { "inputOpen", toStringLower(shouldShow) });
        }

        void inputClose(bool shouldShow = true)
        {
            execute(plugin, 5, { "inputClose", toStringLower(shouldShow) });
        }

        void inputCapture(bool shouldShow = true)
        {
            execute(plugin, 6, { "inputCapture", toStringLower(shouldShow) });
        }

        void inputRelease(bool shouldShow = true)
        {
            execute(plugin, 7, { "inputRelease", toStringLower(shouldShow) });
        }

        void inputActivate(bool shouldShow = true)
        {
            execute(plugin, 8, { "inputActivate", toStringLower(shouldShow) });
        }

        void inputDeactivate(bool shouldShow = true)
        {
            execute(plugin, 9, { "inputDeactivate", toStringLower(shouldShow) });
        }

        void editActionMap(bool shouldShow = true)
        {
            execute(plugin, 10, { "editActionMap", toStringLower(shouldShow) });
        }

        void newActionMap(bool shouldShow = true)
        {
            execute(plugin, 11, { "newActionMap", toStringLower(shouldShow) });
        }

        void bindAction(bool shouldShow = true)
        {
            execute(plugin, 12, { "bindAction", toStringLower(shouldShow) });
        }

        void bindCommand(bool shouldShow = true)
        {
            execute(plugin, 13, { "bindCommand", toStringLower(shouldShow) });
        }

        void bind(bool shouldShow = true)
        {
            execute(plugin, 14, { "bind", toStringLower(shouldShow) });
        }

        void saveActionMap(bool shouldShow = true)
        {
            execute(plugin, 15, { "saveActionMap", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_SIMINPUTPLUGIN_HPP
