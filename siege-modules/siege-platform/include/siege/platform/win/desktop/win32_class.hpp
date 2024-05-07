#ifndef WIN32_CLASS_HPP
#define WIN32_CLASS_HPP

#include "win32_messages.hpp"

namespace win32
{
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


    template <typename TWindow, int StaticSize = 0>
    struct window_meta_class : WNDCLASSEXW
    {
        window_meta_class() : WNDCLASSEXW
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

            this->cbSize = sizeof(WNDCLASSEXW);
            this->lpfnWndProc = handler::WindowHandler;
            auto window_type_name = type_name<TWindow>();
            this->lpszClassName = this->lpszClassName ? this->lpszClassName : window_type_name.c_str();

            static_assert(StaticSize <= 40, "StaticSize is too big for cbClsExtra");
            this->cbClsExtra = StaticSize;

            if constexpr (can_fit)
            {
                static_assert(total_size <= 40, "TWindow is too big for cbWndExtra");
                static_assert(std::is_trivially_copyable_v<TWindow>, "TWindow must be trivially copyable");
                this->cbWndExtra = int(total_size);
            }
            else
            {
                this->cbWndExtra = sizeof(std::size_t) * 3;
            }
        }
    }

    template <TWindow>
    struct static_window_meta_class : ::WNDCLASSEXW
    {
        static_window_meta_class() : ::WNDCLASSEXW{}
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

            this->cbSize = sizeof(WNDCLASSEXW);
            this->lpfnWndProc = handler::WindowHandler;
            this->cbWndExtra = 0;

            if constexpr (can_fit)
            {
                static_assert(total_size <= 40 - sizeof(LONG_PTR), "TWindow is too big for cbClsExtra");
                static_assert(std::is_trivially_copyable_v<TWindow>, "TWindow must be trivially copyable");
                this->cbClsExtra = int(total_size + sizeof(LONG_PTR));
            }
            else
            {
                this->cbClsExtra =  sizeof(std::size_t) * 4;
            }
        }
    }
}

#endif