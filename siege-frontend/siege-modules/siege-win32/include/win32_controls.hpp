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

    auto widen(std::string_view data)
    {
        return std::wstring(data.begin(), data.end());
    }

    template <typename TType>
    auto type_name()
    {
        return widen(typeid(TType).name());
    }

    template <typename TWindow, int StaticSize = 0>
    auto RegisterClassExW(WNDCLASSEXW descriptor)
    {
        constexpr static auto data_size = sizeof(TWindow) / sizeof(LONG_PTR);
        constexpr static auto extra_size = sizeof(TWindow) % sizeof(LONG_PTR) == 0 ? 0 : 1;
        constexpr static auto total_size = (data_size + extra_size) * sizeof(LONG_PTR);
        constexpr static auto can_fit = std::is_trivially_copyable_v<TWindow> && total_size <= 40;

        struct handler
        {
            static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
            {
                TWindow* self = nullptr;

                auto do_dispatch = [&]() -> lresult_t {
                       std::optional<lresult_t> result = std::nullopt;

                    if (self)
                    {
                        result = dispatch_message(self, message, wParam, lParam);

                        if (message == WM_NCDESTROY)
                        {
                          self->~TWindow();

                          if constexpr (!can_fit)
                          {
                            auto heap = std::bit_cast<HANDLE>(GetWindowLongPtrW(hWnd, 0));
                            //auto size = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
                            auto data = std::bit_cast<TWindow*>(GetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR)));

                            ::HeapFree(heap, 0, data);
                          }
                          return 0;
                        }
                    }
                    
                    return result.or_else([&]{ return std::make_optional(DefWindowProc(hWnd, message, wParam, lParam)); }).value();
                };

                if constexpr (can_fit)
                {
                    std::array<LONG_PTR, data_size + extra_size> raw_data{};

                    if (message == pre_create_message::id)
                    {
                        auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

                        self = new (raw_data.data()) TWindow(hWnd, *pCreate);

                        for (auto i = 0; i < raw_data.size(); i++)
                        {
                            SetWindowLongPtrW(hWnd, i * sizeof(LONG_PTR), raw_data[i]);
                        }
                    }
                    else
                    {
                        for (auto i = 0; i < raw_data.size(); i++)
                        {
                            raw_data[i] = GetWindowLongPtrW(hWnd, i * sizeof(LONG_PTR));
                        }

                        self = std::bit_cast<TWindow*>(raw_data.data());
                    }

                    return do_dispatch();
                }
                else
                {
                    if (message == pre_create_message::id)
                    {
                        auto heap = ::GetProcessHeap();
                        auto size = sizeof(TWindow);
                        auto raw_data = ::HeapAlloc(heap, 0, size);

                        auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

                        self = new (raw_data) TWindow(hWnd, *pCreate);

                        SetWindowLongPtrW(hWnd, 0, std::bit_cast<LONG_PTR>(heap));
                        SetWindowLongPtrW(hWnd, sizeof(LONG_PTR), size);
                        SetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR), raw_data);
                    }
                    else
                    {
                        //auto heap = GetWindowLongPtrW(hWnd, 0);
                        //auto size = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
                        self = std::bit_cast<TWindow*>(GetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR)));
                    }

                    return do_dispatch();
                }
            }
        };

        descriptor.cbSize = sizeof(WNDCLASSEXW);
        descriptor.lpfnWndProc = handler::WindowHandler;
        auto window_type_name = type_name<TWindow>();
        descriptor.lpszClassName = descriptor.lpszClassName ? descriptor.lpszClassName : window_type_name.c_str();

        static_assert(StaticSize <= 40, "StaticSize is too big for cbClsExtra");
        descriptor.cbClsExtra = StaticSize;

        if constexpr (can_fit)
        {
            static_assert(total_size <= 40, "TWindow is too big for cbWndExtra");
            static_assert(std::is_trivially_copyable_v<TWindow>, "TWindow must be trivially copyable");
            descriptor.cbWndExtra = int(total_size);
        }
        else
        {
            descriptor.cbWndExtra = sizeof(std::size_t) * 3;
        }
        
        return ::RegisterClassExW(&descriptor);
    }

    template <typename TWindow>
    auto RegisterStaticClassExW(WNDCLASSEXW descriptor)
    {
        constexpr static auto data_size = sizeof(TWindow) / sizeof(LONG_PTR);
        constexpr static auto extra_size = sizeof(TWindow) % sizeof(LONG_PTR) == 0 ? 0 : 1;
        constexpr static auto total_size = (data_size + extra_size) * sizeof(LONG_PTR);
        constexpr static auto can_fit = std::is_trivially_copyable_v<TWindow> && total_size <= 40 - sizeof(LONG_PTR);
        
        struct handler
        {
            static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
            {
                TWindow* self = nullptr;

                auto do_dispatch = [&]() -> lresult_t
                {
                    std::optional<lresult_t> result = std::nullopt;

                    if (self)
                    {
                        result = dispatch_message(self, message, wParam, lParam);

                        if (message == WM_NCDESTROY)
                        {
                            auto ref_count = GetClassLongPtrW(hWnd, 0);
                            ref_count--;

                            if (ref_count == 0)
                            {
                                self->~TWindow();

                                if constexpr (!can_fit)
                                {
                                    auto heap = std::bit_cast<HANDLE>(GetClassLongPtrW(hWnd, sizeof(LONG_PTR)));
                                    //auto size = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
                                    auto data = std::bit_cast<TWindow*>(GetClassLongPtrW(hWnd, 3 * sizeof(LONG_PTR)));

                                    ::HeapFree(heap, 0, data);
                                }
                            }

                            SetClassLongPtrW(hWnd, 0, ref_count);
                            return 0;
                        }
                    }
                    
                    return result.or_else([&]{ return std::make_optional(DefWindowProc(hWnd, message, wParam, lParam)); }).value();  
                };
          
                if constexpr (can_fit)
                {
                    std::array<LONG_PTR, data_size + extra_size> raw_data{};

                    if (message == pre_create_message::id)
                    {
                        auto ref_count = GetClassLongPtrW(hWnd, 0);
                        auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

                        if (ref_count == 0)
                        {
                            self = new (raw_data.data()) TWindow(hWnd, *pCreate);

                            for (auto i = 0; i < raw_data.size(); i++)
                            {
                                SetClassLongPtrW(hWnd, (i + 1) * sizeof(LONG_PTR), raw_data[i]);
                            }

                        }
                        ref_count++;

                        SetClassLongPtrW(hWnd, 0, ref_count);
                    }
                    else
                    {
                        for (auto i = 0; i < raw_data.size(); i++)
                        {
                            raw_data[i] = GetClassLongPtrW(hWnd, i * sizeof(LONG_PTR));
                        }

                        self = std::bit_cast<TWindow*>(raw_data.data());
                    }

                    return do_dispatch();
                }
                else
                {
                    if (message == pre_create_message::id)
                    {
                        auto ref_count = GetClassLongPtrW(hWnd, 0);

                        if (ref_count == 0)
                        {
                            auto heap = ::GetProcessHeap();
                            auto size = sizeof(TWindow);
                            auto raw_data = ::HeapAlloc(heap, 0, size);

                            auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

                            self = new (raw_data) TWindow(hWnd, *pCreate);

                            SetClassLongPtrW(hWnd, sizeof(LONG_PTR), std::bit_cast<LONG_PTR>(heap));
                            SetClassLongPtrW(hWnd, 2 * sizeof(LONG_PTR), size);
                            SetClassLongPtrW(hWnd, 3 * sizeof(LONG_PTR), std::bit_cast<LONG_PTR>(raw_data));
                        }

                        ref_count++;
                        SetClassLongPtrW(hWnd, 0, ref_count);
                    }
                    else
                    {
                        //auto heap = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR);
                        //auto size = GetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR));
                        self = std::bit_cast<TWindow*>(GetClassLongPtrW(hWnd, 3 * sizeof(LONG_PTR)));
                    }

                    return do_dispatch();
                }
            }
        };

        descriptor.cbSize = sizeof(WNDCLASSEXW);
        descriptor.lpfnWndProc = handler::WindowHandler;
        descriptor.cbWndExtra = 0;

        if constexpr (can_fit)
        {
            static_assert(total_size <= 40 - sizeof(LONG_PTR), "TWindow is too big for cbClsExtra");
            static_assert(std::is_trivially_copyable_v<TWindow>, "TWindow must be trivially copyable");
            descriptor.cbClsExtra = int(total_size + sizeof(LONG_PTR));
        }
        else
        {
            descriptor.cbClsExtra =  sizeof(std::size_t) * 4;
        }
     
        return ::RegisterClassExW(&descriptor);
    }

    template <typename TWindow>
    auto UnregisterClassW(HINSTANCE instance)
    {
        return ::UnregisterClassW(type_name<TWindow>().c_str(), instance);
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

    [[maybe_unused]] auto EnumPropsExW(hwnd_t control, std::move_only_function<bool(hwnd_t, std::wstring_view, HANDLE)> callback)
    {
        struct Handler
        {
            static BOOL HandleEnum(hwnd_t self, LPWSTR key, HANDLE data, ULONG_PTR raw_callback)
            {
                if (key == nullptr)
                {
                    return TRUE;
                }

                auto real_callback = std::bit_cast<std::move_only_function<bool(hwnd_t, std::wstring_view, HANDLE)>*>(raw_callback);

                return real_callback->operator()(self, key, data) ? TRUE : FALSE;
            }
        };

        return ::EnumPropsExW(control, Handler::HandleEnum, std::bit_cast<LPARAM>(&callback));
    }

    [[maybe_unused]] auto ForEachPropertyExW(hwnd_t control, std::move_only_function<void(hwnd_t, std::wstring_view, HANDLE)> callback)
    {
        return EnumPropsExW(control, [callback = std::move(callback)] (auto self, auto key, auto value) mutable
        {
            callback(self, key, value);
            return true;
        });
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
}

#endif