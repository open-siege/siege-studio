#ifndef WIN32_BUILDERS_HPP
#define WIN32_BUILDERS_HPP
#include "win32_controls.hpp"

namespace win32
{
    template<std::size_t TitleSize = 1, std::size_t ClassSize = 2, std::size_t DataSize = 1>
    struct alignas(DWORD) DialogItemTemplate
    {
        alignas(DWORD) DLGITEMTEMPLATE item;
        alignas(WORD) std::array<wchar_t, ClassSize> predefined_class = {{0xFFFF, static_control::dialog_id}};
        alignas(WORD) std::array<wchar_t, TitleSize> title = {};
        alignas(WORD) std::array<WORD, DataSize> data = {};
    };

    template<std::size_t TitleSize = 1, std::size_t ClassSize = 2, std::size_t DataSize = 1>
    auto MakeDialogItemTemplate(DLGITEMTEMPLATE item,
            std::array<wchar_t, TitleSize> title = {},
            std::array<wchar_t, ClassSize> predefined_class = {{0xFFFF, static_control::dialog_id}},
            std::array<WORD, DataSize> data = {})
    {
        return DialogItemTemplate<TitleSize, ClassSize, DataSize> {
            .item = std::move(item),
            .predefined_class = std::move(predefined_class),
            .title = std::move(title),
            .data = std::move(data)
        };
    }


    template<std::size_t TitleSize = 1, typename ItemTemplate = DWORD, std::size_t ClassSize = 1, std::size_t MenuSize = 1>
    struct alignas(DWORD) DialogTemplate
    {
        alignas(DWORD) DLGTEMPLATE dialog;
        alignas(WORD) std::array<WORD, MenuSize> menu = {};
        alignas(WORD) std::array<wchar_t, ClassSize> predefined_class = {};
        alignas(WORD) std::array<wchar_t, TitleSize> title = {};
        alignas(DWORD) ItemTemplate items;
    };

    template<std::size_t TitleSize = 1, typename ItemTemplate = DWORD, std::size_t ClassSize = 1, std::size_t MenuSize = 1>
    auto MakeDialogTemplate(DLGTEMPLATE dialog,
            std::array<wchar_t, TitleSize> title = {},
            ItemTemplate items = {},
            std::array<wchar_t, ClassSize> predefined_class = {},
            std::array<WORD, MenuSize> menu = {})
    {
        return DialogTemplate<TitleSize, ItemTemplate, ClassSize, MenuSize> {
            .dialog = std::move(dialog),
            .menu = std::move(menu),
            .predefined_class = std::move(predefined_class),
            .title = std::move(title),
            .items = std::move(items)
        };
    }
}

#endif