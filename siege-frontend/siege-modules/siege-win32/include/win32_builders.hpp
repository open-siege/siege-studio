#ifndef WIN32_BUILDERS_HPP
#define WIN32_BUILDERS_HPP
#include "win32_controls.hpp"

namespace win32
{
    struct dialog_builder
    {
        struct root_template
        {
            DLGTEMPLATE root;
            std::variant<std::monostate, std::uint16_t, std::wstring> menu_resource;
            std::variant<std::monostate, std::uint16_t, std::wstring> root_class;
            std::wstring title;
        } root;

        template<typename TClass = std::monostate, typename TMenu = std::monostate>
        dialog_builder&& create_dialog(DLGTEMPLATE root, std::wstring_view title = L"", std::optional<TClass> root_class = std::nullopt, std::optional<TMenu> menu = std::nullopt)
        {
            this->root.root = std::move(root);

            if (!title.empty())
            {
                this->root.title = std::move(title);
            }

            if (root_class.has_value())
            {
                this->root.root_class = std::move(root_class.value());
            }

            if (menu.has_value())
            {
                this->root.menu_resource = std::move(menu.value());
            }

            return std::move(*this);
        }

        struct child_template
        {
            DLGITEMTEMPLATE child;
            std::variant<std::uint16_t, std::wstring> child_class;
            std::variant<std::uint16_t, std::wstring> caption;
            std::vector<std::byte> data;
        };

        std::deque<child_template> children;

        std::uint16_t convert(std::uint16_t value)
        {
            return value;
        }

        std::wstring convert(std::u16string_view value)
        {
            return std::wstring{value.begin(), value.end()};
        }

        std::wstring convert(std::wstring_view value)
        {
            return std::wstring{value};
        }

        template<typename TClass, typename TCaption>
        dialog_builder&& add_child(DLGITEMTEMPLATE child, TClass&& child_class, TCaption&& caption, std::span<std::byte> data = std::span<std::byte>{})
        {
            auto& new_child = children.emplace_back(std::move(child), convert(std::forward<TClass>(child_class)), convert(std::forward<TCaption>(caption)));
            new_child.data.assign(data.begin(), data.end());

            root.root.cdit = std::uint16_t(children.size());
            return std::move(*this);
        }

        DLGTEMPLATE* result(std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
        {
            std::pmr::monotonic_buffer_resource resource{1024, upstream};
            void* root_storage = resource.allocate(sizeof(root.root), alignof(std::uint32_t));
            DLGTEMPLATE* result = new (root_storage)DLGTEMPLATE{root.root};

            auto specify_no_data = [&]() {
              void* temp = resource.allocate(sizeof(std::uint16_t), alignof(std::uint16_t));
                new (temp)std::uint16_t(0x0000);
            };

            auto specify_id = [&](std::uint16_t resource_id) {
              void* temp = resource.allocate(sizeof(std::array<std::uint16_t, 2>), alignof(std::uint16_t));
                new (temp) std::array<std::uint16_t, 2>{{0xFFFF, resource_id}};
            };

            auto specify_string = [&](std::wstring& temp_str) {
              const auto str_size = (temp_str.size() + 1) * sizeof(wchar_t);
              void* temp = resource.allocate(str_size, alignof(std::uint16_t));
              std::memcpy(temp, temp_str.c_str(), str_size);
            };

            if (std::holds_alternative<std::monostate>(this->root.menu_resource))
            {
                specify_no_data();
            }
            else if (std::holds_alternative<std::uint16_t>(this->root.menu_resource))
            {
                specify_id(std::get<std::uint16_t>(this->root.menu_resource));
            }
            else if (std::holds_alternative<std::wstring>(this->root.menu_resource))
            {
                specify_string(std::get<std::wstring>(this->root.menu_resource));
            }

            if (std::holds_alternative<std::monostate>(this->root.root_class))
            {
                specify_no_data();
            }
            else if (std::holds_alternative<std::uint16_t>(this->root.root_class))
            {
                specify_id(std::get<std::uint16_t>(this->root.root_class));
            }
            else if (std::holds_alternative<std::wstring>(this->root.root_class))
            {
                specify_string(std::get<std::wstring>(this->root.root_class));
            }

            if (!root.title.empty())
            {
                specify_string(root.title);
            }
            else
            {
                specify_no_data();
            }

            for (auto& child : children)
            {
                void* temp = resource.allocate(sizeof(child.child), alignof(std::uint32_t));
                new (temp)DLGITEMTEMPLATE{child.child};

                if (std::holds_alternative<std::uint16_t>(child.child_class))
                {
                    specify_id(std::get<std::uint16_t>(child.child_class));
                }
                else if (std::holds_alternative<std::wstring>(child.child_class))
                {
                    specify_string(std::get<std::wstring>(child.child_class));
                }

                if (std::holds_alternative<std::uint16_t>(child.caption))
                {
                    specify_id(std::get<std::uint16_t>(child.caption));
                }
                else if (std::holds_alternative<std::wstring>(child.caption))
                {
                    specify_string(std::get<std::wstring>(child.caption));
                }

                if (!child.data.empty())
                {
                    std::uint16_t data_size = sizeof(std::uint16_t) + std::uint16_t(child.data.size());
                    void* temp = resource.allocate(data_size, alignof(std::uint16_t));

                    std::memcpy(temp, &data_size, sizeof(data_size));
                    std::memcpy(reinterpret_cast<std::uint16_t*>(temp) + sizeof(data_size), child.data.data(), child.data.size() + 1);
                }
                else
                {
                    specify_no_data();
                }
            }

            return result;
        }
    };

    struct menu_builder
    {
        struct root_template
        {
            MENUITEMTEMPLATEHEADER root;
        } root;

        menu_builder&& create_menu(MENUITEMTEMPLATEHEADER root)
        {
            this->root.root = std::move(root);
            return std::move(*this);
        }

        struct child_template
        {
            MENUITEMTEMPLATE child;
            std::wstring caption;
        };

        std::deque<child_template> children;

        menu_builder&& add_child(MENUITEMTEMPLATE child, std::wstring caption)
        {
            auto& new_child = children.emplace_back(std::move(child), std::move(caption));
            return std::move(*this);
        }

        MENUITEMTEMPLATEHEADER* result(std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
        {
            std::pmr::monotonic_buffer_resource resource{1024, upstream};
            void* root_storage = resource.allocate(sizeof(root.root), alignof(std::uint16_t));
            MENUITEMTEMPLATEHEADER* result = new (root_storage)MENUITEMTEMPLATEHEADER{root.root};

            return result;
        }
    };

    struct property_sheet_builder
    {
        struct root_template
        {
            PROPSHEETHEADER root;
        } root;

        property_sheet_builder&& create_property_sheet(PROPSHEETHEADER root)
        {
            this->root.root = std::move(root);
            return std::move(*this);
        }

        struct child_template
        {
            PROPSHEETPAGE child;
        };

        std::deque<child_template> children;

        property_sheet_builder&& add_child(PROPSHEETPAGE child)
        {
            auto& new_child = children.emplace_back(std::move(child));
            return std::move(*this);
        }

        PROPSHEETHEADER* result(std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
        {
            std::pmr::monotonic_buffer_resource resource{1024, upstream};
            void* root_storage = resource.allocate(sizeof(root.root), alignof(std::uint16_t));
            PROPSHEETHEADER* result = new (root_storage)PROPSHEETHEADER{root.root};

            return result;
        }
    };

    struct task_dialog_builder
    {
        struct root_template
        {
            TASKDIALOGCONFIG root;
        } root;

        task_dialog_builder&& create_task_dialog(TASKDIALOGCONFIG root)
        {
            this->root.root = std::move(root);
            return std::move(*this);
        }

        struct child_template
        {
            TASKDIALOG_BUTTON child;
        };

        std::deque<child_template> children;

        task_dialog_builder&& add_child(TASKDIALOG_BUTTON child)
        {
            auto& new_child = children.emplace_back(std::move(child));
            return std::move(*this);
        }

        TASKDIALOGCONFIG* result(std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
        {
            std::pmr::monotonic_buffer_resource resource{1024, upstream};
            void* root_storage = resource.allocate(sizeof(root.root), alignof(std::uint16_t));
            TASKDIALOGCONFIG* result = new (root_storage)TASKDIALOGCONFIG{root.root};

            return result;
        }
    };

    struct toolbar_builder
    {
        struct root_template
        {
            CREATESTRUCTW root;
        } root;

        toolbar_builder&& create_toolbar(CREATESTRUCTW root)
        {
            this->root.root = std::move(root);
            return std::move(*this);
        }

        struct child_template
        {
            TBBUTTON child;
        };

        std::deque<child_template> children;

        toolbar_builder&& add_child(TBBUTTON child)
        {
            auto& new_child = children.emplace_back(std::move(child));
            return std::move(*this);
        }

        hwnd_t result(std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
        {
            auto toolbar = win32::CreateWindowExW(std::move(root.root));

            auto results = std::pmr::vector<TBBUTTON>(children.size(), upstream);

            SendMessageW(toolbar, TB_ADDBUTTONS, results.size(), std::bit_cast<LPARAM>(results.data()));

            return toolbar;
        }
    };

    struct list_view_builder
    {
        struct root_template
        {
            CREATESTRUCTW root;
        } root;

        list_view_builder&& create_list_view(CREATESTRUCTW root)
        {
            this->root.root = std::move(root);
            return std::move(*this);
        }

        std::deque<LVITEMW> items;
        std::deque<LVGROUP> groups;
        std::deque<LVCOLUMNW> columns;

        list_view_builder&& add_child(LVITEMW child)
        {
            auto& new_child = items.emplace_back(std::move(child));
            return std::move(*this);
        }

        list_view_builder&& add_child(LVGROUP child)
        {
            auto& new_child = groups.emplace_back(std::move(child));
            return std::move(*this);
        }

        list_view_builder&& add_child(LVCOLUMNW child)
        {
            auto& new_child = columns.emplace_back(std::move(child));
            return std::move(*this);
        }

        hwnd_t result(std::pmr::memory_resource* upstream = nullptr)
        {
            auto toolbar = win32::CreateWindowExW(std::move(root.root));

            for (auto& item : items)
            {
                SendMessageW(toolbar, LVM_INSERTITEM, 0, std::bit_cast<LPARAM>(&item));
            }

            for (auto& column : columns)
            {
                SendMessageW(toolbar, LVM_INSERTCOLUMN, 0, std::bit_cast<LPARAM>(&column));
            }

            for (auto& group : groups)
            {
                SendMessageW(toolbar, LVM_INSERTGROUP, -1, std::bit_cast<LPARAM>(&group));
            }

            return toolbar;
        }
    };
}

#endif