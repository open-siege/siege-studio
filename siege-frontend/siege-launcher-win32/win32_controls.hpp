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

    struct window
    {
        using callback_type = std::move_only_function<std::optional<lresult_t>(window&, window_message)>;
        
        hwnd_t handle;
        callback_type HandleMessage;
        std::function<BOOL()> unregister_class;
        

        window(WNDCLASSEXW descriptor, CREATESTRUCTW params, callback_type handler) 
            : handle(0),
            HandleMessage(std::move(handler))
            {
                descriptor.cbSize = sizeof(WNDCLASSEXW);
                descriptor.lpfnWndProc = WindowHandler;
                
                if (GetClassInfoExW(descriptor.hInstance, descriptor.lpszClassName, &descriptor) == FALSE)
                {
                    auto result = RegisterClassExW(&descriptor);
                    unregister_class = std::bind(UnregisterClassW, descriptor.lpszClassName, descriptor.hInstance);
                }

                handle = CreateWindowExW(params.dwExStyle,
                    descriptor.lpszClassName,
                    params.lpszName,
                    params.style,
                    params.x,
                    params.y,
                    params.cx,
                    params.cy,
                    params.hwndParent,
                    params.hMenu,
                    descriptor.hInstance,
                    std::bit_cast<LPVOID>(this));
            }

        window(WNDCLASSEXW descriptor, DLGTEMPLATE params, hwnd_t parent, std::wstring caption, callback_type handler)
            : window(std::move(descriptor), CREATESTRUCTW{
                .hwndParent = parent,
                .cy = params.cy,
                .cx = params.cx,
                .y = params.y,
                .x = params.x,
                .style = std::bit_cast<LONG>(params.style),
                .lpszName = caption.c_str(),
                .dwExStyle = params.dwExtendedStyle
            }, std::move(handler))
        {
        }

        window(WNDCLASSEXW descriptor, DLGTEMPLATE params, std::wstring caption, callback_type handler)
            : window(std::move(descriptor), std::move(params), 0, std::move(caption), std::move(handler))
        {
        }

        operator hwnd_t()
        {
            return handle;
        }

        static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
        {
            window* self = nullptr;

            if (message == pre_create_message::id)
            {
                auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);
                self = std::bit_cast<window*>(pCreate->lpCreateParams);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
                self->handle = hWnd;
            }
            else
            {
                self = std::bit_cast<window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
            }

            if (self && self->HandleMessage)
            {
                auto result = self->HandleMessage(*self, make_window_message(message, wParam, lParam));

                if (message == WM_DESTROY)
                {
                  SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
                  self->HandleMessage = nullptr;
                  self->handle = 0;

                  if (self->unregister_class)
                  {
                    self->unregister_class();
                    self->unregister_class = nullptr;

                  }
                  return 0;
                }

                if (result.has_value())
                {
                    return result.value();
                }
            }

            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    };

    struct dialog
    {
        using callback_type = std::move_only_function<INT_PTR(dialog&, window_message)>;
        hwnd_t handle;
        callback_type HandleMessage;
        
        dialog(callback_type handler): handle(0), HandleMessage(std::move(handler))
        {
        }

        operator hwnd_t()
        {
            return handle;
        }

        [[maybe_unused]] static auto show_modal(hwnd_t parent, DLGTEMPLATE* dialog_template, callback_type handler)
        {
            dialog temp{std::move(handler)};
            auto hInstance = std::bit_cast<hinstance_t>(GetWindowLongPtrW(parent, GWLP_HINSTANCE));
            return DialogBoxIndirectParamW(hInstance, dialog_template, parent, dialog::HandleAboutDialogMessage, std::bit_cast<lparam_t>(&temp));
        }

        [[maybe_unused]] static auto show_modal(hwnd_t parent, LPWSTR templateName, callback_type handler)
        {
            dialog temp{std::move(handler)};
            auto hInstance = std::bit_cast<hinstance_t>(GetWindowLongPtrW(parent, GWLP_HINSTANCE));
            return DialogBoxParamW(hInstance, templateName, parent, dialog::HandleAboutDialogMessage, std::bit_cast<lparam_t>(&temp));
        }
        

        static INT_PTR CALLBACK HandleAboutDialogMessage(hwnd_t hDlg, std::uint32_t message, wparam_t wParam, lparam_t lParam)
        {
            dialog* child = nullptr;

            if (message == init_dialog_message::id)
            {
                child = std::bit_cast<dialog*>(lParam);
                
                if (child->handle == 0)
                {
                    child->handle = hDlg;
                }

                SetWindowLongPtr(hDlg, DWLP_USER, std::bit_cast<LONG_PTR>(child));
                return (INT_PTR)TRUE;
            }
            else
            {
                child = std::bit_cast<dialog*>(GetWindowLongPtr(hDlg, DWLP_USER));
            }

            if (child && child->HandleMessage)
            {
                auto result = child->HandleMessage(*child, make_window_message(message, wParam, lParam));

                if (message == WM_DESTROY)
                {
                    SetWindowLongPtr(child->handle, DWLP_USER, std::bit_cast<LONG_PTR>(nullptr));
                    child->HandleMessage = nullptr;
                    child->handle = 0;
                    return (INT_PTR)TRUE;
                }

                return result;
            }

            return (INT_PTR)FALSE;
        }
    };

    template<typename Control>
    struct control
    {
        hwnd_t handle;

        using callback_type = std::move_only_function<std::optional<lresult_t>(Control&, UINT_PTR uIdSubclass, window_message)>;
        callback_type HandleMessage;

        operator hwnd_t()
        {
            return handle;
        }

        void cleanup(UINT_PTR uIdSubclass = 0) noexcept
        {
            if (handle && IsWindow(handle))
            {
                RemoveWindowSubclass(handle, Control::CustomButtonHandler, uIdSubclass);
            }

            HandleMessage = nullptr;
            handle = 0;
        }

        static lresult_t CALLBACK CustomButtonHandler(hwnd_t hWnd, std::uint32_t uMsg, wparam_t wParam,
            lparam_t lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
        {
            auto* child = reinterpret_cast<Control*>(dwRefData);

            if (child && child->HandleMessage)
            {
                auto result = child->HandleMessage(*child, uIdSubclass, make_window_message(uMsg, wParam, lParam));

                if (uMsg == destroy_message::id)
                {
                    child->cleanup();
                    return TRUE;
                }

                if (result.has_value())
                {
                    return result.value();
                }
            }

            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }

        control(hwnd_t handle, callback_type handler)
            : handle(handle), HandleMessage(std::move(handler))
        {
            SetWindowSubclass(handle, control::CustomButtonHandler, 0, std::bit_cast<DWORD_PTR>(this));
        }

        control(DLGITEMTEMPLATE params, hwnd_t parent, std::wstring caption, callback_type handler)
            : control(CreateWindowExW(
                params.dwExtendedStyle,
                Control::class_name,
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
            ), std::move(handler)) 
        {
        }

        control(CREATESTRUCTW params, callback_type handler)
            : control(CreateWindowExW(
                params.dwExStyle,
                params.lpszClass ? params.lpszClass : Control::class_name,
                params.lpszName,
                params.style,
                params.x,
                params.y,
                params.cx,
                params.cy,
                params.hwndParent,
                params.hMenu,
                params.hInstance != 0 ? params.hInstance : std::bit_cast<hinstance_t>(GetWindowLongPtrW(params.hwndParent, GWLP_HINSTANCE)),
                params.lpCreateParams
            ), std::move(handler)) 
        {
        }

        control(const control&) = delete;

        control(control&& other) noexcept
            : control(other.handle, std::move(other.HandleMessage))
        {
            other.handle = 0;
            other.HandleMessage = nullptr;
            if (IsWindow(handle))
            {
                SetWindowSubclass(handle, control::CustomButtonHandler, 0, std::bit_cast<DWORD_PTR>(this));
            }
        }

        control& operator=(control&& other) noexcept
        {
            this->handle = std::move(other.handle);
            this->HandleMessage = std::move(other.HandleMessage);
            return *this;
        }

        ~control() noexcept
        {
            cleanup();
        }
    };

    struct button : control<button>
    {
        using control::control;

        constexpr static auto class_name = WC_BUTTONW;
        constexpr static std::uint16_t dialog_id = 0x0080;
    };

    struct edit : control<edit>
    {
        using control::control;

        constexpr static auto class_name = WC_EDITW;
        constexpr static std::uint16_t dialog_id = 0x0081;
    };

    struct static_text : control<static_text>
    {
        using control::control;

        constexpr static auto class_name = WC_STATICW;
        constexpr static std::uint16_t dialog_id = 0x0082;
    };

    struct list_box : control<list_box>
    {
        using control::control;

        constexpr static auto class_name = WC_LISTBOXW;
        constexpr static std::uint16_t dialog_id = 0x0083;
    };

    struct scroll_bar : control<scroll_bar>
    {
        using control::control;
        
        constexpr static auto class_name = WC_SCROLLBARW;
        constexpr static std::uint16_t dialog_id = 0x0084;
    };

    struct combo_box : control<combo_box>
    {
        using control::control;

        constexpr static auto class_name = WC_COMBOBOXW;
        constexpr static std::uint16_t dialog_id = 0x0085;
    };

    struct combo_box_ex : control<combo_box_ex>
    {
        using control::control;

        constexpr static auto class_name = WC_COMBOBOXEXW;
    };

    struct header : control<header>
    {
        using control::control;

        constexpr static auto class_name = WC_HEADERW;
    };

    struct link : control<link>
    {
        using control::control;

        constexpr static auto class_name = WC_LINK;
    };

    struct ip_address : control<ip_address>
    {
        using control::control;

        constexpr static auto class_name = WC_IPADDRESSW;
    };

    struct list_view : control<list_view>
    {
        using control::control;

        constexpr static auto class_name = WC_LISTVIEWW;
    };

    struct native_font : control<native_font>
    {
        using control::control;

        constexpr static auto class_name = WC_NATIVEFONTCTLW;
    };

    struct page_scroller : control<page_scroller>
    {
        using control::control;

        constexpr static auto class_name = WC_PAGESCROLLERW;
    };

    struct tab_control : control<tab_control>
    {
        using control::control;

        constexpr static auto class_name = WC_TABCONTROLW;
    };

    struct tree_view : control<tree_view>
    {
        using control::control;

        constexpr static auto class_name = WC_TREEVIEWW;
    };

    using client_control = std::variant<button, combo_box, edit, list_box, scroll_bar, static_text>;


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