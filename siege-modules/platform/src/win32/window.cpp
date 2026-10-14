module;
#include <siege/platform/windows.hpp>
#include <oleacc.h>
#include <oleauto.h>
#include <cassert>

export module win32.window;

import std;
export import win32.utils;
export import win32.module;
export import win32.drawing;

export namespace win32
{
  using ::HWND;
  using ::HMENU;
  using ::WPARAM;
  using ::LPARAM;
  using ::LRESULT;
  using ::ATOM;
  using ::MSG;
  using ::CREATESTRUCTW;
  using ::WNDCLASSEXW;
  using ::COPYDATASTRUCT;
  using ::HRAWINPUT;

  export constexpr WORD loword(LPARAM value) noexcept
  {
    return LOWORD(value);
  }

  export constexpr WORD hiword(LPARAM value) noexcept
  {
    return HIWORD(value);
  }

  export constexpr int get_x_lparam(LPARAM value) noexcept
  {
    return GET_X_LPARAM(value);
  }

  export constexpr int get_y_lparam(LPARAM value) noexcept
  {
    return GET_Y_LPARAM(value);
  }

  export BOOL unregister_class(ATOM atom, HINSTANCE instance) noexcept
  {
    return ::UnregisterClassW(reinterpret_cast<LPCWSTR>(static_cast<ULONG_PTR>(atom)), instance);
  }

  export struct menu_no_deleter
  {
    void operator()(HMENU menu)
    {
    }
  };

  export struct menu_deleter
  {
    void operator()(HMENU menu)
    {
      ::DestroyMenu(menu);
    }
  };

  export template<typename TDeleter>
  struct menu_base : win32::auto_handle<HMENU, TDeleter>
  {
    using base = win32::auto_handle<HMENU, TDeleter>;
    using base::base;

    template<typename TMenu, typename TPopupMenu>
    struct notifications
    {
      virtual std::optional<LRESULT> wm_command(TMenu, int)
      {
        return std::nullopt;
      }

      virtual std::optional<LRESULT> wm_menu_command(TMenu, int)
      {
        return std::nullopt;
      }

      virtual std::optional<LRESULT> wm_enter_menu_loop(bool)
      {
        return std::nullopt;
      }

      virtual std::optional<LRESULT> wm_exit_menu_loop(bool)
      {
        return std::nullopt;
      }

      virtual std::optional<LRESULT> wm_init_menu_popup(TPopupMenu, int, bool)
      {
        return std::nullopt;
      }

      virtual std::optional<LRESULT> wm_uninit_menu_popup(TPopupMenu, int)
      {
        return std::nullopt;
      }

      virtual std::optional<LRESULT> wm_draw_item(TPopupMenu, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }

      virtual SIZE wm_measure_item(TPopupMenu, const MEASUREITEMSTRUCT&)
      {
        return SIZE{};
      }

      virtual std::optional<LRESULT> wm_draw_item(TMenu, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }

      virtual SIZE wm_measure_item(TMenu, const MEASUREITEMSTRUCT&)
      {
        return SIZE{};
      }

      template<typename TWindow>
      static std::optional<LRESULT> dispatch_message(TWindow* window, HWND owner, std::uint32_t message, WPARAM wParam, LPARAM lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
          auto* self = static_cast<notifications*>(window);

          if (message == WM_COMMAND && win32::hiword(wParam) == 0)
          {
            return self->wm_command(TMenu(::GetMenu(owner)), win32::loword(wParam));
          }

          if (message == WM_MENUCOMMAND)
          {
            return self->wm_menu_command(TMenu((HMENU)lParam), wParam);
          }

          if (message == WM_ENTERMENULOOP)
          {
            return self->wm_enter_menu_loop(wParam == TRUE);
          }

          if (message == WM_EXITMENULOOP)
          {
            return self->wm_exit_menu_loop(wParam == TRUE);
          }

          thread_local bool measuring_popup = false;

          if (message == WM_INITMENUPOPUP)
          {
            measuring_popup = true;
            return self->wm_init_menu_popup(TPopupMenu((HMENU)wParam), win32::loword(lParam), win32::hiword(lParam) == TRUE);
          }

          if (message == WM_UNINITMENUPOPUP)
          {
            measuring_popup = false;
            return self->wm_uninit_menu_popup(TPopupMenu((HMENU)wParam), win32::hiword(lParam));
          }

          if (message == WM_MEASUREITEM)
          {
            auto& context = *(MEASUREITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_MENU)
            {
              SIZE size;

              if (measuring_popup)
              {
                size = self->wm_measure_item(TPopupMenu(nullptr), context);
              }
              else
              {
                size = self->wm_measure_item(TMenu(::GetMenu(owner)), context);
              }

              context.itemWidth = size.cx;
              context.itemHeight = size.cy;
              return 0;
            }
          }

          if (message == WM_DRAWITEM)
          {
            auto& context = *(DRAWITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_MENU)
            {
              if (::GetWindowLongPtrW(owner, GWL_STYLE) & ~WS_CHILD)
              {
                auto menu = ::GetMenu(owner);

                if (menu == (HMENU)context.hwndItem)
                {
                  return self->wm_draw_item(TMenu((HMENU)context.hwndItem), context);
                }
              }
              return self->wm_draw_item(TPopupMenu((HMENU)context.hwndItem), context);
            }
          }
        }

        return std::nullopt;
      }
    };

    struct co_task_deleter
    {
      void operator()(void* data)
      {
        ::CoTaskMemFree(data);
      }
    };

    struct menu_string_info : MSAAMENUINFO
    {
      menu_string_info(std::wstring_view data) : MSAAMENUINFO{ .dwMSAASignature = MSAA_MENU_SIG, .cchWText = data.size(), .pszWText = ::SysAllocString(data.data()) }
      {
      }

      ~menu_string_info()
      {
        if (pszWText)
        {
          ::SysFreeString(pszWText);
        }
      }
    };

    auto AppendMenuW(UINT flags, UINT_PTR id, std::wstring_view data = L"")
    {
      if (flags & MF_OWNERDRAW && !data.empty())
      {
        static std::map<std::wstring_view, std::unique_ptr<menu_string_info, co_task_deleter>> menu_items;
        
        auto iter = menu_items.find(data);

        if (iter == menu_items.end())
        {
          auto* storage = ::CoTaskMemAlloc(sizeof(menu_string_info));
          auto* acc_info = new (storage) menu_string_info(data);

          iter = menu_items.emplace(acc_info->pszWText, std::unique_ptr<menu_string_info, co_task_deleter>(acc_info)).first;
        }

        MENUITEMINFOW info{
          .cbSize = sizeof(MENUITEMINFOW),
          .fMask = MIIM_TYPE | MIIM_DATA
        };

        info.fType = flags;
        info.dwItemData = (ULONG_PTR)iter->second.get();

        BOOL by_position = FALSE;
        if (flags & MF_POPUP)
        {
          info.fType = info.fType & ~MF_POPUP;
          info.fType = info.fType & MFT_OWNERDRAW;

          info.fMask |= MIIM_SUBMENU;
          info.hSubMenu = (HMENU)id;
          by_position = TRUE;
        }
        else
        {
          info.fMask |= MIIM_ID;
          info.wID = id;
        }

        return this->InsertMenuItemW(id, by_position, info);
      }

      return ::AppendMenuW(*this, flags, id, data.data());
    }

    auto InsertMenuItemW(UINT item, BOOL by_position, MENUITEMINFOW info)
    {
      return ::InsertMenuItemW(*this, item, by_position, &info);
    }
  };

  export struct menu_ref : menu_base<menu_no_deleter>
  {
    using base = menu_base<menu_no_deleter>;
    using base::base;
    //   using notifications = base::notifications<menu_ref, popup_menu_ref>;
  };

  export template<typename TDeleter>
  struct popup_menu_base : menu_base<TDeleter>
  {
    using base = menu_base<TDeleter>;
    using base::base;

    auto TrackPopupMenuEx(UINT flags, POINT coords, HWND owner, std::optional<TPMPARAMS> params = std::nullopt)
    {
      if (params)
      {
        params.value().cbSize = sizeof(TPMPARAMS);
        return ::TrackPopupMenuEx(*this, flags, coords.x, coords.y, owner, &params.value());
      }
      else
      {
        return ::TrackPopupMenuEx(*this, flags, coords.x, coords.y, owner, nullptr);
      }
    }
  };

  export struct popup_menu_ref : popup_menu_base<menu_no_deleter>
  {
    using base = popup_menu_base<menu_no_deleter>;
    using base::base;
  };

  export struct menu : menu_base<menu_deleter>
  {
    using base = menu_base<menu_deleter>;
    using notifications = base::notifications<menu_ref, popup_menu_ref>;

    menu()
    {
      this->reset(::CreateMenu());
    }
  };

  export struct popup_menu : popup_menu_base<menu_deleter>
  {
    using base = popup_menu_base<menu_deleter>;
    using base::base;
    using notifications = base::notifications<menu_ref, popup_menu_ref>;

    popup_menu()
    {
      this->reset(::CreatePopupMenu());
    }
  };

  static_assert(sizeof(void*) == sizeof(LONG_PTR));
  static_assert(sizeof(std::uint32_t) == sizeof(UINT));

  export struct message
  {
    std::uint32_t message;
    WPARAM wParam;
    LPARAM lParam;
  };

  export struct size_message
  {
    constexpr static std::uint32_t id = WM_SIZE;

    int type;
    SIZE client_size;

    size_message(WPARAM wParam, LPARAM lParam) : type(static_cast<int>(wParam)),
                                                 client_size(SIZE{ loword(lParam), hiword(lParam) })
    {
    }
  };

  export struct setting_change_message
  {
    constexpr static std::uint32_t id = WM_SETTINGCHANGE;

    WPARAM wparam;

    std::wstring setting;

    setting_change_message(WPARAM wParam, LPARAM lParam) : wparam(wParam),
                                                           setting(lParam != 0 ? (wchar_t*)lParam : L"")
    {
    }
  };

  export struct input_message
  {
    constexpr static std::uint32_t id = WM_INPUT;
    WPARAM code;
    HRAWINPUT handle;

    input_message(WPARAM code, LPARAM item) : code(GET_RAWINPUT_CODE_WPARAM(code)), handle(reinterpret_cast<HRAWINPUT>(item))
    {
    }
  };

  export struct input_device_change_message
  {
    constexpr static std::uint32_t id = WM_INPUT_DEVICE_CHANGE;
    WPARAM code;
    HANDLE device_handle;

    input_device_change_message(WPARAM code, LPARAM handle) : code(code), device_handle(reinterpret_cast<HANDLE>(handle))
    {
    }
  };

  export struct get_object_message
  {
    constexpr static std::uint32_t id = WM_GETOBJECT;

    std::uint32_t flags;
    std::uint32_t object_id;

    get_object_message(WPARAM flags, LPARAM objectId) : flags(std::uint32_t(flags)), object_id(std::uint32_t(objectId))
    {
    }
  };

  export template<typename TChar = std::byte>
  struct copy_data_message
  {
    constexpr static std::uint32_t id = WM_COPYDATA;
    HWND sender;
    std::size_t data_type;
    std::span<TChar> data;

    copy_data_message(WPARAM wParam, LPARAM lParam) : sender(HWND(wParam)), data_type(0), data()
    {
      COPYDATASTRUCT* raw_data = std::bit_cast<COPYDATASTRUCT*>(lParam);
      if (raw_data)
      {
        data_type = raw_data->dwData;
        if (raw_data->cbData == 0 || raw_data->lpData == nullptr)
        {
          return;
        }

        data = std::span<TChar>(std::bit_cast<TChar*>(raw_data->lpData), raw_data->cbData);
      }
    }
  };

  export enum struct window_style : DWORD
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

  export enum struct extended_window_style : DWORD
  {

  };

  export struct window_deleter
  {
    void operator()(HWND window)
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
      if (::GetParent(window) == nullptr)
#endif
      {
        ::DestroyWindow(window);
      }
    }
  };

  export struct window_no_deleter
  {
    void operator()(HWND window)
    {
    }
  };

  export struct window_ref;

  export template<typename TDeleter, typename window_ref>
  struct window_base : win32::auto_handle<HWND, TDeleter>
  {
    using base = win32::auto_handle<HWND, TDeleter>;
    using base::base;

    operator HWND() const
    {
      return this->get();
    }

    window_ref ref() const
    {
      return window_ref(*this);
    }

    std::wstring_view RealGetWindowClassW()
    {
      thread_local std::array<wchar_t, 256> name{};
      name.fill(L'\0');
      ::RealGetWindowClassW(*this, name.data(), name.size());
      return name.data();
    }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

    std::optional<window_ref> GetAncestor(std::uint32_t flags) const
    {
      auto parent = ::GetAncestor(*this, flags);

      if (!parent)
      {
        return std::nullopt;
      }

      return window_ref(parent);
    }

    std::optional<window_ref> GetParent() const
    {
      auto parent = ::GetParent(*this);

      if (!parent)
      {
        return std::nullopt;
      }

      return window_ref(parent);
    }

    std::optional<window_ref> GetWindow(std::uint32_t type) const
    {
      auto parent = ::GetWindow(*this, type);

      if (!parent)
      {
        return std::nullopt;
      }

      return window_ref(parent);
    }

    std::optional<window_ref> GetChild() const
    {
      return GetWindow(GW_CHILD);
    }

    std::optional<window_ref> GetFirstSibling() const
    {
      return GetWindow(GW_HWNDFIRST);
    }

    std::optional<window_ref> GetNextSibling() const
    {
      return GetWindow(GW_HWNDNEXT);
    }

    std::optional<RECT> GetWindowRect() const
    {
      RECT result;
      if (::GetWindowRect(*this, &result))
      {
        return result;
      }

      return std::nullopt;
    }

    std::optional<RECT> MapDialogRect(RECT raw) const
    {
      if (::MapDialogRect(*this, &raw))
      {
        return raw;
      }

      return std::nullopt;
    }

    bool CopyData(HWND sender, ::COPYDATASTRUCT data)
    {
      return ::SendMessageW(*this, WM_COPYDATA, WPARAM(HWND(sender)), LPARAM(&data));
    }

    auto GetWindowStyle() const
    {
      return ::GetWindowLongPtrW(*this, GWL_STYLE);
    }

    auto SetWindowStyle(LONG_PTR style)
    {
      return ::SetWindowLongPtrW(*this, GWL_STYLE, style);
    }

    template<typename TValue>
    TValue GetPropW(std::wstring_view name) const
    {
      if constexpr (std::is_integral_v<TValue> || std::is_enum_v<TValue> || std::is_pointer_v<TValue>)
      {
        auto raw = std::size_t(::GetPropW(*this, name.data()));
        return TValue(raw);
      }

      return TValue();
    }

    template<typename TValue>
    auto SetPropW(std::wstring_view name, TValue value)
    {
      if constexpr (std::is_integral_v<TValue> || std::is_enum_v<TValue> || std::is_pointer_v<TValue>)
      {
        return ::SetPropW(*this, name.data(), HANDLE(value));
      }

      return ::SetPropW(*this, name.data(), nullptr);
    }

    template<typename TValue = HANDLE>
    [[maybe_unused]] TValue RemovePropW(std::wstring_view name)
    {
      auto value = std::size_t(::RemovePropW(*this, name.data()));

      return TValue(value);
    }

#endif

    std::optional<RECT> GetClientRect() const
    {
      RECT result;
      if (::GetClientRect(*this, &result))
      {
        return result;
      }

      return std::nullopt;
    }

    std::optional<std::pair<POINT, RECT>> MapWindowPoints(HWND to, RECT source)
    {
      ::SetLastError(0);
      auto result = ::MapWindowPoints(*this, to, reinterpret_cast<POINT*>(&source), sizeof(RECT) / sizeof(POINT));

      if (result || (result == 0 && ::GetLastError() == 0))
      {
        return std::make_pair(POINT{ win32::loword(result), win32::hiword(result) }, source);
      }

      return std::nullopt;
    }

    std::optional<SIZE> GetClientSize() const
    {
      auto rect = GetClientRect();

      if (rect)
      {
        return SIZE{ .cx = rect->right - rect->left, .cy = rect->bottom - rect->top };
      }

      return std::nullopt;
    }

    std::optional<std::pair<POINT, SIZE>> GetClientPositionAndSize() const
    {
      RECT result;
      if (::GetClientRect(*this, &result))
      {
        return std::make_pair(POINT{ .x = result.left, .y = result.top }, SIZE{ .cx = result.right - result.left, .cy = result.bottom - result.top });
      }

      return std::nullopt;
    }

    HANDLE DeferWindowPos(HANDLE hWinPosInfo, POINT position, UINT uFlags = 0)
    {
      return DeferWindowPos(hWinPosInfo, *this, nullptr, position.x, position.y, 0, 0, uFlags | SWP_NOSIZE | SWP_NOZORDER);
    }

    HANDLE DeferWindowPos(HANDLE hWinPosInfo, SIZE size, UINT uFlags = 0)
    {
      return DeferWindowPos(hWinPosInfo, *this, nullptr, 0, 0, size.cx, size.cy, uFlags | SWP_NOMOVE | SWP_NOZORDER);
    }

    HANDLE DeferWindowPos(HANDLE hWinPosInfo, HWND hWndInsertAfter, UINT uFlags = 0)
    {
      return DeferWindowPos(hWinPosInfo, *this, hWndInsertAfter, 0, 0, 0, 0, uFlags | SWP_NOMOVE | SWP_NOSIZE);
    }

    auto SetWindowPos(HWND hWndInsertAfter, UINT uFlags = 0)
    {
      return ::SetWindowPos(*this, hWndInsertAfter, 0, 0, 0, 0, uFlags | SWP_NOMOVE | SWP_NOSIZE);
    }

    auto SetWindowPos(RECT position_and_size, UINT uFlags = 0)
    {
      return ::SetWindowPos(*this, nullptr, position_and_size.left, position_and_size.top, position_and_size.right - position_and_size.left, position_and_size.bottom - position_and_size.top, uFlags | SWP_NOZORDER);
    }

    auto SetWindowPos(POINT position, SIZE size, UINT uFlags = 0)
    {
      return ::SetWindowPos(*this, nullptr, position.x, position.y, size.cx, size.cy, uFlags | SWP_NOZORDER);
    }

    auto SetWindowPos(POINT position, UINT uFlags = 0)
    {
      return ::SetWindowPos(*this, nullptr, position.x, position.y, 0, 0, uFlags | SWP_NOSIZE | SWP_NOZORDER);
    }

    auto SetWindowPos(SIZE size, UINT uFlags = 0)
    {
      return ::SetWindowPos(*this, nullptr, 0, 0, size.cx, size.cy, uFlags | SWP_NOMOVE | SWP_NOZORDER);
    }

    auto MoveWindow(RECT position_and_size, bool repaint)
    {
      return ::MoveWindow(*this, position_and_size.left, position_and_size.top, position_and_size.right - position_and_size.left, position_and_size.bottom - position_and_size.top, repaint ? TRUE : FALSE);
    }

    auto MoveWindow(POINT position, SIZE size, bool repaint)
    {
      return ::MoveWindow(*this, position.x, position.y, size.cx, size.cy, repaint ? TRUE : FALSE);
    }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    [[maybe_unused]] auto EnumPropsExW(std::move_only_function<bool(HWND, std::wstring_view, HANDLE)> callback) const
    {
      struct Handler
      {
        static BOOL CALLBACK HandleEnum(HWND self, LPWSTR key, HANDLE data, ULONG_PTR raw_callback)
        {
          if (key == nullptr)
          {
            return TRUE;
          }

          auto real_callback = std::bit_cast<std::move_only_function<bool(HWND, std::wstring_view, HANDLE)>*>(raw_callback);

          std::wstring_view keyView;

          auto intAtom = (ATOM)(std::size_t)key;

          if (intAtom > 0 && intAtom < MAXINTATOM)
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

      return EnumPropsExW(*this, Handler::HandleEnum, std::bit_cast<LPARAM>(&callback));
    }

    [[maybe_unused]] auto ForEachPropertyExW(std::move_only_function<void(HWND, std::wstring_view, HANDLE)> callback) const
    {
      return EnumPropsExW([callback = std::move(callback)](auto self, auto key, auto value) mutable {
        callback(self, key, value);
        return true;
      });
    }

    template<typename TResult = HANDLE>
    [[maybe_unused]] std::expected<TResult, int> FindPropertyExW(std::move_only_function<bool(HWND, std::wstring_view, HANDLE)> callback) const
    {
      std::optional<TResult> result = std::nullopt;

      auto enum_result = EnumPropsExW([&result, callback = std::move(callback)](auto self, auto key, auto value) mutable {
        if (callback(self, key, value))
        {
          result.emplace((TResult)value);
          return false;
        }
        return true;
      });

      if (enum_result == -1)
      {
        return std::unexpected(enum_result);
      }

      if (!result)
      {
        return std::unexpected(0);
      }

      return *result;
    }

    template<typename TResult = HANDLE>
    [[maybe_unused]] std::optional<TResult> FindPropertyExW(std::wstring_view find_key) const
    {
      auto result = FindPropertyExW<TResult>([find_key](auto, auto key, auto) {
        return key == find_key;
      });

      if (!result && result.error() == -1)
      {
        auto prop = GetPropW<TResult>(find_key);

        if (prop)
        {
          return prop;
        }
        else
        {
          return std::nullopt;
        }
      }

      if (!result)
      {
        return std::nullopt;
      }

      return *result;
    }
#endif
  };

  export struct window_ref : window_base<window_no_deleter, window_ref>
  {
    using base = window_base<window_no_deleter, window_ref>;
    using base::base;
  };

  export struct window : window_base<window_deleter, window_ref>
  {
    using base = window_base<window_deleter, window_ref>;
    using base::base;
  };

  export std::function<bool()> SetTimer(win32::window_ref window, std::uint32_t delay, std::move_only_function<void(win32::window_ref, std::uint32_t, std::function<bool()>, std::uint32_t)> callback)
  {
    auto* callback_ptr = new decltype(callback)(std::move(callback));

    struct callback_struct
    {
      static VOID CALLBACK timer_callback(
        HWND hwnd,
        UINT message,
        UINT_PTR idTimer,
        DWORD dwTime)
      {
        auto* raw_callback = (std::add_pointer_t<decltype(callback)>)idTimer;

        if (raw_callback)
        {
          (*raw_callback)(
            win32::window_ref(hwnd), message, [idTimer, hwnd, raw_callback] {
              if (::KillTimer(hwnd, idTimer))
              {
                delete raw_callback;
              }

              return false;
            },
            dwTime);
        }
      }
    };

    auto result = ::SetTimer(window, (UINT_PTR)callback_ptr, delay, callback_struct::timer_callback);

    return [result, window = window.get(), callback_ptr] {
      if (::KillTimer(window, result))
      {
        delete callback_ptr;
      }

      return false;
    };
  }

  export class local_atom
  {
  public:
    local_atom(std::wstring_view string) : value(::AddAtomW(string.data()))
    {
    }

    local_atom(std::string_view string) : value(::AddAtomA(string.data()))
    {
    }

    ~local_atom()
    {
      ::DeleteAtom(value);
    }

    operator ATOM()
    {
      return value;
    }

  private:
    ATOM value;
  };

  export struct default_window : win32::window_ref
  {
    default_window(HWND handle, CREATESTRUCTW& params) : win32::window_ref(handle)
    {
    }

    virtual std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam)
    {
      return std::nullopt;
    }

    virtual ~default_window() = default;
  };

  export template<typename TWindow>
  struct basic_window : default_window
  {
    using default_window::default_window;

    static LRESULT __stdcall window_proc(HWND self, UINT message, WPARAM wparam, LPARAM lparam)
    {
      if (message == WM_NCCREATE)
      {
        try
        {
          default_window* temp = new TWindow(self, *(CREATESTRUCTW*)lparam);
          ::SetWindowLongPtrW(self, 0, (LONG_PTR)temp);

          return ::DefWindowProcW(self, message, wparam, lparam);
        }
        catch (...)
        {
          return FALSE;
        }
      }

      auto* window = (default_window*)::GetWindowLongPtrW(self, 0);

      if (!window)
      {
        return ::DefWindowProcW(self, message, wparam, lparam);
      }

      if (message == WM_NCDESTROY && *window == self)
      {
        auto result = ::DefWindowProcW(self, message, wparam, lparam);
        delete window;
        ::SetWindowLongPtrW(self, 0, 0);
        return result;
      }

      if (*window == self)
      {
        try
        {
          auto result = window->window_proc(message, wparam, lparam);

          if (result)
          {
            return *result;
          }
        }
        catch (const std::exception& ex)
        {
          ::OutputDebugStringA(ex.what());
          ::OutputDebugStringA("\n");
          assert(false);
        }
        catch (...)
        {
          assert(false);
        }
      }

      return ::DefWindowProcW(self, message, wparam, lparam);
    }
  };

  export auto widen(std::string_view data)
  {
    return std::wstring(data.begin(), data.end());
  }

  export template<typename TType>
  auto& type_name()
  {
    static auto name = widen(typeid(TType).name());
    return name;
  }

  export template<typename TType>
  std::wstring window_class_name()
  {
    if constexpr (requires(TType t) { TType::class_name; })
    {
      return TType::class_name;
    }

    return L"";
  }

  export template<typename TModule = win32::module>
  struct window_module_base : TModule
  {
    using TModule::TModule;

    auto RegisterClassExW(::WNDCLASSEXW meta)
    {
      meta.cbSize = sizeof(::WNDCLASSEXW);
      meta.hInstance = *this;
      return ::RegisterClassExW(&meta);
    }

    template<typename TClass>
    auto UnregisterClassW()
    {
      return ::UnregisterClassW(type_name<TClass>().c_str(), this->get());
    }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    std::optional<WNDCLASSEXW> GetClassInfoExW(std::wstring_view name)
    {
      WNDCLASSEXW temp{ .cbSize = sizeof(WNDCLASSEXW) };

      if (::GetClassInfoExW(*this, name.data(), &temp))
        ;
      {
        if (!temp.lpfnWndProc)
        {
          return std::nullopt;
        }

        try
        {
          temp.hInstance = win32::module_ref(temp.lpfnWndProc);
        }
        catch (...)
        {
        }

        return temp;
      }

      return std::nullopt;
    }
#endif

    template<typename TControl = window>
    std::expected<TControl, DWORD> CreateWindowExW(CREATESTRUCTW params)
    {
      std::wstring class_name;

      if (!params.lpszClass)
      {
        class_name = window_class_name<TControl>();
        params.lpszClass = class_name.c_str();
      }

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
        *this,
        params.lpCreateParams);

      if (!result)
      {
        return std::unexpected(::GetLastError());
      }

      return TControl(result);
    }
  };

  export template<typename TControl = window>
  std::expected<TControl, DWORD> CreateWindowExW(CREATESTRUCTW params)
  {
    std::wstring class_name;

    if constexpr (!std::is_same_v<TControl, window>)
    {
      if (!params.lpszClass || params.lpszClass[0] == L'\0')
      {
        class_name = window_class_name<TControl>();
        params.lpszClass = class_name.c_str();
      }
    }

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
      params.hInstance,
      params.lpCreateParams);

    if (!result)
    {
      return std::unexpected(::GetLastError());
    }

    return TControl(result);
  }

  export using window_module = window_module_base<module>;

  export struct window_module_ref : window_module_base<module_ref>
  {
    using window_module_base<module_ref>::window_module_base;

    static window_module_ref current_module()
    {
      return window_module_ref(module_ref::current_module().get());
    }
  };
}// namespace win32
