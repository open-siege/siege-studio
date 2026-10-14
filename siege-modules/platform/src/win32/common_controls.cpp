module;
#include <siege/platform/windows.hpp>
#include <CommCtrl.h>

export module win32.common_controls;

import std;
import win32.drawing;

export import win32.user_controls;
namespace win32
{
  export BOOL init_common_controls_ex(INITCOMMONCONTROLSEX info)
  {
    static auto module = [] {
      auto result = ::GetModuleHandleW(L"comctl32.dll");

      if (!result)
      {
        result = ::LoadLibraryExW(L"comctl32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
      }

      return result;
    }();

    using init_common_controls_ex = std::add_pointer_t<decltype(::InitCommonControlsEx)>;
    static auto proc = (init_common_controls_ex)::GetProcAddress(module, "InitCommonControlsEx");

    if (proc)
    {
      return proc(&info);
    }

    using init_common_controls = std::add_pointer_t<decltype(::InitCommonControls)>;
    static auto proc2 = (init_common_controls)::GetProcAddress(module, "InitCommonControls");

    if (proc2)
    {
      proc2();
      return TRUE;
    }

    throw new std::runtime_error("comctl32 InitCommonControlsEx is not available");
  }

  export HIMAGELIST image_list_create(
    int cx,
    int cy,
    UINT flags,
    int cInitial,
    int cGrow)
  {
    static auto module = ::GetModuleHandleW(L"comctl32.dll");

    using get_icon_size_func = std::add_pointer_t<decltype(::ImageList_Create)>;
    static auto proc = (get_icon_size_func)::GetProcAddress(module, "ImageList_Create");

    if (proc)
    {
      return proc(cx, cy, flags, cInitial, cGrow);
    }

    throw new std::runtime_error("comctl32 ImageList_Create is not available");
  }

  export BOOL image_list_get_icon_size(HIMAGELIST himl, int* cx, int* cy)
  {
    static auto module = ::GetModuleHandleW(L"comctl32.dll");

    using get_icon_size_func = std::add_pointer_t<decltype(::ImageList_GetIconSize)>;
    static auto proc = (get_icon_size_func)::GetProcAddress(module, "ImageList_GetIconSize");

    if (proc)
    {
      return proc(himl, cx, cy);
    }

    throw new std::runtime_error("comctl32 ImageList_GetIconSize is not available");
  }

  export BOOL set_window_command_subclass(HWND window, WORD source_id, std::move_only_function<std::optional<LRESULT>(win32::window_ref window, WPARAM wparam, LPARAM lparam)> callback)
  {
    if (!callback)
    {
      return FALSE;
    }

    struct function_context
    {
      WORD source_id;
      std::move_only_function<std::optional<LRESULT>(win32::window_ref window, WPARAM wparam, LPARAM lparam)> callback;
    };

    struct handler
    {
      static LRESULT __stdcall sub_class_proc(
        HWND hwnd,
        UINT message,
        WPARAM wparam,
        LPARAM lparam,
        UINT_PTR id,
        DWORD_PTR data)
      {
        if (id && message == WM_COMMAND)
        {
          auto* context = (function_context*)id;

          if (context->source_id == win32::loword(wparam))
          {
            auto result = context->callback(win32::window_ref(hwnd), wparam, lparam);

            if (result)
            {
              return *result;
            }
          }
        }

        if (message == WM_NCDESTROY)
        {
          auto* context = (function_context*)id;
          delete context;
          remove_window_subclass(hwnd, sub_class_proc, id);
        }

        return def_subclass_proc(hwnd, message, wparam, lparam);
      }
    };

    return set_window_subclass(window, handler::sub_class_proc, (UINT_PTR) new function_context{ source_id, std::move(callback) }, 0);
  }

  export struct image_list_deleter
  {
    void operator()(HIMAGELIST list)
    {
      auto result = ::ImageList_Destroy(list);
      // assert(result == TRUE);
    }
  };

  export struct image_list : win32::auto_handle<HIMAGELIST, image_list_deleter>
  {
    using base = win32::auto_handle<HIMAGELIST, image_list_deleter>;
    using base::base;

    image_list() : base()
    {
    }

    struct params
    {
      int width;
      int height;
      std::uint32_t flags = ILC_COLOR32;
      int capacity = 0;
      int size_limit = 0;
    };

    image_list(int width, int height, std::uint32_t flags = ILC_COLOR32, int capacity = 0, int size_limit = 0)
    {
      auto result = image_list_create(width, height, flags, capacity, size_limit);

      if (result == nullptr)
      {
        throw std::runtime_error("Could not create image list");
      }

      reset(result);
    }

    image_list(params params) : image_list(params.width, params.height, params.flags, params.capacity, params.size_limit)
    {
    }

    std::optional<SIZE> GetIconSize()
    {
      if (!get())
      {
        return std::nullopt;
      }

      int width;
      int height;

      if (image_list_get_icon_size(get(), &width, &height))
      {
        return SIZE{ .cx = width, .cy = height };
      }

      return std::nullopt;
    }

    void** put_void()
    {
      static_assert(sizeof(base) == sizeof(void*));
      static_assert(std::is_standard_layout_v<image_list>);
      return reinterpret_cast<void**>(this);
    }
  };

  export struct task_dialog_result
  {
    std::vector<int> buttons;
    std::vector<int> radio_buttons;
    BOOL verfication_flag_checked = FALSE;
  };

  export std::expected<task_dialog_result, HRESULT> task_dialog_indirect(TASKDIALOGCONFIG config, std::move_only_function<HRESULT(HWND, UINT, WPARAM, LPARAM)> callback)
  {
    static auto module = ::GetModuleHandleW(L"comctl32.dll");

    using task_dialog_indirect_func = std::add_pointer_t<decltype(::TaskDialogIndirect)>;

    static auto proc = (task_dialog_indirect_func)::GetProcAddress(module, "TaskDialogIndirect");

    if (proc)
    {
      task_dialog_result result;

      result.buttons.resize(config.cButtons);
      result.radio_buttons.resize(config.cRadioButtons);

      struct handler
      {
        static HRESULT __stdcall dialog_callback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
        {
          if (lpRefData)
          {
            auto* callback = (std::move_only_function<HRESULT(HWND, UINT, WPARAM, LPARAM)>*)lpRefData;

            auto result = callback->operator()(hwnd, msg, wParam, lParam);

            if (msg == TDN_DIALOG_CONSTRUCTED)
            {
              struct sub_class
              {
                static HRESULT __stdcall sub_class_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
                {
                  if (message == TDM_NAVIGATE_PAGE && lparam)
                  {
                    auto* config = (TASKDIALOGCONFIG*)lparam;
                    if (!config->pfCallback)
                    {
                      config->pfCallback = handler::dialog_callback;
                      config->lpCallbackData = (LONG_PTR)uIdSubclass;
                    }
                  }

                  if (message == WM_DESTROY)
                  {
                    remove_window_subclass(window, sub_class::sub_class_callback, uIdSubclass);
                  }

                  return def_subclass_proc(window, message, wparam, lparam);
                }
              };
              set_window_subclass(hwnd, sub_class::sub_class_callback, (UINT_PTR)lpRefData, 0);
            }

            if (msg == TDN_DESTROYED)
            {
              delete callback;
            }
            return result;
          }

          return S_FALSE;
        }
      };

      config.pfCallback = handler::dialog_callback;
      config.lpCallbackData = (LONG_PTR) new std::move_only_function<HRESULT(HWND, UINT, WPARAM, LPARAM)>(std::move(callback));

      auto hresult = proc(&config,
        result.buttons.empty() ? nullptr : result.buttons.data(),
        result.radio_buttons.empty() ? nullptr : result.radio_buttons.data(),
        &result.verfication_flag_checked);

      if (hresult != S_OK)
      {
        return std::unexpected(hresult);
      }

      return result;
    }

    throw new std::runtime_error("comctl32 TaskDialogIndirect is not available");
  }

  export struct control : window
  {
    using window::window;

    template<typename TControl, typename TNotification, typename TReturn = void>
    [[maybe_unused]] std::function<void()> bind_notification(UINT code, std::move_only_function<TReturn(TControl, const TNotification&)> callback)
    {
      return win32::bind_notification<TControl, TNotification, TReturn>(this->GetParent()->ref(), this->ref(), code, std::move(callback));
    }
  };

  export struct ip_address_edit : control
  {
    using control::control;
    using control::bind_notification;
    constexpr static auto class_name = WC_IPADDRESSW;

    [[maybe_unused]] std::function<void()> bind_en_change(std::move_only_function<void(ip_address_edit, const NMHDR&)> callback)
    {
      return bind_notification<ip_address_edit, NMHDR>(EN_CHANGE, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_en_kill_focus(std::move_only_function<void(ip_address_edit, const NMHDR&)> callback)
    {
      return bind_notification<ip_address_edit, NMHDR>(EN_KILLFOCUS, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_en_set_focus(std::move_only_function<void(ip_address_edit, const NMHDR&)> callback)
    {
      return bind_notification<ip_address_edit, NMHDR>(EN_SETFOCUS, std::move(callback));
    }
  };

  export struct sys_link : control
  {
    using control::control;
    constexpr static auto class_name = WC_LINK;
  };

  export struct track_bar : control
  {
    using control::control;
    constexpr static auto class_name = TRACKBAR_CLASSW;
  };

  export struct header : control
  {
    using control::control;
    using control::bind_notification;
    constexpr static auto class_name = WC_HEADERW;

    [[nodiscard]] WPARAM GetItemCount()
    {
      return Header_GetItemCount(*this);
    }

    [[maybe_unused]] std::optional<HDITEMW> GetItem(WPARAM index, HDITEMW item)
    {
      if (Header_GetItem(*this, index, &item))
      {
        return item;
      }

      return std::nullopt;
    }

    [[maybe_unused]] std::optional<RECT> GetItemRect(WPARAM index)
    {
      RECT item;

      if (Header_GetItemRect(*this, index, &item))
      {
        return item;
      }

      return std::nullopt;
    }

    [[nodiscard]] auto GetBitmapMargin()
    {
      return Header_GetBitmapMargin(*this);
    }

    [[nodiscard]] auto GetFocusedItem()
    {
      return Header_GetFocusedItem(*this);
    }

    [[maybe_unused]] bool SetItem(WPARAM index, HDITEMW item)
    {
      return Header_SetItem(*this, index, &item) != 0;
    }

    [[maybe_unused]] WPARAM InsertItem(WPARAM index, HDITEMW info)
    {
      if (index == -1)
      {
        index = this->GetItemCount();
      }

      return Header_InsertItem(*this, index, &info);
    }

    [[maybe_unused]] WPARAM SetFilterChangeTimeout(int timeout = 1000)
    {
      return Header_SetFilterChangeTimeout(*this, timeout);
    }

    [[maybe_unused]] std::function<void()> bind_hdn_filter_btn_click(std::move_only_function<bool(win32::header, const NMHDFILTERBTNCLICK&)> callback)
    {
      return bind_notification<win32::header, NMHDFILTERBTNCLICK, bool>(HDN_FILTERBTNCLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_hdn_filter_change(std::move_only_function<void(win32::header, const NMHEADERW&)> callback)
    {
      return bind_notification<win32::header, NMHEADERW>(HDN_FILTERCHANGE, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_hdn_end_filter_edit(std::move_only_function<void(win32::header, const NMHEADERW&)> callback)
    {
      return bind_notification<win32::header, NMHEADERW>(HDN_ENDFILTEREDIT, std::move(callback));
    }

    struct custom_draw_callbacks
    {
      std::move_only_function<LRESULT(win32::header, NMCUSTOMDRAW&)> nm_custom_draw;
    };

    [[maybe_unused]] std::function<void()> bind_custom_draw(custom_draw_callbacks callbacks)
    {
      struct function_context
      {
        HWND source;
        custom_draw_callbacks callbacks;
      };

      struct dispatcher
      {
        static LRESULT __stdcall handle_message(
          HWND hWnd,
          UINT uMsg,
          WPARAM wParam,
          LPARAM lParam,
          UINT_PTR uIdSubclass,
          DWORD_PTR dwRefData)
        {
          if (uMsg == WM_NOTIFY && lParam && uIdSubclass)
          {
            auto* header = (NMHDR*)lParam;
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.nm_custom_draw && header->hwndFrom == context.source && header->code == NM_CUSTOMDRAW)
            {
              return context.callbacks.nm_custom_draw(win32::header(header->hwndFrom), *(NMCUSTOMDRAW*)header);
            }
          }

          if (uMsg == WM_NCDESTROY)
          {
            auto* context = (function_context*)uIdSubclass;
            delete context;
            remove_window_subclass(hWnd, dispatcher::handle_message, uIdSubclass);
          }

          return def_subclass_proc(hWnd, uMsg, wParam, lParam);
        }
      };

      auto* context = new function_context{ this->get(), std::move(callbacks) };

      set_window_subclass(this->GetParent()->get(), dispatcher::handle_message, (UINT_PTR)context, 0);

      return [context, target = this->GetParent()->get()] {
        remove_window_subclass(target, dispatcher::handle_message, (UINT_PTR)context);
        delete context;
      };
    }
  };

  export struct list_view_column : ::LVCOLUMNW
  {
  };

  export struct list_view_item : ::LVITEMW
  {
    std::wstring text;

    std::vector<std::wstring> sub_items;

    list_view_item(std::wstring text) : ::LVITEMW{}, text(std::move(text)), sub_items{}
    {
      this->pszText = this->text.data();
      this->mask = LVIF_TEXT;
    }

    list_view_item(std::wstring text, int image_index) : ::LVITEMW{ .iImage = image_index }, text(std::move(text)), sub_items{}
    {
      this->pszText = this->text.data();
      this->mask = LVIF_TEXT | LVIF_IMAGE;
    }

    list_view_item(const list_view_item& item) : ::LVITEMW(item), text(item.text), sub_items(item.sub_items)
    {
      this->pszText = text.data();
    }
  };

  export struct list_view_group : ::LVGROUP
  {
    std::wstring text;

    std::vector<list_view_item> items;

    list_view_group(std::wstring text) : ::LVGROUP{}, text(std::move(text))
    {
      this->pszHeader = this->text.data();
    }

    list_view_group(std::wstring text, std::vector<list_view_item> items) : ::LVGROUP{}, text(std::move(text)), items(std::move(items))
    {
      this->pszHeader = this->text.data();
    }

    list_view_group(const list_view_group& group) : ::LVGROUP(group), text(group.text), items(group.items)
    {
      this->pszHeader = text.data();
    }
  };

  export struct list_view : control
  {
    constexpr static auto class_name = WC_LISTVIEWW;

    using control::control;

    enum struct view_type : DWORD
    {
      details_view = LV_VIEW_DETAILS,
      icon_view = LV_VIEW_ICON,
      list_view = LV_VIEW_LIST,
      small_icon_view = LV_VIEW_LIST,
      tile_view = LV_VIEW_TILE
    };

    [[maybe_unused]] std::function<void()> bind_nm_hover(std::move_only_function<void(win32::list_view, const NMHDR&)> callback)
    {
      return bind_notification<win32::list_view, NMHDR>(NM_HOVER, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_click(std::move_only_function<void(win32::list_view, const NMITEMACTIVATE&)> callback)
    {
      return bind_notification<win32::list_view, NMITEMACTIVATE>(NM_CLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_dbl_click(std::move_only_function<void(win32::list_view, const NMITEMACTIVATE&)> callback)
    {
      return bind_notification<win32::list_view, NMITEMACTIVATE>(NM_DBLCLK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_dbl_rclick(std::move_only_function<void(win32::list_view, const NMITEMACTIVATE&)> callback)
    {
      return bind_notification<win32::list_view, NMITEMACTIVATE>(NM_RDBLCLK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_rclick(std::move_only_function<void(win32::list_view, const NMITEMACTIVATE&)> callback)
    {
      return bind_notification<win32::list_view, NMITEMACTIVATE>(NM_RCLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_lvn_item_changed(std::move_only_function<void(win32::list_view, const NMLISTVIEW&)> callback)
    {
      return bind_notification<win32::list_view, NMLISTVIEW>(LVN_ITEMCHANGED, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_lvn_end_scroll(std::move_only_function<void(win32::list_view, const NMLVSCROLL&)> callback)
    {
      return bind_notification<win32::list_view, NMLVSCROLL>(LVN_ENDSCROLL, std::move(callback));
    }

    HIMAGELIST SetImageList(WPARAM wparam, HIMAGELIST image_list)
    {
      return reinterpret_cast<HIMAGELIST>(::SendMessageW(*this, LVM_SETIMAGELIST, wparam, reinterpret_cast<LPARAM>(image_list)));
    }

    bool SetView(view_type type)
    {
      return ::SendMessageW(*this, LVM_SETVIEW, WPARAM(type), 0) == 1;
    }

    view_type GetView()
    {
      return view_type(::SendMessageW(*this, LVM_GETVIEW, 0, 0));
    }

    std::optional<bool> EnableGroupView(bool should_enable)
    {
      auto result = ::SendMessageW(*this, LVM_ENABLEGROUPVIEW, should_enable ? TRUE : FALSE, 0);

      if (result == 0)
      {
        return std::nullopt;
      }

      return result == 1;
    }

    bool IsGroupViewEnabled()
    {
      return ::SendMessageW(*this, LVM_ISGROUPVIEWENABLED, 0, 0);
    }

    LPARAM GetGroupCount()
    {
      return ::SendMessageW(*this, LVM_GETGROUPCOUNT, 0, 0);
    }

    LRESULT SetExtendedListViewStyle(WPARAM wParam, LPARAM lParam)
    {
      return ::SendMessageW(*this, LVM_SETEXTENDEDLISTVIEWSTYLE, wParam, lParam);
    }

    bool SetTileViewInfo(LVTILEVIEWINFO info)
    {
      info.cbSize = sizeof(info);

      bool mask_not_set = info.dwMask == 0;

      if (mask_not_set && (info.sizeTile.cx || info.sizeTile.cy))
      {
        info.dwMask |= LVTVIM_TILESIZE;
      }

      if (mask_not_set && info.cLines)
      {
        info.dwMask |= LVTVIM_COLUMNS;
      }

      if (mask_not_set && (info.rcLabelMargin.left || info.rcLabelMargin.right || info.rcLabelMargin.top || info.rcLabelMargin.bottom))
      {
        info.dwMask |= LVTVIM_LABELMARGIN;
      }

      return ::SendMessageW(*this, LVM_SETTILEVIEWINFO, 0, reinterpret_cast<LPARAM>(&info));
    }

    WPARAM InsertGroup(WPARAM index, LVGROUP group)
    {
      group.cbSize = sizeof(group);

      bool mask_not_set = group.mask == 0;

      if (mask_not_set && group.pszHeader)
      {
        group.mask |= LVGF_HEADER;
      }

      if (mask_not_set && group.pszFooter)
      {
        group.mask |= LVGF_FOOTER;
      }

      if (mask_not_set && group.state)
      {
        group.mask |= LVGF_STATE;
      }

      if (mask_not_set && group.uAlign)
      {
        group.mask |= LVGF_ALIGN;
      }

      if (mask_not_set && group.iGroupId)
      {
        group.mask |= LVGF_GROUPID;
      }

      if (mask_not_set && group.pszSubtitle)
      {
        group.mask |= LVGF_SUBTITLE;
      }

      if (mask_not_set && group.pszTask)
      {
        group.mask |= LVGF_TASK;
      }

      if (mask_not_set && group.pszDescriptionTop)
      {
        group.mask |= LVGF_DESCRIPTIONTOP;
      }

      if (mask_not_set && group.pszDescriptionBottom)
      {
        group.mask |= LVGF_DESCRIPTIONBOTTOM;
      }

      if (mask_not_set && group.iTitleImage)
      {
        group.mask |= LVGF_TITLEIMAGE;
      }

      if (mask_not_set && group.iExtendedImage)
      {
        group.mask |= LVGF_EXTENDEDIMAGE;
      }

      if (mask_not_set && group.cItems)
      {
        group.mask |= LVGF_ITEMS;
      }

      if (mask_not_set && group.pszSubsetTitle)
      {
        group.mask |= LVGF_SUBSET;
      }

      if (mask_not_set && group.cchSubsetTitle)
      {
        group.mask |= LVGF_SUBSETITEMS;
      }

      return ::SendMessageW(*this, LVM_INSERTGROUP, index, reinterpret_cast<LPARAM>(&group));
    }

    [[nodiscard]] win32::header GetHeader()
    {
      return win32::header(HWND(::SendMessageW(*this, LVM_GETHEADER, 0, 0)));
    }

    [[nodiscard]] WPARAM GetColumnCount()
    {
      return GetHeader().GetItemCount();
    }

    [[nodiscard]] std::optional<LVCOLUMNW> GetColumn(WPARAM index, LVCOLUMNW result = {})
    {
      if (::SendMessageW(*this, LVM_GETCOLUMNW, index, reinterpret_cast<LPARAM>(&result)))
      {
        return result;
      }

      return std::nullopt;
    }

    LPARAM GetColumnWidth(WPARAM index)
    {
      return ::SendMessageW(*this, LVM_GETCOLUMNWIDTH, index, 0);
    }

    [[maybe_unused]] bool SetColumnWidth(WPARAM index, LPARAM width)
    {
      return ::SendMessageW(*this, LVM_SETCOLUMNWIDTH, index, width);
    }

    [[maybe_unused]] WPARAM SetGroupInfo(WPARAM id, LVGROUP group)
    {
      group.cbSize = sizeof(LVGROUP);
      return ::SendMessageW(*this, LVM_SETGROUPINFO, id, LPARAM(&group));
    }

    [[maybe_unused]] WPARAM SetItem(LVITEMW item)
    {
      return ::SendMessageW(*this, LVM_SETITEMW, 0, LPARAM(&item));
    }

    [[maybe_unused]] WPARAM InsertColumn(WPARAM position, LVCOLUMNW column)
    {
      bool mask_not_set = column.mask == 0;

      if (mask_not_set && column.fmt)
      {
        column.mask |= LVCF_FMT;
      }

      if (mask_not_set && column.pszText)
      {
        column.mask |= LVCF_TEXT;
      }

      if (mask_not_set && column.cx)
      {
        if (!(column.cx == LVSCW_AUTOSIZE || column.cx == LVSCW_AUTOSIZE_USEHEADER))
        {
          column.mask |= LVCF_WIDTH;
        }
      }

      if (mask_not_set && column.cxMin)
      {
        column.mask |= LVCF_MINWIDTH;
      }

      if (mask_not_set && column.cxDefault)
      {
        column.mask |= LVCF_DEFAULTWIDTH;
      }

      if (mask_not_set && column.cxIdeal)
      {
        column.mask |= LVCF_IDEALWIDTH;
      }

      if (mask_not_set && column.iOrder)
      {
        column.mask |= LVCF_ORDER;
      }

      position = position == -1 ? GetColumnCount() : position;

      auto index = ::SendMessageW(*this, LVM_INSERTCOLUMNW, position, reinterpret_cast<LPARAM>(&column));

      if (column.cx)
      {
        SetColumnWidth(index, column.cx);
      }

      return index;
    }

    WPARAM GetItemCount() const
    {
      return ::SendMessageW(*this, LVM_GETITEMCOUNT, 0, 0);
    }

    WPARAM InsertItem(WPARAM index, LVITEMW item)
    {
      if (index == -1)
      {
        index = GetItemCount();
      }

      item.iItem = index;

      return ::SendMessageW(*this, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));
    }

    std::optional<LVITEMW> GetItem(LVITEMW item)
    {
      if (::SendMessageW(*this, LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&item)))
      {
        return item;
      }

      return std::nullopt;
    }

    [[maybe_unused]] bool SetTileInfo(LVTILEINFO info)
    {
      info.cbSize = sizeof(LVTILEINFO);
      return ::SendMessageW(*this, LVM_SETTILEINFO, 0, reinterpret_cast<LPARAM>(&info)) != 0;
    }

    [[nodiscard]] LRESULT FindItem(WPARAM start, LVFINDINFOW info)
    {
      return ::SendMessageW(*this, LVM_FINDITEMW, start, reinterpret_cast<LPARAM>(&info));
    }

    [[maybe_unused]] void GetItemText(WPARAM item, int sub_item, std::span<wchar_t> text)
    {
      ListView_GetItemText(*this, item, sub_item, text.data(), static_cast<int>(text.size()));
    }

    // LVM_SETITEMTEXT requires a null-terminated buffer. Views from std::wstring
    // and string literals already are; otherwise copy to a temporary.
    [[maybe_unused]] void SetItemText(WPARAM item, int sub_item, std::wstring_view text)
    {
      if (!text.empty() && text.data()[text.size()] == L'\0')
      {
        ListView_SetItemText(*this, item, sub_item, const_cast<wchar_t*>(text.data()));
        return;
      }
      std::wstring owned(text);
      ListView_SetItemText(*this, item, sub_item, owned.data());
    }

    [[maybe_unused]] bool Scroll(int dx, int dy)
    {
      return ::SendMessageW(*this, LVM_SCROLL, dx, dy) != 0;
    }

    [[maybe_unused]] bool DeleteColumn(WPARAM index)
    {
      return ::SendMessageW(*this, LVM_DELETECOLUMN, index, 0) != 0;
    }

    [[maybe_unused]] bool SetColumn(WPARAM index, LVCOLUMNW column)
    {
      return ::SendMessageW(*this, LVM_SETCOLUMNW, index, reinterpret_cast<LPARAM>(&column)) != 0;
    }

    [[maybe_unused]] bool DeleteAllItems()
    {
      return ::SendMessageW(*this, LVM_DELETEALLITEMS, 0, 0) != 0;
    }

    [[maybe_unused]] bool DeleteItem(WPARAM index)
    {
      return ::SendMessageW(*this, LVM_DELETEITEM, index, 0) != 0;
    }

    [[maybe_unused]] void SetCheckState(WPARAM index, bool checked)
    {
      ListView_SetCheckState(*this, index, checked ? TRUE : FALSE);
    }

    [[maybe_unused]] void SetItemState(WPARAM index, UINT state, UINT mask)
    {
      ListView_SetItemState(*this, index, state, mask);
    }

    [[nodiscard]] LRESULT GetNextItem(int start, UINT flags)
    {
      return ::SendMessageW(*this, LVM_GETNEXTITEM, start, MAKELPARAM(flags, 0));
    }

    [[nodiscard]] LRESULT SubItemHitTest(LVHITTESTINFO& info)
    {
      return ::SendMessageW(*this, LVM_SUBITEMHITTEST, 0, reinterpret_cast<LPARAM>(&info));
    }

    [[maybe_unused]] std::optional<RECT> GetSubItemRect(int item, int sub_item, int area)
    {
      RECT rect{ .left = area, .top = sub_item };
      if (::SendMessageW(*this, LVM_GETSUBITEMRECT, item, reinterpret_cast<LPARAM>(&rect)))
      {
        return rect;
      }
      return std::nullopt;
    }

    [[maybe_unused]] bool SetBkColor(COLORREF color)
    {
      return ::SendMessageW(*this, LVM_SETBKCOLOR, 0, color) != 0;
    }

    [[maybe_unused]] bool SetTextColor(COLORREF color)
    {
      return ::SendMessageW(*this, LVM_SETTEXTCOLOR, 0, color) != 0;
    }

    [[maybe_unused]] bool SetTextBkColor(COLORREF color)
    {
      return ::SendMessageW(*this, LVM_SETTEXTBKCOLOR, 0, color) != 0;
    }

    [[maybe_unused]] COLORREF SetOutlineColor(COLORREF color)
    {
      return (COLORREF)::SendMessageW(*this, LVM_SETOUTLINECOLOR, 0, color);
    }

    [[maybe_unused]] std::optional<LVGROUP> GetGroupInfo(int group_id, LVGROUP info)
    {
      info.cbSize = sizeof(LVGROUP);
      if (::SendMessageW(*this, LVM_GETGROUPINFO, group_id, reinterpret_cast<LPARAM>(&info)) != -1)
      {
        return info;
      }
      return std::nullopt;
    }

    [[maybe_unused]] std::optional<RECT> GetGroupRect(int group_id, int type = LVGGR_GROUP)
    {
      RECT rect{ .top = type };
      if (::SendMessageW(*this, LVM_GETGROUPRECT, group_id, reinterpret_cast<LPARAM>(&rect)) != 0)
      {
        return rect;
      }
      return std::nullopt;
    }

    auto InsertRow(list_view_item row)
    {
      auto index = this->InsertItem(-1, row);

      LVITEMW sub_item{};

      for (auto i = 1u; i <= row.sub_items.size(); ++i)
      {
        sub_item.iItem = index;
        sub_item.iSubItem = i;
        sub_item.pszText = row.sub_items[i - 1].data();
        sub_item.mask = LVIF_TEXT;

        ::SendMessageW(*this, LVM_SETITEMTEXT, index, (LPARAM)&sub_item);
      }

      return index;
    }

    void InsertGroups(std::span<list_view_group> groups)
    {
      int group_id = int(GetGroupCount() + 1);
      for (auto& group : groups)
      {
        group.iGroupId = group_id;

        InsertGroup(-1, group);
        auto items = std::move(group.items);

        for (auto& item : items)
        {
          item.iGroupId = group_id;
          item.mask = item.mask | LVIF_GROUPID;
          InsertRow(item);
        }

        group_id++;
      }
    }

    struct custom_draw_callbacks
    {
      std::move_only_function<LRESULT(win32::list_view, NMLVCUSTOMDRAW&)> nm_custom_draw;
    };

    [[maybe_unused]] std::function<void()> bind_custom_draw(custom_draw_callbacks callbacks)
    {
      struct function_context
      {
        HWND source;
        custom_draw_callbacks callbacks;
      };

      struct dispatcher
      {
        static LRESULT __stdcall handle_message(
          HWND hWnd,
          UINT uMsg,
          WPARAM wParam,
          LPARAM lParam,
          UINT_PTR uIdSubclass,
          DWORD_PTR dwRefData)
        {
          if (uMsg == WM_NOTIFY && lParam && uIdSubclass)
          {
            auto* header = (NMHDR*)lParam;
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.nm_custom_draw && header->hwndFrom == context.source && header->code == NM_CUSTOMDRAW)
            {
              return context.callbacks.nm_custom_draw(win32::list_view(header->hwndFrom), *(NMLVCUSTOMDRAW*)header);
            }
          }

          if (uMsg == WM_NCDESTROY)
          {
            auto* context = (function_context*)uIdSubclass;
            delete context;
            remove_window_subclass(hWnd, dispatcher::handle_message, uIdSubclass);
          }

          return def_subclass_proc(hWnd, uMsg, wParam, lParam);
        }
      };

      auto* context = new function_context{ this->get(), std::move(callbacks) };

      set_window_subclass(this->GetParent()->get(), dispatcher::handle_message, (UINT_PTR)context, 0);

      return [context, target = this->GetParent()->get()] {
        remove_window_subclass(target, dispatcher::handle_message, (UINT_PTR)context);
        delete context;
      };
    }
  };

  export struct rebar : window
  {
    using window::window;
    constexpr static auto class_Name = REBARCLASSNAMEW;
  };

  export struct tab_control : control
  {
    using control::control;
    using control::bind_notification;

    constexpr static auto class_name = WC_TABCONTROLW;

    [[nodiscard]] WPARAM GetItemCount()
    {
      return TabCtrl_GetItemCount(*this);
    }

    [[maybe_unused]] WPARAM InsertItem(WPARAM index, TCITEMW info)
    {
      if (index == -1)
      {
        index = GetItemCount();
      }

      return TabCtrl_InsertItem(*this, index, &info);
    }

    [[nodiscard]] WPARAM GetCurrentSelection()
    {
      return TabCtrl_GetCurSel(*this);
    }

    [[maybe_unused]] WPARAM SetCurrentSelection(WPARAM index)
    {
      return TabCtrl_SetCurSel(*this, index);
    }

    [[maybe_unused]] SIZE SetItemSize(SIZE size)
    {
      auto result = TabCtrl_SetItemSize(*this, size.cx, size.cy);
      return SIZE{ .cx = win32::loword(result), .cy = win32::hiword(result) };
    }

    [[maybe_unused]] auto SetPadding(int x, int y)
    {
      return TabCtrl_SetPadding(*this, x, y);
    }

    [[maybe_unused]] auto DeleteItem(WPARAM index)
    {
      return TabCtrl_DeleteItem(*this, index);
    }

    [[nodiscard]] auto HitTest(TCHITTESTINFO& info)
    {
      return TabCtrl_HitTest(*this, &info);
    }

    [[nodiscard]] std::optional<TCITEMW> GetItem(WPARAM index, TCITEMW result)
    {
      if (TabCtrl_GetItem(*this, index, &result))
      {
        return result;
      }

      return std::nullopt;
    }

    [[nodiscard]] std::optional<RECT> GetItemRect(WPARAM index)
    {
      RECT result;

      if (TabCtrl_GetItemRect(*this, index, &result))
      {
        return result;
      }

      return std::nullopt;
    }

    [[nodiscard]] std::optional<TCITEMW> GetItem(WPARAM index, std::uint32_t mask = TCIF_PARAM | TCIF_STATE)
    {
      TCITEMW result{ .mask = mask };

      if (TabCtrl_GetItem(*this, index, &result))
      {
        return result;
      }

      return std::nullopt;
    }

    [[maybe_unused]] bool SetItem(WPARAM index, TCITEMW result)
    {
      if (TabCtrl_SetItem(*this, index, &result))
      {
        return true;
      }

      return false;
    }

    RECT AdjustRect(bool dispay_to_window, RECT rect)
    {
      TabCtrl_AdjustRect(*this, dispay_to_window ? TRUE : FALSE, &rect);
      return rect;
    }

    [[maybe_unused]] std::function<void()> bind_nm_click(std::move_only_function<void(win32::tab_control, const NMHDR&)> callback)
    {
      return bind_notification<win32::tab_control, NMHDR>(NM_CLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_rclick(std::move_only_function<void(win32::tab_control, const NMHDR&)> callback)
    {
      return bind_notification<win32::tab_control, NMHDR>(NM_RCLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_dbl_click(std::move_only_function<void(win32::tab_control, const NMHDR&)> callback)
    {
      return bind_notification<win32::tab_control, NMHDR>(NM_DBLCLK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_dbl_rclick(std::move_only_function<void(win32::tab_control, const NMHDR&)> callback)
    {
      return bind_notification<win32::tab_control, NMHDR>(NM_RDBLCLK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_tcn_sel_changing(std::move_only_function<bool(win32::tab_control, const NMHDR&)> callback)
    {
      return bind_notification<win32::tab_control, NMHDR, bool>(TCN_SELCHANGING, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_tcn_sel_change(std::move_only_function<void(win32::tab_control, const NMHDR&)> callback)
    {
      return bind_notification<win32::tab_control, NMHDR>(TCN_SELCHANGE, std::move(callback));
    }

    struct custom_draw_callbacks
    {
      std::move_only_function<LRESULT(win32::tab_control, DRAWITEMSTRUCT&)> wm_draw_item;
    };

    [[maybe_unused]] std::function<void()> bind_custom_draw(custom_draw_callbacks callbacks)
    {
      struct function_context
      {
        HWND source;
        custom_draw_callbacks callbacks;
      };

      struct dispatcher
      {
        static LRESULT __stdcall handle_message(
          HWND hWnd,
          UINT uMsg,
          WPARAM wParam,
          LPARAM lParam,
          UINT_PTR uIdSubclass,
          DWORD_PTR dwRefData)
        {

          if (uMsg == WM_DRAWITEM && lParam && uIdSubclass)
          {
            auto* header = (DRAWITEMSTRUCT*)lParam;
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.wm_draw_item && header->hwndItem == context.source)
            {
              return context.callbacks.wm_draw_item(win32::tab_control(header->hwndItem), *header);
            }
          }

          if (uMsg == WM_NCDESTROY)
          {
            auto* context = (function_context*)uIdSubclass;
            delete context;
            remove_window_subclass(hWnd, dispatcher::handle_message, uIdSubclass);
          }

          return def_subclass_proc(hWnd, uMsg, wParam, lParam);
        }
      };

      auto* context = new function_context{ this->get(), std::move(callbacks) };

      set_window_subclass(this->GetParent()->get(), dispatcher::handle_message, (UINT_PTR)context, 0);

      return [context, target = this->GetParent()->get()] {
        remove_window_subclass(target, dispatcher::handle_message, (UINT_PTR)context);
        delete context;
      };
    }
  };

  export struct tool_bar : control
  {
    using control::control;
    constexpr static auto class_name = TOOLBARCLASSNAMEW;

    enum extended_style : DWORD
    {
      mixed_buttons = TBSTYLE_EX_MIXEDBUTTONS,
      draw_drop_down_arrows = TBSTYLE_EX_DRAWDDARROWS
    };

    void AutoSize()
    {
      ::SendMessageW(*this, TB_AUTOSIZE, 0, 0);
    }

    [[maybe_unused]] bool SetButtonWidth(std::array<int, 2> range)
    {
      auto [min, max] = range;

      return ::SendMessageW(*this, TB_SETBUTTONWIDTH, 0, MAKELPARAM(min, max));
    }

    [[maybe_unused]] bool SetButtonSize(SIZE size)
    {
      return ::SendMessageW(*this, TB_SETBUTTONSIZE, 0, MAKELPARAM(size.cx, size.cy));
    }

    [[nodiscard]] auto ButtonCount() const
    {
      return ::SendMessageW(*this, TB_BUTTONCOUNT, 0, 0);
    }

    [[nodiscard]] SIZE GetPadding()
    {
      auto result = ::SendMessageW(*this, TB_GETPADDING, 0, 0);

      return SIZE{ .cx = win32::loword(result), .cy = win32::hiword(result) };
    }

    [[nodiscard]] SIZE GetButtonSize()
    {
      auto result = ::SendMessageW(*this, TB_GETBUTTONSIZE, 0, 0);

      return SIZE{ .cx = win32::loword(result), .cy = win32::hiword(result) };
    }

    [[nodiscard]] SIZE GetIdealIconSize(std::optional<SIZE> size_hint = std::nullopt)
    {
      auto button_size = size_hint ? *size_hint : GetButtonSize();
      auto padding = GetPadding();

      button_size.cx -= padding.cx;
      button_size.cy -= padding.cy;

      if (this->GetWindowStyle() & TBSTYLE_FLAT)
      {
        auto font = SendMessage(*this, WM_GETFONT, 0, 0);

        if (font)
        {
          win32::gdi::memory_drawing_context context;
          SelectFont(context, font);
          ::SIZE text_size{};
          if (::GetTextExtentPoint32W(context, L"MWQqPpYyZz", 11, &text_size))
          {
            button_size.cy -= text_size.cy;
          }
        }
      }

      return SIZE{ .cx = button_size.cy, .cy = button_size.cy };
    }

    [[nodiscard]] std::optional<RECT> GetItemRect(WPARAM index)
    {
      RECT result;
      if (::SendMessageW(*this, TB_GETITEMRECT, index, reinterpret_cast<LPARAM>(&result)))
      {
        return result;
      }

      return std::nullopt;
    }

    [[nodiscard]] std::optional<RECT> GetRect(WPARAM id)
    {
      RECT result;
      if (::SendMessageW(*this, TB_GETRECT, id, reinterpret_cast<LPARAM>(&result)))
      {
        return result;
      }

      return std::nullopt;
    }

    [[maybe_unused]] extended_style SetExtendedStyle(LPARAM style)
    {
      return extended_style(::SendMessageW(*this,
        TB_SETEXTENDEDSTYLE,
        0,
        style));
    }

    [[maybe_unused]] bool InsertButton(WPARAM index, TBBUTTON button, bool auto_id = true)
    {
      if (index < 0)
      {
        index = ButtonCount() - index + 1;
      }

      if (auto_id)
      {
        button.idCommand = ButtonCount();
      }

      ::SendMessageW(*this, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
      return ::SendMessageW(*this, TB_INSERTBUTTONW, index, reinterpret_cast<LPARAM>(&button));
    }

    [[maybe_unused]] bool AddButtons(std::span<TBBUTTON> buttons)
    {
      ::SendMessageW(*this, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
      return ::SendMessageW(*this, TB_ADDBUTTONSW, WPARAM(buttons.size()), reinterpret_cast<LPARAM>(buttons.data()));
    }

    auto LoadImages(WPARAM image_list_id)
    {
      return ::SendMessageW(*this, TB_LOADIMAGES, image_list_id, (LPARAM)HINST_COMMCTRL);
    }

    [[maybe_unused]] HIMAGELIST SetImageList(HIMAGELIST list, WPARAM image_list_id = 0)
    {
      return reinterpret_cast<HIMAGELIST>(::SendMessageW(*this, TB_SETIMAGELIST, image_list_id, reinterpret_cast<LPARAM>(list)));
    }

    [[maybe_unused]] bool SetBitmapSize(SIZE size)
    {
      return ::SendMessageW(*this, TB_SETBITMAPSIZE, 0, MAKELPARAM(size.cx, size.cy)) != 0;
    }

    [[nodiscard]] LRESULT GetState(WPARAM id)
    {
      return ::SendMessageW(*this, TB_GETSTATE, id, 0);
    }

    [[maybe_unused]] bool SetState(WPARAM id, int state)
    {
      return ::SendMessageW(*this, TB_SETSTATE, id, MAKELPARAM(state, 0)) != 0;
    }

    [[maybe_unused]] bool SetButtonInfo(WPARAM id, TBBUTTONINFOW info)
    {
      info.cbSize = sizeof(TBBUTTONINFOW);
      return ::SendMessageW(*this, TB_SETBUTTONINFO, id, reinterpret_cast<LPARAM>(&info)) != 0;
    }

    [[maybe_unused]] void SetColorScheme(COLORSCHEME scheme)
    {
      scheme.dwSize = sizeof(COLORSCHEME);
      ::SendMessageW(*this, TB_SETCOLORSCHEME, 0, reinterpret_cast<LPARAM>(&scheme));
    }

    auto PressButton(WPARAM id, bool is_pressed = true)
    {
      return ::SendMessageW(*this, TB_PRESSBUTTON, id, MAKELPARAM(is_pressed ? TRUE : FALSE, 0));
    }

    [[maybe_unused]] std::function<void()> bind_nm_click(std::move_only_function<bool(win32::tool_bar, const NMMOUSE&)> callback)
    {
      return bind_notification<win32::tool_bar, NMMOUSE, bool>(NM_CLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_rclick(std::move_only_function<bool(win32::tool_bar, const NMMOUSE&)> callback)
    {
      return bind_notification<win32::tool_bar, NMMOUSE, bool>(NM_RCLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_dbl_click(std::move_only_function<bool(win32::tool_bar, const NMMOUSE&)> callback)
    {
      return bind_notification<win32::tool_bar, NMMOUSE, bool>(NM_DBLCLK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_dbl_rclick(std::move_only_function<bool(win32::tool_bar, const NMMOUSE&)> callback)
    {
      return bind_notification<win32::tool_bar, NMMOUSE, bool>(NM_RDBLCLK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_tbn_dropdown(std::move_only_function<LRESULT(win32::tool_bar, const NMTOOLBARW&)> callback)
    {
      return bind_notification<win32::tool_bar, NMTOOLBARW, LRESULT>(TBN_DROPDOWN, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_tbn_hot_item_change(std::move_only_function<LRESULT(win32::tool_bar, const NMTBHOTITEM&)> callback)
    {
      return bind_notification<win32::tool_bar, NMTBHOTITEM, LRESULT>(TBN_HOTITEMCHANGE, std::move(callback));
    }

    struct custom_draw_callbacks
    {
      std::move_only_function<LRESULT(tool_bar, NMTBCUSTOMDRAW&)> nm_custom_draw;
    };

    [[maybe_unused]] std::function<void()> bind_custom_draw(custom_draw_callbacks callbacks)
    {
      struct function_context
      {
        HWND source;
        custom_draw_callbacks callbacks;
      };

      struct dispatcher
      {
        static LRESULT __stdcall handle_message(
          HWND hWnd,
          UINT uMsg,
          WPARAM wParam,
          LPARAM lParam,
          UINT_PTR uIdSubclass,
          DWORD_PTR dwRefData)
        {

          if (uMsg == WM_NOTIFY && lParam && uIdSubclass)
          {
            auto* header = (NMHDR*)lParam;
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.nm_custom_draw && header->hwndFrom == context.source && header->code == NM_CUSTOMDRAW)
            {
              return context.callbacks.nm_custom_draw(win32::tool_bar(header->hwndFrom), *(NMTBCUSTOMDRAW*)header);
            }
          }

          if (uMsg == WM_NCDESTROY)
          {
            auto* context = (function_context*)uIdSubclass;
            delete context;
            remove_window_subclass(hWnd, dispatcher::handle_message, uIdSubclass);
          }

          return def_subclass_proc(hWnd, uMsg, wParam, lParam);
        }
      };

      auto* context = new function_context{ this->get(), std::move(callbacks) };

      set_window_subclass(this->GetParent()->get(), dispatcher::handle_message, (UINT_PTR)context, 0);

      return [context, target = this->GetParent()->get()] {
        remove_window_subclass(target, dispatcher::handle_message, (UINT_PTR)context);
        delete context;
      };
    }
  };

  export struct tree_view_item : ::TVINSERTSTRUCTW
  {
    std::wstring text;

    std::vector<tree_view_item> children;

    tree_view_item(const std::filesystem::path& path) : ::TVINSERTSTRUCTW{}, text(path.wstring())
    {
      this->item.pszText = this->text.data();
      this->item.lParam = (LPARAM)path.c_str();
    }

    tree_view_item(const std::filesystem::path& path, std::wstring text) : ::TVINSERTSTRUCTW{}, text(std::move(text))
    {
      this->item.pszText = this->text.data();
      this->item.lParam = (LPARAM)path.c_str();
    }

    tree_view_item(const std::filesystem::path& path, std::wstring text, std::vector<tree_view_item> children) : ::TVINSERTSTRUCTW{}, text(std::move(text)), children(std::move(children))
    {
      this->item.pszText = this->text.data();
      this->item.lParam = (LPARAM)path.c_str();
    }

    tree_view_item(const tree_view_item& item) : ::TVINSERTSTRUCTW(item), text(item.text), children(item.children)
    {
      this->item.pszText = text.data();
    }
  };

  export struct tree_view : control
  {
    using control::control;
    constexpr static auto class_name = WC_TREEVIEWW;

    [[maybe_unused]] bool Expand(HTREEITEM item, WPARAM action = TVE_EXPAND)
    {
      return TreeView_Expand(*this, item, action);
    }

    [[maybe_unused]] bool Collapse(HTREEITEM item, WPARAM action = TVE_COLLAPSE)
    {
      return TreeView_Expand(*this, item, action);
    }

    [[maybe_unused]] bool EnsureVisible(HTREEITEM item)
    {
      return ::SendMessageW(*this, TVM_ENSUREVISIBLE, 0, reinterpret_cast<LPARAM>(item));
    }

    [[maybe_unused]] bool DeleteItem(HTREEITEM item)
    {
      return ::SendMessageW(*this, TVM_DELETEITEM, 0, reinterpret_cast<LPARAM>(item));
    }

    [[maybe_unused]] bool SetImageList(WPARAM type, HIMAGELIST list)
    {
      return ::SendMessageW(*this, TVM_SETIMAGELIST, type, reinterpret_cast<LPARAM>(list));
    }

    [[nodiscard]] std::optional<TVITEMW> GetItem(TVITEMW info)
    {
      if (TreeView_GetItem(*this, &info))
      {
        return info;
      }

      return std::nullopt;
    }

    [[nodiscard]] HTREEITEM GetParent(HTREEITEM item)
    {
      return TreeView_GetParent(*this, item);
    }

    [[nodiscard]] HTREEITEM GetChild(HTREEITEM item)
    {
      return TreeView_GetChild(*this, item);
    }

    [[nodiscard]] HTREEITEM HitTest(TVHITTESTINFO& info)
    {
      return TreeView_HitTest(*this, &info);
    }

    [[maybe_unused]] auto SetBkColor(COLORREF color)
    {
      return TreeView_SetBkColor(*this, color);
    }

    [[maybe_unused]] auto SetTextColor(COLORREF color)
    {
      return TreeView_SetTextColor(*this, color);
    }

    [[maybe_unused]] auto SetLineColor(COLORREF color)
    {
      return TreeView_SetLineColor(*this, color);
    }

    void SetItemMask(TVITEMW& info)
    {
      bool mask_not_set = info.mask == 0;

      if (mask_not_set && info.pszText)
      {
        info.mask |= TVIF_TEXT;
      }

      if (mask_not_set && info.hItem)
      {
        info.mask |= TVIF_HANDLE;
      }

      if (mask_not_set && info.iImage)
      {
        info.mask |= TVIF_IMAGE;
      }

      if (mask_not_set && info.lParam)
      {
        info.mask |= TVIF_PARAM;
      }

      if (mask_not_set && info.iSelectedImage)
      {
        info.mask |= TVIF_SELECTEDIMAGE;
      }

      if (mask_not_set && (info.state || info.stateMask))
      {
        info.mask |= TVIF_STATE;
      }

      if (mask_not_set && info.cChildren)
      {
        info.mask |= TVIF_CHILDREN;
      }
    }

    [[maybe_unused]] std::optional<HTREEITEM> InsertItem(TVINSERTSTRUCTW info)
    {
      SetItemMask(info.item);

      if (auto result = HTREEITEM(::SendMessageW(*this, TVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&info))); result)
      {
        return result;
      }

      return std::nullopt;
    }

    [[maybe_unused]] bool SetItem(TVITEMW info)
    {
      SetItemMask(info);

      return ::SendMessageW(*this, TVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&info));
    }

    [[maybe_unused]] void InsertRoots(std::span<tree_view_item> items)
    {
      for (auto& item : items)
      {
        item.hParent = TVI_ROOT;
        item.item.cChildren = 1;

        auto result = InsertItem(item);

        auto children = std::move(item.children);

        for (auto& child : children)
        {
          child.hParent = *result;
          child.hInsertAfter = TVI_LAST;
          InsertItem(child);
        }

        Expand(*result);
      }
    }

    [[maybe_unused]] std::function<void()> bind_nm_click(std::move_only_function<bool(win32::tree_view, const NMHDR&)> callback)
    {
      return bind_notification<win32::tree_view, NMHDR, bool>(NM_CLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_dbl_click(std::move_only_function<bool(win32::tree_view, const NMHDR&)> callback)
    {
      return bind_notification<win32::tree_view, NMHDR, bool>(NM_DBLCLK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_dbl_rclick(std::move_only_function<bool(win32::tree_view, const NMHDR&)> callback)
    {
      return bind_notification<win32::tree_view, NMHDR, bool>(NM_RDBLCLK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_nm_rclick(std::move_only_function<bool(win32::tree_view, const NMHDR&)> callback)
    {
      return bind_notification<win32::tree_view, NMHDR, bool>(NM_RCLICK, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_tvn_sel_changed(std::move_only_function<void(win32::tree_view, const NMTREEVIEWW&)> callback)
    {
      return bind_notification<win32::tree_view, NMTREEVIEWW>(TVN_SELCHANGED, std::move(callback));
    }

    [[maybe_unused]] std::function<void()> bind_tvn_item_expanding(std::move_only_function<bool(win32::tree_view, const NMTREEVIEWW&)> callback)
    {
      return bind_notification<win32::tree_view, NMTREEVIEWW, bool>(TVN_ITEMEXPANDING, std::move(callback));
    }
  };

  export struct page_scroller : window
  {
    using window::window;
    constexpr static auto class_name = WC_PAGESCROLLERW;
  };

  export struct combo_box_ex : control
  {
    using control::control;
    constexpr static auto class_name = WC_COMBOBOXEXW;

    [[maybe_unused]] auto ResetContent()
    {
      return ComboBox_ResetContent(*this);
    }

    [[maybe_unused]] bool ShowDropDown(bool show = true)
    {
      return ComboBox_ShowDropdown(*this, show ? TRUE : FALSE) != 0;
    }

    [[nodiscard]] std::optional<int> GetCurSel()
    {
      auto result = ComboBox_GetCurSel(*this);
      if (result == CB_ERR)
      {
        return std::nullopt;
      }
      return result;
    }

    [[maybe_unused]] std::optional<int> InsertItem(COMBOBOXEXITEMW item)
    {
      auto result = ::SendMessageW(*this, CBEM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));
      if (result == -1)
      {
        return std::nullopt;
      }
      return (int)result;
    }

    [[maybe_unused]] std::optional<COMBOBOXEXITEMW> GetItem(COMBOBOXEXITEMW item)
    {
      if (::SendMessageW(*this, CBEM_GETITEMW, 0, reinterpret_cast<LPARAM>(&item)))
      {
        return item;
      }
      return std::nullopt;
    }

    [[maybe_unused]] std::function<void()> bind_cbn_sel_change(std::move_only_function<void(combo_box_ex, const NMHDR&)> callback)
    {
      return bind_notification<combo_box_ex, NMHDR>(CBN_SELCHANGE, std::move(callback));
    }
  };

}// namespace win32
