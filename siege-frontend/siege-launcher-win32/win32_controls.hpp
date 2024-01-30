#ifndef WIN32_CONTROLS_HPP
#define WIN32_CONTROLS_HPP

#include <deque>
#include <vector>
#include <span>
#include <utility>
#include <optional>
#include <string>
#include <memory_resource>
#include "win32_messages.hpp"
#include "CommCtrl.h"

namespace win32
{
    using hinstance_t = HINSTANCE;
    using lresult_t = LRESULT;


    template<typename TWindow>
    std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
    {
        if constexpr (requires(TWindow t) { t.on_pre_create(pre_create_message{wParam, lParam}); })
        {
            if (message == pre_create_message::id)
            {
                return self->on_pre_create(pre_create_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_create(create_message{wParam, lParam}); })
        {
            if (message == create_message::id)
            {
                return self->on_create(create_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_init_dialog(init_dialog_message{wParam, lParam}); })
        {
            if (message == init_dialog_message::id)
            {
                return self->on_init_dialog(init_dialog_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_destroy(destroy_message{wParam, lParam}); })
        {
            if (message == destroy_message::id)
            {
                return self->on_destroy(destroy_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_command(command_message{wParam, lParam}); })
        {
            if (message == command_message::id)
            {
                return self->on_command(command_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_keyboard_key_up(keyboard_key_up_message{wParam, lParam}); })
        {
            if (message == keyboard_key_up_message::id)
            {
                return self->on_keyboard_key_up(keyboard_key_up_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_keyboard_key_down(keyboard_key_down_message{wParam, lParam}); })
        {
            if (message == keyboard_key_down_message::id)
            {
                return self->on_keyboard_key_down(keyboard_key_down_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_keyboard_char(keyboard_char_message{wParam, lParam}); })
        {
            if (message == keyboard_key_down_message::id)
            {
                return self->on_keyboard_char(keyboard_char_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_message(win32::message{message, wParam, lParam}); })
        {
            return self->on_message(win32::message{message, wParam, lParam});
        }

        return std::nullopt;
    }


    template <typename TWindow>
    auto RegisterClassExW(WNDCLASSEXW descriptor)
    {
        struct handler
        {
            static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
            {
                TWindow* self = nullptr;

                if (message == pre_create_message::id)
                {
                    auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

                    auto* allocator = std::pmr::get_default_resource();
                    auto* data = allocator->allocate(sizeof(TWindow), alignof(TWindow));
                    self = new (data) TWindow(hWnd, *pCreate);
                    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
                }
                else
                {
                    self = std::bit_cast<TWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
                }

                std::optional<lresult_t> result = std::nullopt;

                if (self)
                {
                    result = dispatch_message(self, message, wParam, lParam);

                    if (message == WM_NCDESTROY)
                    {
                      SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
                      self->~TWindow();
                      auto* allocator = std::pmr::get_default_resource();
                      allocator->deallocate(self, sizeof(TWindow), alignof(TWindow));
                      return 0;
                    }
                }
                    
                return result.or_else([&]{ return std::make_optional(DefWindowProc(hWnd, message, wParam, lParam)); }).value();
            }
        };

        descriptor.cbSize = sizeof(WNDCLASSEXW);
        descriptor.lpfnWndProc = handler::WindowHandler;
            
        return ::RegisterClassExW(&descriptor);
    }

    auto CreateWindowExW(CREATESTRUCTW params)
    {
        hinstance_t hinstance = params.hInstance;
        auto parent_hinstance = hinstance == nullptr && params.hwndParent != nullptr ? std::bit_cast<hinstance_t>(GetWindowLongPtrW(params.hwndParent, GWLP_HINSTANCE)) : nullptr;
        hinstance = hinstance ? hinstance : ::GetModuleHandleW(nullptr);
        return ::CreateWindowExW(
                params.dwExStyle,
                params.lpszClass,
                params.lpszName,
                params.style,
                params.x,
                params.y,
                params.cx,
                params.cy,
                params.hwndParent,
                params.hMenu,
                hinstance,
                params.lpCreateParams
            );
    }

    auto CreateWindowExW(DLGITEMTEMPLATE params, hwnd_t parent, std::wstring class_name, std::wstring caption)
    {
        return ::CreateWindowExW(
                params.dwExtendedStyle,
                class_name.c_str(),
                caption.c_str(),
                params.style,
                params.x,
                params.y,
                params.cx,
                params.cy,
                parent,
                0,
                std::bit_cast<hinstance_t>(GetWindowLongPtrW(parent, GWLP_HINSTANCE)),
                0
            );
    }

    auto DialogBoxIndirectParamW(hwnd_t parent, DLGTEMPLATE* dialog_template, std::move_only_function<INT_PTR(hwnd_t, win32::message)> on_message)
    {
        struct handler
        {
            using callback_type = std::move_only_function<INT_PTR(hwnd_t, win32::message)>;
            static INT_PTR CALLBACK DialogHandler(hwnd_t hDlg, std::uint32_t message, wparam_t wParam, lparam_t lParam)
            {
                callback_type* child = nullptr;

                if (message == init_dialog_message::id)
                {
                    child = std::bit_cast<callback_type*>(lParam);
                    SetWindowLongPtr(hDlg, DWLP_USER, std::bit_cast<LONG_PTR>(child));
                    return (INT_PTR)TRUE;
                }
                else
                {
                    child = std::bit_cast<callback_type*>(GetWindowLongPtr(hDlg, DWLP_USER));
                }

                if (child)
                {
                    auto result = child->operator()(hDlg, win32::message(message, wParam, lParam));

                    if (message == WM_NCDESTROY)
                    {
                        SetWindowLongPtr(hDlg, DWLP_USER, std::bit_cast<LONG_PTR>(nullptr));
                        return (INT_PTR)TRUE;
                    }

                    return result;
                }

                return (INT_PTR)FALSE;
            }
        };

        auto hInstance = std::bit_cast<hinstance_t>(GetWindowLongPtrW(parent, GWLP_HINSTANCE));
        return ::DialogBoxIndirectParamW(hInstance, dialog_template, parent, handler::DialogHandler, std::bit_cast<lparam_t>(&on_message));
    }

    auto SetWindowSubclass(hwnd_t handle, std::optional<lresult_t> (*on_message)(hwnd_t, win32::message))
    {
        struct handler
        {
            static lresult_t CALLBACK HandleMessage(hwnd_t hWnd, std::uint32_t uMsg, wparam_t wParam,
                lparam_t lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
            {

                using callback_type = decltype(on_message);
                auto* child = reinterpret_cast<callback_type>(dwRefData);

                if (child && child == std::bit_cast<callback_type>(uIdSubclass))
                {
                    auto result = child(hWnd, win32::message(uMsg, wParam, lParam));

                    if (uMsg == WM_NCDESTROY)
                    {
                        ::RemoveWindowSubclass(hWnd, handler::HandleMessage, std::bit_cast<UINT_PTR>(child));
                        return TRUE;
                    }

                    if (result.has_value())
                    {
                        return result.value();
                    }
                }

                return DefSubclassProc(hWnd, uMsg, wParam, lParam);
            }
        };

        return ::SetWindowSubclass(handle, handler::HandleMessage, std::bit_cast<UINT_PTR>(on_message), std::bit_cast<DWORD_PTR>(on_message));
    }

    template<typename TWindow>
    auto SetWindowSubclass(hwnd_t handle)
    {
        struct handler
        {
            static lresult_t CALLBACK HandleMessage(hwnd_t hWnd, std::uint32_t uMsg, wparam_t wParam,
                lparam_t lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
            {
                TWindow* self = std::bit_cast<TWindow*>(dwRefData);

                std::optional<lresult_t> result = std::nullopt;

                if (self && typeid(TWindow).hash_code() == std::bit_cast<std::size_t>(uIdSubclass))
                {
                    result = dispatch_message(self, uMsg, wParam, lParam);

                    if (uMsg == WM_NCDESTROY)
                    {
                      ::RemoveWindowSubclass(hWnd, handler::HandleMessage, uIdSubclass);
                      self->~TWindow();
                      auto* allocator = std::pmr::get_default_resource();
                      allocator->deallocate(self, sizeof(TWindow), alignof(TWindow));
                      return 0;
                    }
                }
                    

                return result.or_else([&]{ return std::make_optional(DefSubclassProc(hWnd, uMsg, wParam, lParam)); }).value();
            }
        };

        auto* allocator = std::pmr::get_default_resource();
        auto* data = allocator->allocate(sizeof(TWindow), alignof(TWindow));
        auto* self = new (data) TWindow(handle, CREATESTRUCTW{});

        return ::SetWindowSubclass(handle, handler::HandleMessage, std::bit_cast<UINT_PTR>(typeid(TWindow).hash_code()), std::bit_cast<DWORD_PTR>(self));
    }

    struct button
    {
        constexpr static auto class_name = WC_BUTTONW;
        constexpr static std::uint16_t dialog_id = 0x0080;
    };

    struct edit
    {
        constexpr static auto class_name = WC_EDITW;
        constexpr static std::uint16_t dialog_id = 0x0081;
    };

    struct static_text
    {
        constexpr static auto class_name = WC_STATICW;
        constexpr static std::uint16_t dialog_id = 0x0082;
    };

    struct list_box
    {
        constexpr static auto class_name = WC_LISTBOXW;
        constexpr static std::uint16_t dialog_id = 0x0083;
    };

    struct scroll_bar
    {
        constexpr static auto class_name = WC_SCROLLBARW;
        constexpr static std::uint16_t dialog_id = 0x0084;
    };

    struct combo_box
    {
        constexpr static auto class_name = WC_COMBOBOXW;
        constexpr static std::uint16_t dialog_id = 0x0085;
    };

    struct combo_box_ex
    {
        constexpr static auto class_name = WC_COMBOBOXEXW;
    };

    struct header
    {
        constexpr static auto class_name = WC_HEADERW;
    };

    struct link
    {
        constexpr static auto class_name = WC_LINK;
    };

    struct ip_address
    {
        constexpr static auto class_name = WC_IPADDRESSW;
    };

    struct list_view
    {
        constexpr static auto class_name = WC_LISTVIEWW;
    };

    struct native_font
    {
        constexpr static auto class_name = WC_NATIVEFONTCTLW;
    };

    struct page_scroller
    {

        constexpr static auto class_name = WC_PAGESCROLLERW;
    };

    struct tab_control
    {
        constexpr static auto class_name = WC_TABCONTROLW;
    };

    struct tree_view
    {
        constexpr static auto class_name = WC_TREEVIEWW;
    };

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

        std::vector<std::byte> buffer{1024, std::byte{}}; 

        DLGTEMPLATE* result()
        {
            std::pmr::monotonic_buffer_resource resource{buffer.data(), buffer.size()};

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
}

#endif