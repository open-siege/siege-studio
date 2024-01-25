#ifndef WIN32_CONTROLS_HPP
#define WIN32_CONTROLS_HPP

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
        

        window(class_descriptor descriptor, instance_descriptor params, callback_type handler) 
            : handle(0),
            HandleMessage(std::move(handler))
            {
                descriptor.cbSize = sizeof(class_descriptor);
                descriptor.lpfnWndProc = WindowHandler;
                
                if (GetClassInfoExW(descriptor.hInstance, descriptor.lpszClassName, &descriptor) == FALSE)
                {
                    RegisterClassExW(&descriptor);
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

        operator hwnd_t()
        {
            return handle;
        }

        static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
        {
            window* self = nullptr;

            if (message == pre_create_message::id)
            {
                auto* pCreate = std::bit_cast<instance_descriptor*>(lParam);
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

        [[maybe_unused]] static auto show_modal(HWND parent, LPWSTR templateName, callback_type handler)
        {
            dialog temp{std::move(handler)};
            auto hInstance = std::bit_cast<hinstance_t>(GetWindowLongPtrW(parent, GWLP_HINSTANCE));
            return DialogBoxParamW(hInstance, templateName, parent, dialog::HandleAboutDialogMessage, std::bit_cast<lparam_t>(&temp));
        }
        

        static INT_PTR CALLBACK HandleAboutDialogMessage(HWND hDlg, std::uint32_t message, wparam_t wParam, lparam_t lParam)
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

        static lresult_t CALLBACK CustomButtonHandler(hwnd_t hWnd, std::uint32_t uMsg, wparam_t wParam,
            lparam_t lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
        {
            auto* child = reinterpret_cast<Control*>(dwRefData);

            if (child && child->HandleMessage)
            {
                auto result = child->HandleMessage(*child, uIdSubclass, make_window_message(uMsg, wParam, lParam));

                if (uMsg == destroy_message::id)
                {
                    RemoveWindowSubclass(child->handle, Control::CustomButtonHandler, uIdSubclass);
                    child->HandleMessage = nullptr;
                    child->handle = 0;
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

        control(instance_descriptor params, callback_type handler)
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
    };

    struct button : control<button>
    {
        using control::control;

        constexpr static auto class_name = WC_BUTTONW;
        constexpr static std::int16_t dialog_id = 0x0080;
    };

    struct edit : control<edit>
    {
        using control::control;

        constexpr static auto class_name = WC_EDITW;
        constexpr static std::int16_t dialog_id = 0x0081;
    };

    struct static_text : control<static_text>
    {
        using control::control;

        constexpr static auto class_name = WC_STATICW;
        constexpr static std::int16_t dialog_id = 0x0082;
    };

    struct list_box : control<list_box>
    {
        using control::control;

        constexpr static auto class_name = WC_LISTBOXW;
        constexpr static std::int16_t dialog_id = 0x0083;
    };

    struct scroll_bar : control<scroll_bar>
    {
        using control::control;
        
        constexpr static auto class_name = WC_SCROLLBARW;
        constexpr static std::int16_t dialog_id = 0x0084;
    };

    struct combo_box : control<combo_box>
    {
        using control::control;

        constexpr static auto class_name = WC_COMBOBOXW;
        constexpr static std::int16_t dialog_id = 0x0085;
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
}

#endif