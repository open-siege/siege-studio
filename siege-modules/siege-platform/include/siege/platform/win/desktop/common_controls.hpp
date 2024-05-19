#ifndef WIN32_COMPOSITE_CONTROLS_HPP
#define WIN32_COMPOSITE_CONTROLS_HPP

#include <filesystem>
#include <siege/platform/win/desktop/user_controls.hpp>

namespace win32
{
    struct control : window
    {
        using window::window;

        inline void SetVersion()
        {

        }

        inline void GetVersion()
        {

        }

        inline void SetWindowTheme()
        {

        }

        inline void DpiScale()
        {

        }

        inline void GetUnicodeFormat()
        {

        }

        inline void SetUnicodeFormat()
        {

        }

    };

    struct link : window
    {
        using window::window;
        constexpr static auto class_name = WC_LINK;
    };

    struct ip_address : window
    {
        constexpr static auto class_name = WC_IPADDRESSW;
    };

    struct native_font : window
    {
        using window::window;
        constexpr static auto class_name = WC_NATIVEFONTCTLW;
    };

    struct track_bar : window
    {
        using window::window;
        constexpr static auto class_name = TRACKBAR_CLASSW;
    };

    struct up_down : window
    {
        using window::window;
        constexpr static auto class_name = UPDOWN_CLASSW;

        inline void SetBuddy()
        {

        }

        inline void SetRange()
        {
            
        }
    };

    struct header : control
    {
        using control::control;
        constexpr static auto class_name = WC_HEADERW;

        [[nodiscard]] inline wparam_t GetItemCount()
        {
            return SendMessageW(*this, HDM_GETITEMCOUNT, 0, 0);
        }

        [[maybe_unused]] inline wparam_t InsertItem(wparam_t index, HDITEMW info)
        {
            return SendMessageW(*this, HDM_INSERTITEMW, index, std::bit_cast<win32::lparam_t>(&info));
        }
    };

    struct list_view_column : ::LVCOLUMNW
    {
    
    };

    struct list_view_item : ::LVITEMW
    {
        std::wstring text;

        std::vector<std::wstring> sub_items;

        list_view_item(std::wstring text) : ::LVITEMW{}, text(std::move(text)), sub_items{}
        {
            this->pszText = this->text.data();
        }

        list_view_item(const list_view_item& item) : ::LVITEMW(item), text(item.text), sub_items(item.sub_items)
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

    struct list_view : control
    {
        constexpr static auto class_name = WC_LISTVIEWW;

        using control::control;

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

        inline std::optional<LVITEMW> GetItem(LVITEMW item)
        {
            if (SendMessageW(*this, LVM_GETITEM, 0, std::bit_cast<lparam_t>(&item)))
            {
                return item;
            }

            return std::nullopt;
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

        inline void InsertRow(list_view_item row)
        {
            auto index = this->InsertItem(-1, row);

            LVITEMW sub_item{};
           
            for (auto i = 1u; i <= row.sub_items.size(); ++i)
            {
                sub_item.iSubItem = i;
                sub_item.pszText = row.sub_items[i - 1].data();
                sub_item.mask = LVIF_TEXT;
                ::SendMessageW(*this, LVM_SETITEMTEXT, index, (LPARAM)&sub_item);
            }
        }
    };

    struct rebar : window
    {
        using window::window;
        constexpr static auto class_Name = REBARCLASSNAMEW;

        [[nodiscard]] inline std::optional<RECT> GetRect(wparam_t index)
        {
            RECT result;

            if (SendMessageW(*this, RB_GETRECT, index, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        [[nodiscard]] inline std::uint32_t GetBarHeight()
        {
            return std::uint32_t(SendMessageW(*this, RB_GETBARHEIGHT, 0, 0));
        }

        [[nodiscard]] inline lparam_t GetBandCount()
        {
            return SendMessageW(*this, RB_GETBANDCOUNT, 0, 0);
        }

        [[maybe_unused]] inline void SetBandWidth(wparam_t index, lparam_t new_width)
        {
            SendMessageW(*this, RB_SETBANDWIDTH, index, new_width);
        }

        [[maybe_unused]] inline void MaximizeBand(wparam_t index, lparam_t ideal_width = 0)
        {
            SendMessageW(*this, RB_MAXIMIZEBAND, index, ideal_width);
        }

        [[maybe_unused]] inline std::optional<REBARBANDINFOW> GetBandChildSize(wparam_t index)
        {
            REBARBANDINFOW band {.cbSize = sizeof(REBARBANDINFOW), .fMask = RBBIM_CHILDSIZE};
            
            if (SendMessageW(*this, RB_GETBANDINFOW, index, std::bit_cast<win32::lparam_t>(&band)))
            {
                return band;
            }

            return std::nullopt;
        }

        [[maybe_unused]] inline bool SetBandInfo(wparam_t index, REBARBANDINFOW band)
        {
            band.cbSize = sizeof(band);
            return SendMessageW(*this, RB_SETBANDINFOW, 
                index, std::bit_cast<win32::lparam_t>(&band));
        }

        [[maybe_unused]] inline bool InsertBand(wparam_t position, REBARBANDINFOW band)
        {
            band.cbSize = sizeof(band);

            bool mask_not_set = band.fMask == 0;

            if (mask_not_set && band.hwndChild)
            {
                band.fMask |= RBBIM_CHILD;
            }

            if (mask_not_set && band.lpText)
            {
                band.fMask |= RBBIM_TEXT;
            }

            if (mask_not_set && (
                band.cxMinChild ||
                band.cyMinChild ||
                band.cyChild ||
                band.cyMaxChild
                ))
            {
                band.fMask |= RBBIM_CHILDSIZE;
            }

            if (mask_not_set && band.cx)
            {
                band.fMask |= RBBIM_SIZE;
            }

            if (mask_not_set && band.cxIdeal)
            {
                band.fMask |= RBBIM_IDEALSIZE;
            }

            if (mask_not_set && band.cxHeader)
            {
                band.fMask |= RBBIM_HEADERSIZE;
            }

            if (mask_not_set && band.fStyle)
            {
                band.fMask |= RBBIM_STYLE;
            }
            
            return SendMessageW(*this, RB_INSERTBANDW, 
                position, std::bit_cast<win32::lparam_t>(&band));
        }
    };

    struct tab_control : window
    {
        using window::window;
        constexpr static auto class_name = WC_TABCONTROLW;

        [[maybe_unused]] inline wparam_t InsertItem(wparam_t index, TCITEMW info)
        {
            return SendMessageW(*this, TCM_INSERTITEMW, index, std::bit_cast<lparam_t>(&info));
        }

        [[nodiscard]] inline wparam_t GetItemCount()
        {
            return SendMessageW(*this, TCM_GETITEMCOUNT, 0, 0);
        }

        [[maybe_unused]] inline wparam_t SetCurrentSelection(wparam_t index)
        {
            return SendMessageW(*this, TCM_SETCURSEL, index, 0);
        }

        [[nodiscard]] inline std::optional<TCITEMW> GetItem(wparam_t index, std::uint32_t mask = TCIF_PARAM | TCIF_STATE)
        {
            TCITEMW result { .mask = mask };

            if (SendMessageW(*this, TCM_GETITEM, index, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        inline RECT AdjustRect(bool dispay_to_window, RECT rect)
        {
            SendMessageW(*this, TCM_ADJUSTRECT, dispay_to_window ? TRUE : FALSE, std::bit_cast<lparam_t>(&rect));
            return rect;
        }
    };
    
    struct tool_bar : control
    {
        using control::control;
        constexpr static auto class_name = TOOLBARCLASSNAMEW;

        enum extended_style : DWORD
        {
            mixed_buttons = TBSTYLE_EX_MIXEDBUTTONS,
            draw_drop_down_arrows = TBSTYLE_EX_DRAWDDARROWS
        };

        inline void AutoSize()
        {
            SendMessageW(*this, TB_AUTOSIZE, 0, 0);
        }

        [[maybe_unused]] inline bool SetButtonWidth(std::array<int, 2> range)
        {
            auto [min, max] = range;

            return SendMessageW(*this, TB_SETBUTTONWIDTH, 0, MAKELPARAM(min, max));
        }

        [[maybe_unused]] inline bool SetButtonSize(SIZE size)
        {
            return SendMessageW(*this, TB_SETBUTTONSIZE , 0, MAKELPARAM(size.cx, size.cy));
        }

        [[nodiscard]] inline SIZE GetButtonSize()
        {
            auto result = SendMessageW(*this, TB_GETBUTTONSIZE, 0, 0);

            return SIZE {.cx = LOWORD(result), .cy = HIWORD(result)};
        }

        [[nodiscard]] inline std::optional<RECT> GetRect(wparam_t id)
        {
            RECT result;
            if (SendMessageW(*this, TB_GETRECT, id, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        [[maybe_unused]] inline extended_style SetExtendedStyle(lparam_t style)
        {
            return extended_style(SendMessageW(*this, 
                TB_SETEXTENDEDSTYLE, 0, style));
        
        }

        [[maybe_unused]] inline bool InsertButton(wparam_t index, TBBUTTON button)
        {
            SendMessageW(*this, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
            return SendMessageW(*this, TB_INSERTBUTTONW, index, 
                std::bit_cast<win32::lparam_t>(&button));
        }

        [[maybe_unused]] inline bool AddButtons(std::span<TBBUTTON> buttons)
        {
            SendMessageW(*this, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
            return SendMessageW(*this, TB_ADDBUTTONSW, wparam_t(buttons.size()), 
                std::bit_cast<win32::lparam_t>(buttons.data()));
        }
    };

    struct tree_view_item : ::TVINSERTSTRUCTW
    {
        std::wstring text;

        std::vector<tree_view_item> children;

        tree_view_item(const std::filesystem::path& path) : ::TVINSERTSTRUCTW{}, text(path.wstring())
        {
            this->item.pszText = this->text.data();
            this->item.lParam = (LPARAM)path.c_str();
        }

        tree_view_item(const std::filesystem::path& path, std::wstring text) : ::TVINSERTSTRUCTW{}, text(std::move(text))
        {
            this->item.pszText = this->text.data();
            this->item.lParam = (LPARAM)path.c_str();
        }

        tree_view_item(const std::filesystem::path& path, std::wstring text, std::vector<tree_view_item> children) : ::TVINSERTSTRUCTW{}, text(std::move(text)), children(std::move(children))
        {
            this->item.pszText = this->text.data();
            this->item.lParam = (LPARAM)path.c_str();
        }

        tree_view_item(const tree_view_item& item) : ::TVINSERTSTRUCTW(item), text(item.text), children(item.children)
        {
            this->item.pszText = text.data();
        }    
    };


    struct tree_view : control
    {
        using control::control;
        constexpr static auto class_name = WC_TREEVIEWW;

        [[maube_unused]] inline bool Expand(HTREEITEM item, wparam_t action = TVE_EXPAND)
        {
            return SendMessageW(*this, TVM_EXPAND, action, std::bit_cast<lparam_t>(item));
        }

        [[maube_unused]] inline bool Collapse(HTREEITEM item, wparam_t action = TVE_COLLAPSE)
        {
            return SendMessageW(*this, TVM_EXPAND, action, std::bit_cast<lparam_t>(item));
        }

        [[maybe_unused]] inline bool EnsureVisible(HTREEITEM item)
        {
            return SendMessageW(*this, TVM_ENSUREVISIBLE, 0, std::bit_cast<lparam_t>(item));
        }

        [[maybe_unused]] inline bool DeleteItem(HTREEITEM item)
        {
            return SendMessageW(*this, TVM_DELETEITEM, 0, std::bit_cast<lparam_t>(item));
        }

        [[maybe_unused]] inline bool SetImageList(wparam_t type, HIMAGELIST list)
        {
            return SendMessageW(*this, TVM_SETIMAGELIST, type, std::bit_cast<lparam_t>(list));
        }

        void SetItemMask(TVITEMW& info)
        {
             bool mask_not_set = info.mask == 0;

            if (mask_not_set && info.pszText)
            {
                info.mask |= TVIF_TEXT;
            }

            if (mask_not_set && info.hItem)
            {
                info.mask |= TVIF_HANDLE;
            }

            if (mask_not_set && info.iImage)
            {
                info.mask |= TVIF_IMAGE;
            }

            if (mask_not_set && info.lParam)
            {
                info.mask |= TVIF_PARAM;
            }

            if (mask_not_set && info.iSelectedImage)
            {
                info.mask |= TVIF_SELECTEDIMAGE;
            }

            if (mask_not_set && (info.state || info.stateMask))
            {
                info.mask |= TVIF_STATE;
            }

            if (mask_not_set && info.cChildren)
            {
                info.mask |= TVIF_CHILDREN;
            }
        }

        [[maybe_unused]] inline std::optional<HTREEITEM> InsertItem(TVINSERTSTRUCTW info)
        {
            SetItemMask(info.item);

            if (auto result = HTREEITEM(SendMessageW(*this, TVM_INSERTITEMW, 0, std::bit_cast<lparam_t>(&info))); result)
            {
                return result;
            }

            return std::nullopt;
        }

        [[maybe_unused]] inline bool SetItem(TVITEMW info)
        {
            SetItemMask(info);

            return SendMessageW(*this, TVM_SETITEMW, 0, std::bit_cast<lparam_t>(&info));
        }

        [[maybe_unused]] inline void InsertRoots(std::span<tree_view_item> items)
        {
            for (auto& item : items)
            {
                item.hParent = TVI_ROOT; 
                item.item.cChildren = 1;

                auto result = InsertItem(item);

                auto children = std::move(item.children);

                for (auto& child : children)
                {
                    child.hParent = *result;
                    child.hInsertAfter = TVI_LAST;
                    InsertItem(child);
                }

                Expand(*result);
            }
        }
    };

    struct page_scroller : window
    {
        using window::window;
        constexpr static auto class_name = WC_PAGESCROLLERW;
    };

    

    struct combo_box_ex : control
    {
        using control::control;
        constexpr static auto class_name = WC_COMBOBOXEXW;

        [[maybe_unused]] inline wparam_t InsertItem(COMBOBOXEXITEMW info)
        {
            return SendMessageW(*this, CBEM_INSERTITEMW, 0, std::bit_cast<win32::lparam_t>(&info));
        }

        inline std::span<COMBOBOXEXITEMW> GetChildItems(std::span<COMBOBOXEXITEMW> items)
        {
            return items;
        }
    };
}


#endif