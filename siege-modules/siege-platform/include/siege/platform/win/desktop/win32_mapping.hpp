#ifndef WIN32_MAPPING_HPP
#define WIN32_MAPPING_HPP

#include "win32_controls.hpp"

namespace win32
{

    // TODO HDITEMW
    // TODO list view column
    // TODO list view group
    TBBUTTON ToTButton(std::wstring_view text)
    {
        return TBBUTTON{};
    }

    TBBUTTON ToTButton(TBBUTTON item)
    {
        return TBBUTTON{};
    }

    TBBUTTON ToTButton(LVITEMW item)
    {
        return TBBUTTON{};
    }

    TBBUTTON ToTButton(COMBOBOXEXITEMW item)
    {
        return TBBUTTON{};
    }

    TBBUTTON ToTButton(TVINSERTSTRUCTW item)
    {
        return TBBUTTON{};
    }

    LVITEMW ToListViewItem(std::wstring_view text)
    {
        return LVITEMW{};
    }

    LVITEMW ToListViewItem(TBBUTTON item)
    {
        return LVITEMW{};
    }

    LVITEMW ToListViewItem(LVITEMW item)
    {
        return LVITEMW{};
    }

    LVITEMW ToListViewItem(COMBOBOXEXITEMW item)
    {
        return LVITEMW{};
    }

    LVITEMW ToListViewItem(TVINSERTSTRUCTW item)
    {
        return LVITEMW{};
    }

    COMBOBOXEXITEMW ToComboBoxExItem(std::wstring_view text)
    {
        return COMBOBOXEXITEMW{};
    }

    COMBOBOXEXITEMW ToComboBoxExItem(TBBUTTON item)
    {
        return COMBOBOXEXITEMW{};
    }

    COMBOBOXEXITEMW ToComboBoxExItem(LVITEMW item)
    {
        return COMBOBOXEXITEMW{};
    }

    COMBOBOXEXITEMW ToComboBoxExItem(COMBOBOXEXITEMW item)
    {
        return COMBOBOXEXITEMW{};
    }

    COMBOBOXEXITEMW ToComboBoxExItem(TVINSERTSTRUCTW item)
    {
        return COMBOBOXEXITEMW{};
    }

    TVINSERTSTRUCTW ToTreeViewItem(std::wstring_view text)
    {
        return TVINSERTSTRUCTW{};
    }

    TVINSERTSTRUCTW ToTreeViewItem(TBBUTTON item)
    {
        return TVINSERTSTRUCTW{};
    }

    TVINSERTSTRUCTW ToTreeViewItem(LVITEMW item)
    {
        return TVINSERTSTRUCTW{};
    }

    TVINSERTSTRUCTW ToTreeViewItem(COMBOBOXEXITEMW item)
    {
        return TVINSERTSTRUCTW{};
    }

    TVINSERTSTRUCTW ToTreeViewItem(TVINSERTSTRUCTW item)
    {
        return TVINSERTSTRUCTW{};
    }
}

#endif