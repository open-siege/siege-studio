#ifndef WIN32_WINDOW_HPP
#define WIN32_WINDOW_HPP

#include <siege/platform/win/desktop/messages.hpp>
#include <siege/platform/win/auto_handle.hpp>

#undef GetFirstSibling
#undef GetNextSibling
#undef GetWindowStyle

namespace win32
{
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

    struct window_deleter
    {
        void operator()(hwnd_t window) 
        {
            #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            if (::GetParent(window) == nullptr)
            #endif
            {
                ::DestroyWindow(window);            
            }
        }
    };

    struct window_no_deleter
    {
        void operator()(hwnd_t window) 
        {
        }
    };

    struct window_ref;

    template<typename TDeleter, typename window_ref>
	struct window_base : win32::auto_handle<hwnd_t, TDeleter>
	{
        using base = win32::auto_handle<hwnd_t, TDeleter>;
        using base::base;

        operator hwnd_t() const
        {
            return this->get();
        }

        window_ref ref() const
        {
            return window_ref(*this);
        }

        #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

        inline std::optional<window_ref> GetAncestor(std::uint32_t flags) const
        {
            auto parent = ::GetAncestor(*this, flags);

            if (!parent)
            {
                return std::nullopt;
            }

            return window_ref(parent);
        }

        inline std::optional<window_ref> GetParent() const
        {
            auto parent = ::GetParent(*this);

            if (!parent)
            {
                return std::nullopt;
            }

            return window_ref(parent);
        }

        inline std::optional<window_ref> GetWindow(std::uint32_t type) const
        {
            auto parent = ::GetWindow(*this, type);

            if (!parent)
            {
                return std::nullopt;
            }

            return window_ref(parent);
        }

        inline std::optional<window_ref> GetChild() const
        {
            return GetWindow(GW_CHILD);
        }


        inline std::optional<window_ref> GetFirstSibling() const
        {
            return GetWindow(GW_HWNDFIRST);
        }

        inline std::optional<window_ref> GetNextSibling() const
        {
            return GetWindow(GW_HWNDNEXT);
        }

        inline std::optional<RECT> GetWindowRect() const
        {
            RECT result;
            if (::GetWindowRect(*this, &result))
            {
                return result;
            }

            return std::nullopt;
        }

        inline std::optional<RECT> MapDialogRect(RECT raw) const
        {
            if (::MapDialogRect(*this, &raw))
            {
                return raw;
            }

            return std::nullopt;
        }

        inline bool CopyData(win32::hwnd_t sender, ::COPYDATASTRUCT data)
        {
            return SendMessageW(*this, WM_COPYDATA, win32::wparam_t(win32::hwnd_t(sender)), win32::lparam_t(&data));        
        }

        inline auto GetWindowStyle() const
        {
            return ::GetWindowLongPtrW(*this, GWL_STYLE);
        }

        inline auto SetWindowStyle(LONG_PTR style)
        {
            return ::SetWindowLongPtrW(*this, GWL_STYLE, style);
        }

        template <typename TValue>
        inline TValue GetPropW(std::wstring_view name) const
        {
            if constexpr(std::is_integral_v<TValue> || std::is_enum_v<TValue> || std::is_pointer_v<TValue>)
            {
                auto raw = std::size_t(::GetPropW(*this, name.data()));
                return TValue(raw);
            }

            return TValue(); 
        }

        template <typename TValue>
        inline auto SetPropW(std::wstring_view name, TValue value)
        {
            if constexpr(std::is_integral_v<TValue> || std::is_enum_v<TValue> || std::is_pointer_v<TValue>)
            {
                return ::SetPropW(*this, name.data(), HANDLE(value));                 
            }

            return ::SetProp(*this, name.data(), nullptr);
        }

        template <typename TValue = HANDLE>
        [[maybe_unused]] inline TValue RemovePropW(std::wstring_view name)
        {
            return reinterpret_cast<TValue>(::RemovePropW(*this, name.data()));
        }

        #endif

        inline std::optional<RECT> GetClientRect() const
        {
            RECT result;
            if (::GetClientRect(*this, &result))
            {
                return result;
            }

            return std::nullopt;
        }

        inline std::optional<std::pair<POINT, RECT>> MapWindowPoints(hwnd_t to, RECT source)
        {
            auto result = ::MapWindowPoints(*this, to, reinterpret_cast<POINT*>(&source), sizeof(RECT) / sizeof(POINT));

            if (result)
            {
                return std::make_pair(POINT{LOWORD(result), HIWORD(result)}, source);
            }

            return std::nullopt;
        }

        inline std::optional<SIZE> GetClientSize() const
        {
            auto rect = GetClientRect();

            if (rect)
            {
               return SIZE {.cx = rect->right - rect->left, .cy = rect->bottom - rect->top };                
            }

            return std::nullopt;
        }

        inline std::optional<std::pair<POINT, SIZE>> GetClientPositionAndSize() const
        {
            RECT result;
            if (::GetClientRect(*this, &result))
            {
                return std::make_pair(POINT{.x = result.left, .y = result.top}, SIZE {.cx = result.right - result.left, .cy = result.bottom - result.top });
            }

            return std::nullopt;
        }

        inline auto SetWindowPos(hwnd_t hWndInsertAfter, UINT uFlags = 0)
        {
            return ::SetWindowPos(*this, hWndInsertAfter, 0, 0, 0, 0, uFlags | SWP_NOMOVE | SWP_NOSIZE);
        }

        inline auto SetWindowPos(RECT position_and_size, UINT uFlags = 0)
        {
            return ::SetWindowPos(*this, nullptr, position_and_size.left, 
                position_and_size.top, 
                position_and_size.right - position_and_size.left, 
                position_and_size.bottom - position_and_size.top, uFlags | SWP_NOZORDER);
        }

        inline auto SetWindowPos(POINT position, SIZE size, UINT uFlags = 0)
        {
            return ::SetWindowPos(*this, nullptr, position.x, position.y, size.cx, size.cy, uFlags | SWP_NOZORDER);
        }

        inline auto SetWindowPos(POINT position, UINT uFlags = 0)
        {
            return ::SetWindowPos(*this, nullptr, position.x, position.y, 0, 0, uFlags | SWP_NOSIZE | SWP_NOZORDER);
        }

        inline auto SetWindowPos(SIZE size, UINT uFlags = 0)
        {
            return ::SetWindowPos(*this, nullptr, 0, 0, size.cx, size.cy, uFlags | SWP_NOMOVE | SWP_NOZORDER);
        }

        inline auto MoveWindow(RECT position_and_size, bool repaint)
        {
            return ::MoveWindow(*this, position_and_size.left, position_and_size.top, position_and_size.right - position_and_size.left, position_and_size.bottom - position_and_size.top, repaint ? TRUE : FALSE);
        }

        inline auto MoveWindow(POINT position, SIZE size, bool repaint)
        {
            return ::MoveWindow(*this, position.x, position.y, size.cx, size.cy, repaint ? TRUE : FALSE);
        }

        #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        [[maybe_unused]] inline auto EnumPropsExW(hwnd_t control, std::move_only_function<bool(hwnd_t, std::wstring_view, HANDLE)> callback)
        {
            struct Handler
            {
                inline static BOOL CALLBACK HandleEnum(hwnd_t self, LPWSTR key, HANDLE data, ULONG_PTR raw_callback)
                {
                    if (key == nullptr)
                    {
                        return TRUE;
                    }

                    auto real_callback = std::bit_cast<std::move_only_function<bool(hwnd_t, std::wstring_view, HANDLE)>*>(raw_callback);


                    std::wstring_view keyView;

                    auto intAtom = ::GlobalFindAtomW(key);
                    if (intAtom == (ATOM)(std::size_t)key)
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

        [[maybe_unused]] inline auto ForEachPropertyExW(std::move_only_function<void(hwnd_t, std::wstring_view, HANDLE)> callback)
        {
            return EnumPropsExW(*this, [callback = std::move(callback)] (auto self, auto key, auto value) mutable
            {
                callback(self, key, value);
                return true;
            });
        }

        [[maybe_unused]] inline auto FindPropertyExW(std::move_only_function<bool(hwnd_t, std::wstring_view, HANDLE)> callback)
        {
            return EnumPropsExW(*this, [callback = std::move(callback)] (auto self, auto key, auto value) mutable
            {
                return callback(self, key, value) != false;
            });
        }
        #endif

	};

    struct window_ref : window_base<window_no_deleter, window_ref>
    {
        using base = window_base<window_no_deleter, window_ref>;
        using base::base;
    };

    struct window : window_base<window_deleter, window_ref>
    {
        using base = window_base<window_deleter, window_ref>;
        using base::base;
    };
}


#endif // !WIN32_WINDOW_HPP
