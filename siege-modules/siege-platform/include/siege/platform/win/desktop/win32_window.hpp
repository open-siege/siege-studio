#ifndef WIN32_WINDOW_HPP
#define WIN32_WINDOW_HPP

#include <siege/platform/win/desktop/win32_user32.hpp>
#include <siege/platform/win/auto_handle.hpp>

#undef GetFirstSibling
#undef GetNextSibling

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

            operator ::CREATESTRUCTW() const
            {
                auto& params = *this;
                if constexpr (std::is_same_v<TPosition, POINT> && std::is_same_v<TSize, SIZE>)
                {
                    return CREATESTRUCTW{
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
                    };
                }
                else if constexpr (std::is_same_v<TPosition, RECT>)
                {
                    return CREATESTRUCTW{
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
                    };
                }
                else
                {
                    return CREATESTRUCTW{
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
                    };
                }
            }
     };

    using window_point_size = window_params<POINT, SIZE>;
    using window_rect = window_params<RECT>;

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

    struct no_deleter
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

        inline std::optional<SIZE> GetClientSize() const
        {
            RECT result;
            if (::GetClientRect(*this, &result))
            {
                return SIZE {.cx = result.right - result.left, .cy = result.bottom - result.top };
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
	};

    struct window_ref : window_base<no_deleter, window_ref>
    {
        using base = window_base<no_deleter, window_ref>;
        using base::base;
    };

    struct window : window_base<window_deleter, window_ref>
    {
        using base = window_base<window_deleter, window_ref>;
        using base::base;
    };
    
    enum struct StackDirection
    {
        Vertical,
        Horizontal
    };

    inline void StackChildren(SIZE parent_size, std::span<window_ref> children, StackDirection direction = StackDirection::Vertical, std::optional<POINT> start_pos = std::nullopt)
    {
        std::stable_sort(children.begin(), children.end(), [&](const window_ref& a, const window_ref& b) {
            auto rect_a = a.GetClientRect();
            auto rect_b = b.GetClientRect();
            if (rect_a && rect_b)
            {
                if (direction == StackDirection::Vertical)
                {
                    return rect_a->top < rect_b->top;
                }
                else if (direction == StackDirection::Horizontal)
                {
                    return rect_a->left < rect_b->left;
                }
            }

            return false;
        });

        auto x_pos = start_pos ? start_pos->x : 0;
        auto y_pos = start_pos ? start_pos->y : 0;

        if (direction == StackDirection::Vertical)
        {
            for (auto& child : children)
            {
                auto rect = child.GetClientRect();

                rect->right = parent_size.cx + rect->left;
                
                auto height = rect->bottom - rect->top;
                rect->top = y_pos;
                rect->bottom = rect->top + height;
                
                child.SetWindowPos(*rect);
                y_pos += height;
            }
        }
        else if (direction == StackDirection::Horizontal)
        {
            for (auto& child : children)
            {
                auto rect = child.GetClientRect();

                rect->bottom = parent_size.cy + rect->top;

                auto width = rect->right - rect->left;
                rect->left = x_pos;
                rect->right = rect->left + width;

                
                if (start_pos)
                {
                    rect->top = y_pos;
                }

                child.SetWindowPos(*rect);
                x_pos += width;
            }
        }
    }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    inline void StackChildren(const window& parent, StackDirection direction = StackDirection::Vertical)
    {
        std::vector<window_ref> children;
        children.reserve(8);

        if (auto child = parent.GetChild(); child)
        {
          for (auto i = child->GetFirstSibling(); i != std::nullopt; i = i->GetNextSibling())
          {
              if (i)
              {
                children.emplace_back(std::move(*i));              
              }
          }
        }

        auto rect = parent.GetClientRect();
        return StackChildren(SIZE {.cx = rect->left, .cy = rect->bottom }, children, direction);
    }
#endif
}


#endif // !WIN32_WINDOW_HPP
