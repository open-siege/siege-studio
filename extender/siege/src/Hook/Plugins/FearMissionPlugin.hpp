//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_FEARMISSIONPLUGIN_HPP
#define DARKSTARHOOK_FEARMISSIONPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct FearMissionPlugin : GamePluginWrapper
    {
        void Create(bool shouldShow = true)
        {
            execute(plugin, 0, { "ME::Create", toStringLower(shouldShow) });
        }

        void RegisterType(bool shouldShow = true)
        {
            execute(plugin, 1, { "ME::RegisterType", toStringLower(shouldShow) });
        }

        void SetGrabMask(bool shouldShow = true)
        {
            execute(plugin, 2, { "ME::SetGrabMask", toStringLower(shouldShow) });
        }

        void SetPlaceMask(bool shouldShow = true)
        {
            execute(plugin, 3, { "ME::SetPlaceMask", toStringLower(shouldShow) });
        }

        void SetDefaultPlaceMask(bool shouldShow = true)
        {
            execute(plugin, 4, { "ME::SetDefaultPlaceMask", toStringLower(shouldShow) });
        }

        void GetConsoleOptions(bool shouldShow = true)
        {
            execute(plugin, 5, { "ME::GetConsoleOptions", toStringLower(shouldShow) });
        }

        void ObjectToCamera(bool shouldShow = true)
        {
            execute(plugin, 6, { "ME::ObjectToCamera", toStringLower(shouldShow) });
        }

        void CameraToObject(bool shouldShow = true)
        {
            execute(plugin, 7, { "ME::CameraToObject", toStringLower(shouldShow) });
        }

        void ObjectToSC(bool shouldShow = true)
        {
            execute(plugin, 8, { "ME::ObjectToSC", toStringLower(shouldShow) });
        }

        void CreateObject(bool shouldShow = true)
        {
            execute(plugin, 9, { "MissionCreateObject", toStringLower(shouldShow) });
        }

        void onSelected(bool shouldShow = true)
        {
            execute(plugin, 10, { "ME::onSelected", toStringLower(shouldShow) });
        }

        void onUnselected(bool shouldShow = true)
        {
            execute(plugin, 11, { "ME::onUnselected", toStringLower(shouldShow) });
        }

        void onSelectionCleared(bool shouldShow = true)
        {
            execute(plugin, 12, { "ME::onSelectionCleared", toStringLower(shouldShow) });
        }

        void onUseTerrainGrid(bool shouldShow = true)
        {
            execute(plugin, 13, { "ME::onUseTerrainGrid", toStringLower(shouldShow) });
        }

        void PlaceBookmark(bool shouldShow = true)
        {
            execute(plugin, 14, { "ME::PlaceBookmark", toStringLower(shouldShow) });
        }

        void GotoBookmark(bool shouldShow = true)
        {
            execute(plugin, 15, { "ME::GotoBookmark", toStringLower(shouldShow) });
        }

        void DeleteSelection(bool shouldShow = true)
        {
            execute(plugin, 16, { "ME::DeleteSelection", toStringLower(shouldShow) });
        }

        void CopySelection(bool shouldShow = true)
        {
            execute(plugin, 17, { "ME::CopySelection", toStringLower(shouldShow) });
        }

        void CutSelection(bool shouldShow = true)
        {
            execute(plugin, 18, { "ME::CutSelection", toStringLower(shouldShow) });
        }

        void PasteSelection(bool shouldShow = true)
        {
            execute(plugin, 19, { "ME::PasteSelection", toStringLower(shouldShow) });
        }

        void PasteFile(bool shouldShow = true)
        {
            execute(plugin, 20, { "ME::PasteFile", toStringLower(shouldShow) });
        }

        void DuplicateSelection(bool shouldShow = true)
        {
            execute(plugin, 21, { "ME::DuplicateSelection", toStringLower(shouldShow) });
        }

        void CreateGroup(bool shouldShow = true)
        {
            execute(plugin, 22, { "ME::CreateGroup", toStringLower(shouldShow) });
        }

        void DropSelection(bool shouldShow = true)
        {
            execute(plugin, 23, { "ME::DropSelection", toStringLower(shouldShow) });
        }

        void Undo(bool shouldShow = true)
        {
            execute(plugin, 24, { "ME::Undo", toStringLower(shouldShow) });
        }

        void Redo(bool shouldShow = true)
        {
            execute(plugin, 25, { "ME::Redo", toStringLower(shouldShow) });
        }

        void Save(bool shouldShow = true)
        {
            execute(plugin, 26, { "ME::Save", toStringLower(shouldShow) });
        }

        void MissionLight(bool shouldShow = true)
        {
            execute(plugin, 27, { "ME::MissionLight", toStringLower(shouldShow) });
        }

        void RebuildCommandMap(bool shouldShow = true)
        {
            execute(plugin, 28, { "ME::RebuildCommandMap", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_FEARMISSIONPLUGIN_HPP
