#ifndef WIN32_USER32_HPP
#define WIN32_USER32_HPP

#include <deque>
#include <vector>
#include <span>
#include <utility>
#include <span>
#include <optional>
#include <map>
#include <string>
#include <expected>
#include <algorithm>
#include <cassert>
#include <memory_resource>
#include "win32_messages.hpp"

namespace win32
{
    using hinstance_t = HINSTANCE;
    using lresult_t = LRESULT;

    template<typename TWindow>
    std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
    {
        if constexpr (requires(TWindow t) { t.on_non_client_create(non_client_create_message{wParam, lParam}); })
        {
            if (message == non_client_create_message::id)
            {
                return self->on_non_client_create(non_client_create_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_create(create_message{wParam, lParam}); })
        {
            if (message == create_message::id)
            {
                return self->on_create(create_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_paint(paint_message{wParam, lParam}); })
        {
            if (message == paint_message::id)
            {
                return self->on_paint(paint_message{wParam, lParam});
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

        if constexpr (requires(TWindow t) { t.on_copy_data(copy_data_message<char>{wParam, lParam}); })
        {
            if (message == copy_data_message<char>::id)
            {
                return self->on_copy_data(copy_data_message<char>{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_get_object(get_object_message{wParam, lParam}); })
        {
            if (message == get_object_message::id)
            {
                return self->on_get_object(get_object_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_command(command_message{wParam, lParam}); })
        {
            if (message == command_message::id)
            {
                return self->on_command(command_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_notify(notify_message{wParam, lParam}); })
        {
            if (message == notify_message::id)
            {
                return self->on_notify(notify_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_size(size_message{wParam, lParam}); })
        {
            if (message == size_message::id)
            {
                return self->on_size(size_message{wParam, lParam});
            }
        }

        if constexpr (requires(TWindow t) { t.on_pos_changed(pos_changed_message{wParam, lParam}); })
        {
            if (message == pos_changed_message::id)
            {
                return self->on_pos_changed(pos_changed_message{wParam, lParam});
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

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    [[maybe_unused]] inline auto EnumPropsExW(hwnd_t control, std::move_only_function<bool(hwnd_t, std::wstring_view, HANDLE)> callback)
    {
        struct Handler
        {
            inline static BOOL HandleEnum(hwnd_t self, LPWSTR key, HANDLE data, ULONG_PTR raw_callback)
            {
                if (key == nullptr)
                {
                    return TRUE;
                }

                auto real_callback = std::bit_cast<std::move_only_function<bool(hwnd_t, std::wstring_view, HANDLE)>*>(raw_callback);


                std::wstring_view keyView;

                auto intAtom = ::GlobalFindAtomW(key);
                if (intAtom == (ATOM)key)
                {
                    thread_local std::array<wchar_t, 256> temp;
                    auto size = GlobalGetAtomNameW(intAtom, temp.data(), temp.size());
                    temp[size] = 0;
                    keyView = temp.data();
                }
                else
                {
                    keyView = key;
                }

                return real_callback->operator()(self, keyView, data) ? TRUE : FALSE;
            }
        };

        return ::EnumPropsExW(control, Handler::HandleEnum, std::bit_cast<LPARAM>(&callback));
    }

    [[maybe_unused]] inline auto ForEachPropertyExW(hwnd_t control, std::move_only_function<void(hwnd_t, std::wstring_view, HANDLE)> callback)
    {
        return EnumPropsExW(control, [callback = std::move(callback)] (auto self, auto key, auto value) mutable
        {
            callback(self, key, value);
            return true;
        });
    }

    [[maybe_unused]] inline auto FindPropertyExW(hwnd_t control, std::move_only_function<bool(hwnd_t, std::wstring_view, HANDLE)> callback)
    {
        return EnumPropsExW(control, [callback = std::move(callback)] (auto self, auto key, auto value) mutable
        {
            return callback(self, key, value) != false;
        });
    }
#endif

    inline auto widen(std::string_view data)
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

                          ForEachPropertyExW(hWnd, [](auto wnd, std::wstring_view name, HANDLE handle) {
                              ::RemovePropW(wnd, name.data());
                          });
                          return 0;
                        }
                    }
                    
                    return result.or_else([&]{ return std::make_optional(DefWindowProc(hWnd, message, wParam, lParam)); }).value();
                };

                if constexpr (can_fit)
                {
                    std::array<LONG_PTR, data_size + extra_size> raw_data{};

                    if (message == non_client_create_message::id)
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
                    if (message == non_client_create_message::id)
                    {
                        auto heap = ::GetProcessHeap();
                        auto size = sizeof(TWindow);
                        auto raw_data = ::HeapAlloc(heap, 0, size);

                        auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

                        self = new (raw_data) TWindow(hWnd, *pCreate);

                        SetWindowLongPtrW(hWnd, 0, std::bit_cast<LONG_PTR>(heap));
                        SetWindowLongPtrW(hWnd, sizeof(LONG_PTR), size);
                        SetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR), std::bit_cast<LONG_PTR>(raw_data));
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

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
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

                                ForEachPropertyExW(hWnd, [](auto wnd, std::wstring_view name, HANDLE handle) {
                                    ::RemovePropW(wnd, name.data());
                                });
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

                    if (message == non_client_create_message::id)
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
                    if (message == non_client_create_message::id)
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
#endif

    template <typename TWindow>
    auto UnregisterClassW(HINSTANCE instance)
    {
        return ::UnregisterClassW(type_name<TWindow>().c_str(), instance);
    }

    enum struct window_style : DWORD
    {
        border = WS_BORDER,
        caption = WS_CAPTION,
        child = WS_CHILD,
        clip_children = WS_CLIPCHILDREN,
        clip_siblings = WS_CLIPSIBLINGS,
        disabled = WS_DISABLED,
        dlg_frame = WS_DLGFRAME,
        group = WS_GROUP,
        overlapped = WS_OVERLAPPED,
        popup = WS_POPUP,
        visible = WS_VISIBLE
    };

    enum struct extended_window_style : DWORD
    {
    
    };

    template <typename TControl = hwnd_t>
    std::expected<TControl, DWORD> CreateWindowExW(CREATESTRUCTW params)
    {
        hinstance_t hinstance = nullptr;
        auto parent_hinstance = params.hwndParent != nullptr ? std::bit_cast<hinstance_t>(GetWindowLongPtrW(params.hwndParent, GWLP_HINSTANCE)) : nullptr;
        auto module_instance = ::GetModuleHandleW(nullptr);
        thread_local WNDCLASSEXW info{sizeof(WNDCLASSEXW)};

        std::array<HMODULE, 5> modules_to_check = {{
                  params.hInstance,
                  module_instance,  
                  parent_hinstance,
                  ::GetModuleHandleW(L"comctrl32.dll"),
                  nullptr
            }};
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        for (auto module : modules_to_check)
        {
            if (module && ::GetClassInfoExW(module, params.lpszClass, &info))
            {
                hinstance = module;
                break;
            }   
        }
#endif

        auto result = ::CreateWindowExW(
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

        if (!result)
        {
            return std::unexpected(GetLastError());
        }

        return TControl(result);
    }
    #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    template <typename TControl = hwnd_t>
	auto CreateWindowExW(DLGITEMTEMPLATE params, hwnd_t parent, std::wstring class_name, std::wstring caption)
    {
        return CreateWindowExW<TControl>(CREATESTRUCTW{
            .hwndParent = parent,
            .cy = params.cy,
            .cx = params.cx,
            .y = params.y,
            .x = params.x,
            .style = LONG(params.style),
            .lpszName = caption.c_str(),
            .lpszClass = class_name.c_str(),
            .dwExStyle = params.dwExtendedStyle
            });
    }
	#endif

    template<typename TPosition = LONG, typename TSize = LONG>
    struct window_params
    {
        hwnd_t parent;
        std::wstring class_name;
        std::optional<hinstance_t> class_module = std::nullopt;
        std::wstring caption = L"";
        std::optional<window_style> style = std::nullopt;
        std::optional<extended_window_style> extended_style = std::nullopt;
        TPosition position;
        TSize size;

        std::optional<window_style> default_style() const
        {
            if (parent && parent != HWND_MESSAGE && !style)
            {
                return window_style::child;
            }

            return style;
        }
    };

    template <typename TControl = hwnd_t>
    auto CreateWindowExW(window_params<POINT, SIZE> params)
    {
        return CreateWindowExW<TControl>(CREATESTRUCTW{
            .hInstance = params.class_module ? *params.class_module : nullptr,
            .hwndParent = params.parent,
            .cy = params.size.cy,
            .cx = params.size.cx,
            .y = params.position.y,
            .x = params.position.x,
            .style = params.default_style() ? LONG(*params.default_style()) : 0,
            .lpszName = params.caption.c_str(),
            .lpszClass = params.class_name.c_str(),
            .dwExStyle = params.extended_style ? DWORD(*params.extended_style) : 0
            });
    }

    template <typename TControl = hwnd_t>
     auto CreateWindowExW(window_params<RECT> params)
     {
        return CreateWindowExW<TControl>(CREATESTRUCTW{
            .hInstance = params.class_module ? *params.class_module : nullptr,
            .hwndParent = params.parent,
            .cy = params.position.bottom - params.position.top,
            .cx = params.position.right - params.position.left,
            .y = params.position.top,
            .x = params.position.left,
            .style = params.default_style() ? LONG(*params.default_style()) : 0,
            .lpszName = params.caption.c_str(),
            .lpszClass = params.class_name.c_str(),
            .dwExStyle = params.extended_style ? DWORD(*params.extended_style) : 0
            });
     }

     template <typename TControl = hwnd_t>
    auto CreateWindowExW(window_params<> params)
    {
        return CreateWindowExW<TControl>(CREATESTRUCTW{
            .hInstance = params.class_module ? *params.class_module : nullptr,
            .hwndParent = params.parent,
            .cy = LONG(params.size),
            .cx = LONG(params.size),
            .y = LONG(params.position),
            .x = LONG(params.position),
            .style = params.style ? LONG(*params.style) : 0,
            .lpszName = params.caption.c_str(),
            .lpszClass = params.class_name.c_str(),
            .dwExStyle = params.extended_style ? DWORD(*params.extended_style) : 0
            });
    }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    inline auto DialogBoxIndirectParamW(hwnd_t parent, DLGTEMPLATE* dialog_template, std::move_only_function<INT_PTR(hwnd_t, win32::message)> on_message)
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
                    return child->operator()(hDlg, win32::message(message, wParam, lParam));
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

    struct EnumWindowHandler
    {
        inline static BOOL CALLBACK HandleWindow(hwnd_t window, LPARAM raw_callback)
        {
            auto real_callback = std::bit_cast<std::move_only_function<bool(hwnd_t)>*>(raw_callback);

            return real_callback->operator()(window) ? TRUE : FALSE;
        }
    };

    [[maybe_unused]] inline auto EnumWindows(std::move_only_function<bool(hwnd_t)> callback)
    {
        return ::EnumWindows(EnumWindowHandler::HandleWindow, std::bit_cast<LPARAM>(&callback));
    }

    [[maybe_unused]] inline auto EnumChildWindows(hwnd_t parent, std::move_only_function<bool(hwnd_t)> callback)
    {
        return ::EnumChildWindows(parent, EnumWindowHandler::HandleWindow, std::bit_cast<LPARAM>(&callback));
    }

    [[maybe_unused]] inline auto EnumThreadWindows(DWORD thread_id, std::move_only_function<bool(hwnd_t)> callback)
    {
        return ::EnumThreadWindows(thread_id, EnumWindowHandler::HandleWindow, std::bit_cast<LPARAM>(&callback));
    }

    [[maybe_unused]] inline auto ForEachWindow(std::move_only_function<void(hwnd_t)> callback)
    {
        return EnumWindows([callback = std::move(callback)] (auto window) mutable
        {
            callback(window);
            return true;
        });
    }

    [[maybe_unused]] inline auto ForEachChildWindow(hwnd_t parent, std::move_only_function<void(hwnd_t)> callback)
    {
        return EnumChildWindows(parent, [callback = std::move(callback)] (auto window) mutable
        {
            callback(window);
            return true;
        });
    }

    [[maybe_unused]] inline auto ForEachThreadWindow(DWORD thread_id, std::move_only_function<void(hwnd_t)> callback)
    {
        return EnumThreadWindows(thread_id, [callback = std::move(callback)] (auto window) mutable
        {
            callback(window);
            return true;
        });
    }

    [[maybe_unused]] inline auto FindWindow(std::move_only_function<bool(hwnd_t)> callback)
    {
        hwnd_t result = nullptr;
        EnumWindows([callback = std::move(callback), &result] (auto window) mutable
        {
            if (callback(window))
            {
                result = window;
                return false;
            }
            return true;
        });

        return result;
    }

    [[maybe_unused]] inline auto FindThreadWindow(DWORD thread_id, std::move_only_function<bool(hwnd_t)> callback)
    {
        hwnd_t result = nullptr;
        
        EnumThreadWindows(thread_id, [callback = std::move(callback), &result] (auto window) mutable
        {
            if (callback(window))
            {
                result = window;
                return false;
            }

            return true;
        });

        return result;
    }

    inline hwnd_t FindDirectChildWindow(hwnd_t parent, std::wstring_view class_name, std::wstring_view title, std::move_only_function<bool(hwnd_t)> callback)
    {
        hwnd_t current_child = nullptr;

        const wchar_t* class_name_data = class_name.empty() ? nullptr : class_name.data();
        const wchar_t* title_data = class_name.empty() ? nullptr : title.data();

        do  {
            current_child = ::FindWindowExW(parent, current_child, class_name_data, title_data);

            if (current_child && callback(current_child))
            {
                return current_child;
            }
        }
        while (current_child != nullptr);    

        return nullptr;
    }

    inline auto FindDirectChildWindow(hwnd_t parent, std::wstring_view class_name, std::move_only_function<bool(hwnd_t)> callback)
    {
        return FindDirectChildWindow(parent, class_name, L"", std::move(callback));
    }

    inline auto FindDirectChildWindow(hwnd_t parent, std::move_only_function<bool(hwnd_t)> callback)
    {
        return FindDirectChildWindow(parent, L"", L"", std::move(callback));
    }

    inline auto ForEachDirectChildWindow(hwnd_t parent, std::move_only_function<void(hwnd_t)> callback)
    {
        return EnumChildWindows(parent, [callback = std::move(callback), parent] (auto window) mutable
        {
            if (::GetParent(window) == parent)
            {
                callback(window);
            }

            return true;
        });
    }

    [[maybe_unused]] inline auto FindChildWindow(hwnd_t parent, std::move_only_function<bool(hwnd_t)> callback)
    {
        if (parent == HWND_MESSAGE)
        {
            return FindDirectChildWindow(parent, std::move(callback));
        }

        hwnd_t result = nullptr;

        EnumChildWindows(parent, [callback = std::move(callback), &result] (auto window) mutable
        {
            if (callback(window))
            {
                result = window;
                return false;
            }

            return true;
        });

        return result;
    }
#endif

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    inline auto TrackPopupMenuEx(HMENU menu, UINT flags, POINT coords, hwnd_t owner, std::optional<TPMPARAMS> params = std::nullopt)
    {
        if (params)
        {
            params.value().cbSize = sizeof(TPMPARAMS);
            return ::TrackPopupMenuEx(menu, flags, coords.x, coords.y, owner, &params.value());
        }
        else
        {
            return ::TrackPopupMenuEx(menu, flags, coords.x, coords.y, owner, nullptr);
        }
    }

    inline std::optional<std::pair<POINT, RECT>> MapWindowPoints(hwnd_t from, hwnd_t to, RECT source)
    {
        auto result = ::MapWindowPoints(from, to, reinterpret_cast<POINT*>(&source), sizeof(RECT) / sizeof(POINT));

        if (result)
        {
            return std::make_pair(POINT{LOWORD(result), HIWORD(result)}, source);
        }

        return std::nullopt;
    }
#endif
}

#endif