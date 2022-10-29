#ifndef DARKSTAR_HOOK_SIMGUIPLUGIN_HPP
#define DARKSTAR_HOOK_SIMGUIPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct SimGuiPlugin : GamePluginWrapper
    {
        void GuiInspect(bool shouldShow = true)
        {
            execute(plugin, 0, { "GuiInspect", toStringLower(shouldShow) });
        }

        void EditMode(bool shouldShow = true)
        {
            execute(plugin, 1, { "GuiEditMode", toStringLower(shouldShow) });
        }

        void EditNewControl(bool shouldShow = true)
        {
            execute(plugin, 2, { "GuiEditNewControl", toStringLower(shouldShow) });
        }

        void SetCurrentAddSet(bool shouldShow = true)
        {
            execute(plugin, 3, { "GuiSetAddSet", toStringLower(shouldShow) });
        }

        void SetSelection(bool shouldShow = true)
        {
            execute(plugin, 4, { "GuiSetSelection", toStringLower(shouldShow) });
        }

        void NewContent(bool shouldShow = true)
        {
            execute(plugin, 5, { "GuiNewContentCtrl", toStringLower(shouldShow) });
        }

        void LoadContent(bool shouldShow = true)
        {
            execute(plugin, 6, { "GuiLoadContentCtrl", toStringLower(shouldShow) });
        }

        void SaveContent(bool shouldShow = true)
        {
            execute(plugin, 7, { "GuiSaveContentCtrl", toStringLower(shouldShow) });
        }

        void SaveSelection(bool shouldShow = true)
        {
            execute(plugin, 8, { "GuiSaveSelection", toStringLower(shouldShow) });
        }

        void LoadSelection(bool shouldShow = true)
        {
            execute(plugin, 9, { "GuiLoadSelection", toStringLower(shouldShow) });
        }

        void GuiJustify(bool shouldShow = true)
        {
            execute(plugin, 10, { "GuiJustify", toStringLower(shouldShow) });
        }

        void GuiToolbar(bool shouldShow = true)
        {
            execute(plugin, 11, { "GuiToolbar", toStringLower(shouldShow) });
        }

        void GuiSendToBack(bool shouldShow = true)
        {
            execute(plugin, 12, { "GuiSendToBack", toStringLower(shouldShow) });
        }

        void GuiBringToFront(bool shouldShow = true)
        {
            execute(plugin, 13, { "GuiBringToFront", toStringLower(shouldShow) });
        }

        void SendRootMessage(bool shouldShow = true)
        {
            execute(plugin, 14, { "GuiSendRootMessage", toStringLower(shouldShow) });
        }

        void SetCCursor(bool shouldShow = true)
        {
            execute(plugin, 15, { "setCursor", toStringLower(shouldShow) });
        }

        void IsCursorOn(bool shouldShow = true)
        {
            execute(plugin, 16, { "isCursorOn", toStringLower(shouldShow) });
        }

        void CursorOn(bool shouldShow = true)
        {
            execute(plugin, 17, { "cursorOn", toStringLower(shouldShow) });
        }

        void CursorOff(bool shouldShow = true)
        {
            execute(plugin, 18, { "cursorOff", toStringLower(shouldShow) });
        }

        void WindowsMouseEnable(bool shouldShow = true)
        {
            execute(plugin, 19, { "windowsMouseEnable", toStringLower(shouldShow) });
        }

        void WindowsMouseDisable(bool shouldShow = true)
        {
            execute(plugin, 20, { "windowsMouseDisable", toStringLower(shouldShow) });
        }

        void GuiPushDialog(bool shouldShow = true)
        {
            execute(plugin, 21, { "GuiPushDialog", toStringLower(shouldShow) });
        }

        void GuiPopDialog(bool shouldShow = true)
        {
            execute(plugin, 22, { "GuiPopDialog", toStringLower(shouldShow) });
        }

        void GuiSetValue(bool shouldShow = true)
        {
            execute(plugin, 23, { "Control::setValue", toStringLower(shouldShow) });
        }

        void GuiGetValue(bool shouldShow = true)
        {
            execute(plugin, 24, { "Control::getValue", toStringLower(shouldShow) });
        }

        void GuiSetActive(bool shouldShow = true)
        {
            execute(plugin, 25, { "Control::setActive", toStringLower(shouldShow) });
        }

        void GuiGetActive(bool shouldShow = true)
        {
            execute(plugin, 26, { "Control::getActive", toStringLower(shouldShow) });
        }

        void GuiSetVisible(bool shouldShow = true)
        {
            execute(plugin, 27, { "Control::setVisible", toStringLower(shouldShow) });
        }

        void GuiGetVisible(bool shouldShow = true)
        {
            execute(plugin, 28, { "Control::getVisible", toStringLower(shouldShow) });
        }

        void GuiPerformClick(bool shouldShow = true)
        {
            execute(plugin, 29, { "Control::performClick", toStringLower(shouldShow) });
        }

        void GuiSetText(bool shouldShow = true)
        {
            execute(plugin, 30, { "Control::setText", toStringLower(shouldShow) });
        }

        void GuiGetText(bool shouldShow = true)
        {
            execute(plugin, 31, { "Control::getText", toStringLower(shouldShow) });
        }

        void TextListClear(bool shouldShow = true)
        {
            execute(plugin, 32, { "TextList::Clear", toStringLower(shouldShow) });
        }

        void TextListAdd(bool shouldShow = true)
        {
            execute(plugin, 33, { "TextList::AddLine", toStringLower(shouldShow) });
        }

        void WindowsKeyboardEnable(bool shouldShow = true)
        {
            execute(plugin, 34, { "windowsKeyboardEnable", toStringLower(shouldShow) });
        }

        void WindowsKeyboardDisable(bool shouldShow = true)
        {
            execute(plugin, 35, { "windowsKeyboardDisable", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTAR_HOOK_SIMGUIPLUGIN_HPP
