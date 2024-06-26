#ifndef OPEN_SIEGE_THEME_MODULE_HPP
#define OPEN_SIEGE_THEME_MODULE_HPP

#include <siege/platform/win/core/module.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <map>
#include <string>
#include <uxtheme.h>

namespace win32
{
  struct theme_module : private win32::module
  {
    theme_module() : win32::module("uxtheme.dll", true)
    {
      set_window_theme = this->GetProcAddress<decltype(set_window_theme)>("SetWindowTheme");

      if (set_window_theme == nullptr)
      {
        throw std::exception("Could not load theme window functions");
      }
    }

    HRESULT SetWindowTheme(HWND window, const wchar_t* app_name, const wchar_t* id)
    {
      return set_window_theme(window, app_name, id);
    }

  private:
    std::add_pointer_t<decltype(::SetWindowTheme)> set_window_theme;
  };


  struct properties
  {
    struct tree_view
    {
      inline static auto bk_color = std::wstring(win32::tree_view::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::tree_view::class_name) + L"." + L"TextColor";
      inline static auto line_color = std::wstring(win32::tree_view::class_name) + L"." + L"LineColor";
    };

    struct list_view
    {
      inline static auto bk_color = std::wstring(win32::list_view::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::list_view::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::list_view::class_name) + L"." + L"TextBkColor";
      inline static auto outline_color = std::wstring(win32::list_view::class_name) + L"." + L"OutlineColor";
    };

    struct tool_bar
    {
      inline static auto btn_highlight_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnHighlightColor";
      inline static auto btn_shadow_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnShadowColor";
      inline static auto btn_face_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnFaceColor";
      inline static auto text_color = std::wstring(win32::tool_bar::class_name) + L"." + L"TextColor";
      inline static auto text_highlight_color = std::wstring(win32::tool_bar::class_name) + L"." + L"TextHighlightColor";
      inline static auto mark_color = std::wstring(win32::tool_bar::class_name) + L"." + L"MarkColor";
    };

    struct menu
    {
      constexpr static auto bk_color = std::wstring_view(L"Menu.BkColor");
    };

    struct window
    {
      constexpr static auto bk_color = std::wstring_view(L"Window.BkColor");
    };
  };

  inline void apply_theme(const win32::window_ref& colors, win32::list_view& control)
  {
    static auto default_bk_color = ListView_GetBkColor(control);
    auto color = colors.FindPropertyExW<COLORREF>(properties::list_view::bk_color).value_or(default_bk_color);
    ListView_SetBkColor(control, color);

    static auto default_text_color = ListView_GetTextColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::text_color).value_or(default_text_color);
    ListView_SetTextColor(control, color);
   
    static auto default_text_bk_color = ListView_GetTextBkColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::text_bk_color).value_or(default_text_bk_color);
    ListView_SetTextBkColor(control, color);

    static auto default_outline_color = ListView_GetOutlineColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::outline_color).value_or(default_outline_color);
    ListView_SetOutlineColor(control, color);
  }

  inline void apply_theme(const win32::window_ref& colors, win32::tree_view& control)
  {
    auto color = colors.FindPropertyExW<COLORREF>(properties::tree_view::bk_color).value_or(CLR_NONE);
    TreeView_SetBkColor(control, color); 
   
    color = colors.FindPropertyExW<COLORREF>(properties::tree_view::text_color).value_or(CLR_NONE);
    TreeView_SetTextColor(control, color);

    color = colors.FindPropertyExW<COLORREF>(properties::tree_view::line_color).value_or(CLR_NONE);
    TreeView_SetLineColor(control, color);
  }

  inline void apply_theme(const win32::window_ref& colors, win32::tool_bar& control)
  {
    auto highlight_color = colors.FindPropertyExW(properties::tool_bar::btn_highlight_color);
    auto shadow_color = colors.FindPropertyExW(properties::tool_bar::btn_shadow_color);

    bool change_theme = false;

    if (highlight_color || shadow_color)
    {
      change_theme = true;
      COLORSCHEME scheme{ .dwSize = sizeof(COLORSCHEME), .clrBtnHighlight = CLR_DEFAULT, .clrBtnShadow = CLR_DEFAULT };

      if (highlight_color)
      {
        scheme.clrBtnHighlight = (COLORREF)*highlight_color;
      }

      if (shadow_color)
      {
        scheme.clrBtnShadow = (COLORREF)*shadow_color;
      }

      ::SendMessageW(control, TB_SETCOLORSCHEME, 0, (LPARAM)&scheme);
    }

    static auto change_color = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) -> LRESULT __stdcall
    {
      // TODO remove this sub class on destroy
      if (uMsg == WM_NOTIFY && lParam != 0)
      {
        NMHDR* header = (NMHDR*)lParam;

        if (header->code == NM_CUSTOMDRAW && header->hwndFrom == (HWND)uIdSubclass)
        {
          NMTBCUSTOMDRAW* custom_draw = (NMTBCUSTOMDRAW*)lParam;

          if (custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
          {
            return CDRF_NOTIFYITEMDRAW;
          }

          if (custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT || custom_draw->nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
          {
            win32::window_ref(header->hwndFrom).ForEachPropertyExW([=](win32::hwnd_t, std::wstring_view key, HANDLE value) {
              if (key == properties::tool_bar::btn_face_color)
              {
                custom_draw->clrBtnFace = (COLORREF)value;
              }

              if (key == properties::tool_bar::btn_highlight_color)
              {
                custom_draw->clrBtnHighlight = (COLORREF)value;
              }

              if (key == properties::tool_bar::text_color)
              {
                custom_draw->clrText = (COLORREF)value;
              }
            });
            return CDRF_NEWFONT | TBCDRF_USECDCOLORS;
          }

          return CDRF_DODEFAULT;
        }
      }

      return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    };

    if (colors.FindPropertyExW(properties::tool_bar::btn_face_color) || 
        colors.FindPropertyExW(properties::tool_bar::text_color))
    {
      change_theme = true;

      colors.ForEachPropertyExW([&](auto, auto key, auto value) {
        if (key.find(win32::tool_bar::class_name) != std::wstring_view::npos)
        {
          control.SetPropW(key, value);
        }
      });

    ::SetWindowSubclass(
        *control.GetParent(), change_color, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
    }

    if (change_theme)
    {
      win32::theme_module().SetWindowTheme(control, L"", L"");
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
      ::RemoveWindowSubclass(*control.GetParent(), change_color, (UINT_PTR)control.get());
    }
  }

}// namespace win32

#endif