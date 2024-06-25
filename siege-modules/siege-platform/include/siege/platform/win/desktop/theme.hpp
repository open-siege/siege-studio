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

  inline void apply_theme(win32::list_view& list, const std::map<std::wstring, COLORREF>& colors)
  {
    auto color = colors.find(properties::list_view::bk_color);

    if (color != colors.end())
    {
      ListView_SetBkColor(list, color->second);
    }

    color = colors.find(properties::list_view::text_color);

    if (color != colors.end())
    {
      ListView_SetTextColor(list, color->second);
    }

    color = colors.find(properties::list_view::text_bk_color);

    if (color != colors.end())
    {
      ListView_SetTextBkColor(list, color->second);
    }

    color = colors.find(properties::list_view::outline_color);

    if (color != colors.end())
    {
      ListView_SetOutlineColor(list, color->second);
    }
  }

  inline void apply_theme(win32::tree_view& list, const std::map<std::wstring, COLORREF>& colors)
  {
    auto color = colors.find(properties::tree_view::bk_color);

    if (color != colors.end())
    {
      ListView_SetBkColor(list, color->second);
    }

    color = colors.find(properties::tree_view::text_color);

    if (color != colors.end())
    {
      ListView_SetTextColor(list, color->second);
    }

    color = colors.find(properties::tree_view::line_color);

    if (color != colors.end())
    {
      ListView_SetTextBkColor(list, color->second);
    }
  }

  inline void apply_theme(win32::tool_bar& list, const std::map<std::wstring_view, COLORREF>& colors)
  {
    auto color = colors.find(properties::tool_bar::btn_highlight_color);

    bool change_theme = false;

    COLORSCHEME scheme{ .dwSize = sizeof(COLORSCHEME), .clrBtnHighlight = CLR_DEFAULT, .clrBtnShadow = CLR_DEFAULT };

    if (color != colors.end())
    {
      change_theme = true;
      scheme.clrBtnHighlight = color->second;
    }

    color = colors.find(properties::tool_bar::btn_shadow_color);

    if (color != colors.end())
    {
      change_theme = true;
      scheme.clrBtnShadow = color->second;
    }

    if (scheme.clrBtnHighlight != CLR_DEFAULT || scheme.clrBtnShadow != CLR_DEFAULT)
    {
      ::SendMessageW(list, TB_SETCOLORSCHEME, 0, (LPARAM)&scheme);
    }

    static auto change_color = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) -> LRESULT __stdcall
    {
      // TODO remove this sub class on destroy
      if (uMsg == WM_NOTIFY && lParam != 0)
      {
        NMHDR* header = (NMHDR*)lParam;

        if (header->code == NM_CUSTOMDRAW && header->hwndFrom == (HWND)dwRefData)
        {
          NMTBCUSTOMDRAW* custom_draw = (NMTBCUSTOMDRAW*)lParam;

          if (custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
          {
            return CDRF_NOTIFYITEMDRAW;
          }

          if (custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT || custom_draw->nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
          {
            ::EnumPropsExW(
              header->hwndFrom, [](HWND, LPWSTR key, HANDLE value, ULONG_PTR lparam) -> BOOL __stdcall {
                auto* inner = (NMTBCUSTOMDRAW*)lparam;

                auto intAtom = ::GlobalFindAtomW(key);
                if (intAtom == (ATOM)(std::size_t)key)
                {
                  return TRUE;
                }

                if (key && key == properties::tool_bar::btn_face_color)
                {
                  inner->clrBtnFace = (COLORREF)value;
                }

                if (key && key == properties::tool_bar::btn_highlight_color)
                {
                  inner->clrBtnHighlight = (COLORREF)value;
                }

                if (key && key == properties::tool_bar::text_color)
                {
                  inner->clrText = (COLORREF)value;
                }

                return TRUE;
              },
              (LPARAM)custom_draw);
            return CDRF_NEWFONT | TBCDRF_USECDCOLORS;
          }


          return CDRF_DODEFAULT;
        }
      }

      return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    };

    if (colors.contains(properties::tool_bar::btn_face_color) || colors.contains(properties::tool_bar::btn_highlight_color) || colors.contains(properties::tool_bar::text_color))
    {
      for (auto& prop : colors)
      {
        if (prop.first.find(win32::tool_bar::class_name) != std::wstring_view::npos)
        {
          list.SetPropW(prop.first, prop.second);
        }
      }


      change_theme = true;
      ::SetWindowSubclass(
        *list.GetParent(), change_color, (UINT_PTR)properties::tool_bar::text_color.data(), (DWORD_PTR)list.get());
    }

    if (change_theme)
    {
      win32::theme_module().SetWindowTheme(list, L"", L"");
    }
    else
    {
      win32::theme_module().SetWindowTheme(list, nullptr, nullptr);
      ::RemoveWindowSubclass(*list.GetParent(), change_color, (UINT_PTR)properties::tool_bar::text_color.data());
    }
  }

}// namespace win32

#endif