#ifndef WIN32_LIST_VIEW_HPP
#define WIN32_LIST_VIEW_HPP

#include <siege/platform/win/desktop/win32_user32.hpp>
#include <siege/platform/win/desktop/win32_header.hpp>
#include <siege/platform/win/desktop/win32_edit.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct list_view_column : ::LVCOLUMNW
    {
    
    };

    struct list_view_item : ::LVITEMW
    {
        std::wstring text;

        list_view_item(std::wstring text) : ::LVITEMW{}, text(std::move(text))
        {
            this->pszText = this->text.data();
        }

        list_view_item(const list_view_item& item) : ::LVITEMW(item), text(item.text)
        {
            this->pszText = text.data();
        }
    };

    struct list_view_group : ::LVGROUP
    {
        std::wstring text;
        
        std::vector<list_view_item> items;

        list_view_group(std::wstring text) : ::LVGROUP{}, text(std::move(text))
        {
            this->pszHeader = this->text.data();
        }

        list_view_group(std::wstring text, std::vector<list_view_item> items) : ::LVGROUP{}, text(std::move(text)), items(std::move(items))
        {
            this->pszHeader = this->text.data();
        }

        list_view_group(const list_view_group& group) : ::LVGROUP(group), text(group.text), items(group.items)
        {
            this->pszHeader = text.data();
        }
    };

    struct list_view : window
    {
        constexpr static auto class_name = WC_LISTVIEWW;

        using window::window;

        enum struct view_type : DWORD
        {
            details_view = LV_VIEW_DETAILS,
            icon_view = LV_VIEW_ICON,
            list_view = LV_VIEW_LIST,
            small_icon_view = LV_VIEW_LIST,
            tile_view = LV_VIEW_TILE
        };
        
        inline HIMAGELIST SetImageList(wparam_t wparam, HIMAGELIST image_list)
        {
            return std::bit_cast<HIMAGELIST>(SendMessageW(*this, LVM_SETIMAGELIST, wparam, std::bit_cast<lparam_t>(image_list)));
        }

        inline bool SetView(view_type type)
        {
            return SendMessageW(*this, LVM_SETVIEW, wparam_t(type), 0) == 1;
        }

        inline view_type GetView()
        {
            return view_type(SendMessageW(*this, LVM_GETVIEW, 0, 0));
        }

        inline std::optional<bool> EnableGroupView(bool should_enable)
        {
            auto result = SendMessageW(*this, LVM_ENABLEGROUPVIEW, should_enable ? TRUE : FALSE, 0);

            if (result == 0)
            {
                return std::nullopt;
            }

            return result == 1;
        }

        inline bool IsGroupViewEnabled()
        {
            return SendMessageW(*this, LVM_ISGROUPVIEWENABLED, 0, 0);
        }

        inline lparam_t GetGroupCount()
        {
            return SendMessageW(*this, LVM_GETGROUPCOUNT, 0, 0);
        }

        inline lresult_t SetExtendedListViewStyle(wparam_t wParam, lparam_t lParam)
        {
            return SendMessageW(*this, LVM_SETEXTENDEDLISTVIEWSTYLE, wParam, lParam);
        }

        inline bool SetTileViewInfo(LVTILEVIEWINFO info)
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

            return SendMessageW(*this, LVM_SETTILEVIEWINFO, 0, std::bit_cast<lparam_t>(&info));
        }

        inline wparam_t InsertGroup(wparam_t index, LVGROUP group)
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

            return SendMessageW(*this, LVM_INSERTGROUP, index, std::bit_cast<lparam_t>(&group));
        }

        [[nodiscard]] inline win32::header GetHeader()
        {
            return win32::header(hwnd_t(SendMessageW(*this, LVM_GETHEADER, 0, 0)));
        }

        [[nodiscard]] inline wparam_t GetColumnCount()
        {
            return GetHeader().GetItemCount();
        }

        [[nodiscard]] inline std::optional<LVCOLUMNW> GetColumn(wparam_t index)
        {
            LVCOLUMNW result;

            if (SendMessageW(*this, LVM_GETCOLUMNW, index, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        inline lparam_t GetColumnWidth(wparam_t index)
        {
            return SendMessageW(*this, LVM_GETCOLUMNWIDTH, index, 0);
        }

        [[maybe_unused]] inline bool SetColumnWidth(wparam_t index, lparam_t width)
        {
            return SendMessageW(*this, LVM_SETCOLUMNWIDTH, index, width);
        }

        [[maybe_unused]] inline wparam_t InsertColumn(wparam_t position, LVCOLUMNW column)
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

            position = position == -1 ? GetColumnCount() : position;
            
            auto index = SendMessageW(*this, LVM_INSERTCOLUMNW, 
                position, std::bit_cast<win32::lparam_t>(&column));

            
            if (column.cx)
            {
                SetColumnWidth(index, column.cx);
            }

            return index;
        }

        inline wparam_t InsertItem(wparam_t index, LVITEMW item)
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

            return SendMessageW(*this, LVM_INSERTITEMW, index, std::bit_cast<lparam_t>(&item));
        }

        inline void InsertGroups(std::span<list_view_group> groups)
        {
            int group_id = int(GetGroupCount() + 1);
            for (auto& group : groups)
            {
                group.iGroupId = group_id;
                
                InsertGroup(-1, group);
                auto items = std::move(group.items);

                for (auto& item : items)
                {
                    item.iGroupId = group_id;
                    InsertItem(-1, item);
                }

                group_id++;
            }
        }
    };
}

#endif