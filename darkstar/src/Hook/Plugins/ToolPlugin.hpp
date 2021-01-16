//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_TOOLPLUGIN_HPP
#define DARKSTARHOOK_TOOLPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct ToolPlugin : GamePluginWrapper
    {
        void newToolWindow(bool shouldShow = true)
        {
            execute(plugin, 0, { "newToolWindow", toStringLower(shouldShow) });
        }

        void newToolStrip(bool shouldShow = true)
        {
            execute(plugin, 1, { "newToolStrip", toStringLower(shouldShow) });
        }

        void listToolButtons(bool shouldShow = true)
        {
            execute(plugin, 2, { "listToolButtons", toStringLower(shouldShow) });
        }

        void listToolWindows(bool shouldShow = true)
        {
            execute(plugin, 3, { "listToolWindows", toStringLower(shouldShow) });
        }

        void hideToolWin(bool shouldShow = true)
        {
            execute(plugin, 4, { "hideToolWin", toStringLower(shouldShow) });
        }

        void showToolWin(bool shouldShow = true)
        {
            execute(plugin, 5, { "showToolWin", toStringLower(shouldShow) });
        }

        void showToolWinAll(bool shouldShow = true)
        {
            execute(plugin, 6, { "showToolWinAll", toStringLower(shouldShow) });
        }

        void setToolWinPos(bool shouldShow = true)
        {
            execute(plugin, 7, { "setToolWinPos", toStringLower(shouldShow) });
        }

        void addToolButton(bool shouldShow = true)
        {
            execute(plugin, 8, { "addToolButton", toStringLower(shouldShow) });
        }

        void delToolButton(bool shouldShow = true)
        {
            execute(plugin, 9, { "delToolButton", toStringLower(shouldShow) });
        }

        void addToolGap(bool shouldShow = true)
        {
            execute(plugin, 10, { "addToolGap", toStringLower(shouldShow) });
        }

        void setToolCommand(bool shouldShow = true)
        {
            execute(plugin, 11, { "setToolCommand", toStringLower(shouldShow) });
        }

        void setButtonHelp(bool shouldShow = true)
        {
            execute(plugin, 12, { "setButtonHelp", toStringLower(shouldShow) });
        }

        void isButtonDown(bool shouldShow = true)
        {
            execute(plugin, 13, { "isButtonDown", toStringLower(shouldShow) });
        }

        void addStatusBar(bool shouldShow = true)
        {
            execute(plugin, 14, { "addStatusBar", toStringLower(shouldShow) });
        }

        void delStatusBar(bool shouldShow = true)
        {
            execute(plugin, 15, { "delStatusBar", toStringLower(shouldShow) });
        }

        void setStatusField(bool shouldShow = true)
        {
            execute(plugin, 16, { "setStatusField", toStringLower(shouldShow) });
        }

        void getStatusField(bool shouldShow = true)
        {
            execute(plugin, 17, { "getStatusField", toStringLower(shouldShow) });
        }

        void clearStatusField(bool shouldShow = true)
        {
            execute(plugin, 18, { "clearStatusField", toStringLower(shouldShow) });
        }

        void setMainWindow(bool shouldShow = true)
        {
            execute(plugin, 19, { "setMainWindow", toStringLower(shouldShow) });
        }

        void editVar(bool shouldShow = true)
        {
            execute(plugin, 20, { "editVar", toStringLower(shouldShow) });
        }

        void edit2Vars(bool shouldShow = true)
        {
            execute(plugin, 21, { "edit2Vars", toStringLower(shouldShow) });
        }

        void confirmBox(bool shouldShow = true)
        {
            execute(plugin, 22, { "confirmBox", toStringLower(shouldShow) });
        }

        void openFile(bool shouldShow = true)
        {
            execute(plugin, 23, { "openFile", toStringLower(shouldShow) });
        }

        void saveFileAs(bool shouldShow = true)
        {
            execute(plugin, 24, { "saveFileAs", toStringLower(shouldShow) });
        }

        void browseBox(bool shouldShow = true)
        {
            execute(plugin, 25, { "browseBox", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_TOOLPLUGIN_HPP
