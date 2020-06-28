//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_MISSIONPLUGIN_HPP
#define DARKSTARHOOK_MISSIONPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct MissionPlugin : GamePluginWrapper
    {
        void newMissionEditor(bool shouldShow = true)
        {
            execute(plugin, 0, { "MissionEditor", toStringLower(shouldShow) });
        }

        void MissionRegType(bool shouldShow = true)
        {
            execute(plugin, 1, { "MissionRegType", toStringLower(shouldShow) });
        }

        void MissionRegObject(bool shouldShow = true)
        {
            execute(plugin, 2, { "MissionRegObject", toStringLower(shouldShow) });
        }

        void MissionRegTerrain(bool shouldShow = true)
        {
            execute(plugin, 3, { "MissionRegTerrain", toStringLower(shouldShow) });
        }

        void MissionAddObject(bool shouldShow = true)
        {
            execute(plugin, 4, { "MissionAddObject", toStringLower(shouldShow) });
        }

        void MissionCreateObject(bool shouldShow = true)
        {
            execute(plugin, 5, { "MissionCreateObject", toStringLower(shouldShow) });
        }

        void MissionLoadObjects(bool shouldShow = true)
        {
            execute(plugin, 6, { "MissionLoadObjects", toStringLower(shouldShow) });
        }

        void newMissionGroup(bool shouldShow = true)
        {
            execute(plugin, 7, { "newMissionGroup", toStringLower(shouldShow) });
        }

        void newMission(bool shouldShow = true)
        {
            execute(plugin, 8, { "newMission", toStringLower(shouldShow) });
        }

        void addMissionButton(bool shouldShow = true)
        {
            execute(plugin, 9, { "addMissionButton", toStringLower(shouldShow) });
        }

        void removeMissionButton(bool shouldShow = true)
        {
            execute(plugin, 10, { "removeMissionButton", toStringLower(shouldShow) });
        }

        void removeMissionButtons(bool shouldShow = true)
        {
            execute(plugin, 11, { "removeMissionButtons", toStringLower(shouldShow) });
        }

        void setMissionButtonChecked(bool shouldShow = true)
        {
            execute(plugin, 12, { "setMissionButtonChecked", toStringLower(shouldShow) });
        }

        void setMissionButtonEnabled(bool shouldShow = true)
        {
            execute(plugin, 13, { "setMissionButtonEnabled", toStringLower(shouldShow) });
        }

        void isMissionButtonChecked(bool shouldShow = true)
        {
            execute(plugin, 14, { "isMissionButtonChecked", toStringLower(shouldShow) });
        }

        void isMissionButtonEnabled(bool shouldShow = true)
        {
            execute(plugin, 15, { "isMissionButtonEnabled", toStringLower(shouldShow) });
        }

        void setAutoSaveInterval(bool shouldShow = true)
        {
            execute(plugin, 16, { "missionSetAutoSaveInterval", toStringLower(shouldShow) });
        }

        void setAutoSaveName(bool shouldShow = true)
        {
            execute(plugin, 17, { "missionSetAutoSaveName", toStringLower(shouldShow) });
        }

        void saveObjectPersist(bool shouldShow = true)
        {
            execute(plugin, 18, { "missionSaveObjectPersist", toStringLower(shouldShow) });
        }

        void loadObjectPersist(bool shouldShow = true)
        {
            execute(plugin, 19, { "missionLoadObjectPersist", toStringLower(shouldShow) });
        }

        void undoMoveRotate(bool shouldShow = true)
        {
            execute(plugin, 20, { "missionUndoMoveRotate", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_MISSIONPLUGIN_HPP
