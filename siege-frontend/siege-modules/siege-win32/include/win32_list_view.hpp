#ifndef WIN32_LIST_VIEW_HPP
#define WIN32_LIST_VIEW_HPP

#include "win32_user32.hpp"
#include "win32_header.hpp"
#include "win32_edit.hpp"
#include "CommCtrl.h"

namespace win32
{
    struct list_view
    {
        constexpr static auto class_name = WC_LISTVIEWW;

        enum struct view_type : DWORD
        {
            details_view = LV_VIEW_DETAILS,
            icon_view = LV_VIEW_ICON,
            list_view = LV_VIEW_LIST,
            small_icon_view = LV_VIEW_LIST,
            tile_view = LV_VIEW_TILE
        };

        static bool SetView(hwnd_t self, view_type type)
        {
            return SendMessageW(self, LVM_SETVIEW, wparam_t(type), 0) == 1;
        }

        static view_type GetView(hwnd_t self)
        {
            return view_type(SendMessageW(self, LVM_GETVIEW, 0, 0));
        }

        static std::optional<bool> EnableGroupView(hwnd_t self, bool should_enable)
        {
            auto result = SendMessageW(self, LVM_ENABLEGROUPVIEW, should_enable ? TRUE : FALSE, 0);

            if (result == 0)
            {
                return std::nullopt;
            }

            return result == 1;
        }

        static bool IsGroupViewEnabled(hwnd_t self)
        {
            return SendMessageW(self, LVM_ISGROUPVIEWENABLED, 0, 0);
        }

        static lparam_t GetGroupCount(hwnd_t self)
        {
            return SendMessageW(self, LVM_GETGROUPCOUNT, 0, 0);
        }

        static lresult_t SetExtendedListViewStyle(hwnd_t self, wparam_t wParam, lparam_t lParam)
        {
            return SendMessageW(self, LVM_SETEXTENDEDLISTVIEWSTYLE, wParam, lParam);
        }

        static bool SetTileViewInfo(hwnd_t self, LVTILEVIEWINFO info)
        {
            info.cbSize = sizeof(info);

            bool mask_not_set = info.dwMask == 0;
            
            if (mask_not_set && (info.sizeTile.cx || info.sizeTile.cy))
            {
                info.dwMask |= LVTVIM_TILESIZE; 
            }

            if (mask_not_set && info.cLines)
            {
                info.dwMask |= LVTVIM_COLUMNS;
            }

            if (mask_not_set && (info.rcLabelMargin.left || 
                info.rcLabelMargin.right || 
                info.rcLabelMargin.top || 
                info.rcLabelMargin.bottom))
            {
                info.dwMask |= LVTVIM_LABELMARGIN;
            }

            return SendMessageW(self, LVM_SETTILEVIEWINFO, 0, std::bit_cast<lparam_t>(&info));
        }

        static wparam_t InsertGroup(hwnd_t self, wparam_t index, LVGROUP group)
        {
            group.cbSize = sizeof(group);

            bool mask_not_set = group.mask == 0;
            
            if (mask_not_set && group.pszHeader)
            {
                group.mask |= LVGF_HEADER;
            }

            if (mask_not_set && group.pszFooter)
            {
                group.mask |= LVGF_FOOTER;
            }

            if (mask_not_set && group.state)
            {
                group.mask |= LVGF_STATE;
            }

            if (mask_not_set && group.uAlign)
            {
                group.mask |= LVGF_ALIGN;
            }

            if (mask_not_set && group.iGroupId)
            {
                group.mask |= LVGF_GROUPID;
            }

            if (mask_not_set && group.pszSubtitle)
            {
                group.mask |= LVGF_SUBTITLE;
            }

            if (mask_not_set && group.pszTask)
            {
                group.mask |= LVGF_TASK;
            }

            if (mask_not_set && group.pszDescriptionTop)
            {
                group.mask |= LVGF_DESCRIPTIONTOP;
            }

            if (mask_not_set && group.pszDescriptionBottom)
            {
                group.mask |= LVGF_DESCRIPTIONBOTTOM;
            }

            if (mask_not_set && group.iTitleImage)
            {
                group.mask |= LVGF_TITLEIMAGE;
            }

            if (mask_not_set && group.iExtendedImage)
            {
                group.mask |= LVGF_EXTENDEDIMAGE;
            }

            if (mask_not_set && group.cItems)
            {
                group.mask |= LVGF_ITEMS;
            }

            if (mask_not_set && group.pszSubsetTitle)
            {
                group.mask |= LVGF_SUBSET;
            }

            if (mask_not_set && group.cchSubsetTitle)
            {
                group.mask |= LVGF_SUBSETITEMS;
            }

            return SendMessageW(self, LVM_INSERTGROUP, index, std::bit_cast<lparam_t>(&group));
        }

        [[nodiscard]] static hwnd_t GetHeader(hwnd_t self)
        {
            return hwnd_t(SendMessageW(self, LVM_GETHEADER, 0, 0));
        }

        [[nodiscard]] static wparam_t GetColumnCount(hwnd_t self)
        {
            return header::GetItemCount(GetHeader(self));
        }

        [[nodiscard]] static std::optional<LVCOLUMNW> GetColumn(hwnd_t self, wparam_t index)
        {
            LVCOLUMNW result;

            if (SendMessageW(self, LVM_GETCOLUMNW, index, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        static lparam_t GetColumnWidth(hwnd_t self, wparam_t index)
        {
            return SendMessageW(self, LVM_GETCOLUMNWIDTH, index, 0);
        }

        [[maybe_unused]] static bool SetColumnWidth(hwnd_t self, wparam_t index, lparam_t width)
        {
            return SendMessageW(self, LVM_SETCOLUMNWIDTH, index, width);
        }

        [[maybe_unused]] static wparam_t InsertColumn(hwnd_t self, wparam_t position, LVCOLUMNW column)
        {
            bool mask_not_set = column.mask == 0;

            if (mask_not_set && column.fmt)
            {
                column.mask |= LVCF_FMT;
            }

            if (mask_not_set && column.pszText)
            {
                column.mask |= LVCF_TEXT;
            }

            if (mask_not_set && column.cx)
            {
                if (!(column.cx == LVSCW_AUTOSIZE || column.cx == LVSCW_AUTOSIZE_USEHEADER))
                {
                    column.mask |= LVCF_WIDTH;
                }
            }

            if (mask_not_set && column.cxMin)
            {
                column.mask |= LVCF_MINWIDTH;
            }

            if (mask_not_set && column.cxDefault)
            {
                column.mask |= LVCF_DEFAULTWIDTH;
            }

            if (mask_not_set && column.cxIdeal)
            {
                column.mask |= LVCF_IDEALWIDTH;
            }

            if (mask_not_set && column.iOrder)
            {
                column.mask |= LVCF_ORDER;
            }

            position = position == -1 ? GetColumnCount(self) : position;
            
            auto index = SendMessageW(self, LVM_INSERTCOLUMNW, 
                position, std::bit_cast<win32::lparam_t>(&column));

            
            if (column.cx)
            {
                SetColumnWidth(self, index, column.cx);
            }

            return index;
        }

        static wparam_t InsertItem(hwnd_t self, wparam_t index, LVITEMW item)
        {
            bool mask_not_set = item.mask == 0;
            
            if (mask_not_set && item.piColFmt)
            {
                item.mask |= LVIF_COLFMT;
            }

            if (mask_not_set && item.cColumns)
            {
                item.mask |= LVIF_COLUMNS;
            }

            if (mask_not_set && item.iGroupId)
            {
                item.mask |= LVIF_GROUPID;
            }

            if (mask_not_set && item.iImage)
            {
                item.mask |= LVIF_IMAGE;
            }

            if (mask_not_set && item.iIndent)
            {
                item.mask |= LVIF_INDENT;
            }

            if (mask_not_set && item.pszText == LPSTR_TEXTCALLBACKW)
            {
                item.mask |= LVIF_NORECOMPUTE;
            }

            if (mask_not_set && item.lParam)
            {
                item.mask |= LVIF_PARAM;
            }

            if (mask_not_set && item.state)
            {
                item.mask |= LVIF_STATE;
            }

            if (mask_not_set && item.pszText)
            {
                item.mask |= LVIF_TEXT;
            }

            return SendMessageW(self, LVM_INSERTITEMW, index, std::bit_cast<lparam_t>(&item));
        }

        static std::span<LVITEMW> GetChildItems(hwnd_t self, std::span<LVITEMW> items)
        {
            return items;
        }
    };
}

#endif