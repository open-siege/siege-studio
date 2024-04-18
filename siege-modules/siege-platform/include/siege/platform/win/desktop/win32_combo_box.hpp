#ifndef WIN32_COMBO_BOX_HPP
#define WIN32_COMBO_BOX_HPP

#include <siege/platform/win/gaming/win32_user32.hpp>
#include <siege/platform/win/desktop/win32_edit.hpp>
#include <siege/platform/win/desktop/win32_list_box.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct combo_box
    {
        constexpr static auto class_name = WC_COMBOBOXW;
        constexpr static std::uint16_t dialog_id = 0x0085;

        [[maybe_unused]] static wparam_t AddString(hwnd_t self, wparam_t index, std::string_view text)
        {
            return SendMessageW(self, CB_ADDSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }

        [[maybe_unused]] static wparam_t InsertString(hwnd_t self, wparam_t index, std::string_view text)
        {
            return SendMessageW(self, CB_INSERTSTRING, index, std::bit_cast<LPARAM>(text.data()));
        }

        [[nodiscard]] static std::expected<COMBOBOXINFO, DWORD> GetComboBoxInfo(hwnd_t self)
        {
            COMBOBOXINFO result;

            if (SendMessageW(self, CB_GETCOMBOBOXINFO, 0, std::bit_cast<LPARAM>(&result)))
            {
                return result;
            }

            return std::unexpected(GetLastError());
        }

        static std::span<std::string_view> GetChildStrings(hwnd_t self, std::span<std::string_view> strings)
        {
            return strings;
        }
    };

    struct combo_box_ex
    {
        constexpr static auto class_name = WC_COMBOBOXEXW;

        [[maybe_unused]] wparam_t InsertItem(hwnd_t self, COMBOBOXEXITEMW info)
        {
            return SendMessageW(self, CBEM_INSERTITEMW, 0, std::bit_cast<win32::lparam_t>(&info));
        }

        static std::span<COMBOBOXEXITEMW> GetChildItems(hwnd_t self, std::span<COMBOBOXEXITEMW> items)
        {
            return items;
        }
    };
}

#endif