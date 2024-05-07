#ifndef WIN32_TREE_VIEW_HPP
#define WIN32_TREE_VIEW_HPP

#include <siege/platform/win/desktop/win32_window.hpp>
#include <siege/platform/win/desktop/win32_edit.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct tree_view : window
    {
        using window::window;
        constexpr static auto class_name = WC_TREEVIEWW;

        [[maybe_unused]] inline std::optional<HTREEITEM> InsertItem(TVINSERTSTRUCTW info)
        {
            bool mask_not_set = info.itemex.mask == 0;

            if (mask_not_set && info.itemex.pszText)
            {
                info.itemex.mask |= TVIF_TEXT;
            }

            if (mask_not_set && info.itemex.hItem)
            {
                info.itemex.mask |= TVIF_HANDLE;
            }

            if (mask_not_set && info.itemex.iImage)
            {
                info.itemex.mask |= TVIF_IMAGE;
            }

            if (mask_not_set && info.itemex.lParam)
            {
                info.itemex.mask |= TVIF_PARAM;
            }

            if (mask_not_set && info.itemex.iSelectedImage)
            {
                info.itemex.mask |= TVIF_SELECTEDIMAGE;
            }

            if (mask_not_set && (info.itemex.state || info.itemex.stateMask))
            {
                info.itemex.mask |= TVIF_STATE;
            }

            if (mask_not_set && info.itemex.cChildren)
            {
                info.itemex.mask |= TVIF_CHILDREN;
            }

            if (auto result = HTREEITEM(SendMessageW(*this, TVM_INSERTITEMW, 0, std::bit_cast<lparam_t>(&info))); result)
            {
                return result;
            }

            return std::nullopt;
        }
    };
}

#endif