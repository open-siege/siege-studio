#ifndef WIN32_COMBO_BOX_HPP
#define WIN32_COMBO_BOX_HPP

#include <siege/platform/win/desktop/win32_window.hpp>
#include <siege/platform/win/desktop/win32_edit.hpp>
#include <siege/platform/win/desktop/win32_list_box.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct combo_box : window
    {
        using window::window;
        constexpr static auto class_name = WC_COMBOBOXW;
        constexpr static std::uint16_t dialog_id = 0x0085;

        [[maybe_unused]] inline wparam_t AddString(wparam_t index, std::string_view text)
        {
            return SendMessageW(self, CB_ADDSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }

        [[maybe_unused]] inline wparam_t InsertString(wparam_t index, std::string_view text)
        {
            return SendMessageW(self, CB_INSERTSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }

        [[nodiscard]] inline std::expected<COMBOBOXINFO, DWORD> GetComboBoxInfo()
        {
            COMBOBOXINFO result;

            if (SendMessageW(self, CB_GETCOMBOBOXINFO, 0, std::bit_cast<LPARAM>(&result)))
            {
                return result;
            }

            return std::unexpected(GetLastError());
        }
    };

    struct combo_box_ex : window
    {
        using window::window;
        constexpr static auto class_name = WC_COMBOBOXEXW;

        [[maybe_unused]] inline wparam_t InsertItem(COMBOBOXEXITEMW info)
        {
            return SendMessageW(self, CBEM_INSERTITEMW, 0, std::bit_cast<win32::lparam_t>(&info));
        }

        inline std::span<COMBOBOXEXITEMW> GetChildItems(std::span<COMBOBOXEXITEMW> items)
        {
            return items;
        }
    };
}

#endif