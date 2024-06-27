#ifndef OPEN_SIEGE_THEMING_HPP
#define OPEN_SIEGE_THEMING_HPP

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

    struct list_box
    {
      inline static auto bk_color = std::wstring(win32::list_box::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::list_box::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::list_box::class_name) + L"." + L"TextBkColor";
      inline static auto text_highlight_color = std::wstring(win32::list_box::class_name) + L"." + L"TextHighlightColor";
    };

    struct header
    {
      inline static auto bk_color = std::wstring(win32::header::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::header::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::header::class_name) + L"." + L"TextBkColor";
      inline static auto text_highlight_color = std::wstring(win32::header::class_name) + L"." + L"TextHighlightColor";
    };

    struct tab_control
    {
      inline static auto bk_color = std::wstring(win32::tab_control::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::tab_control::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::tab_control::class_name) + L"." + L"TextBkColor";
      inline static auto text_highlight_color = std::wstring(win32::tab_control::class_name) + L"." + L"TextHighlightColor";
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

  void apply_theme(const win32::window_ref& colors, win32::header& control);
  void apply_theme(const win32::window_ref& colors, win32::tab_control& control);
  void apply_theme(const win32::window_ref& colors, win32::list_box& control);
  void apply_theme(const win32::window_ref& colors, win32::list_view& control);
  void apply_theme(const win32::window_ref& colors, win32::tree_view& control);
  void apply_theme(const win32::window_ref& colors, win32::tool_bar& control);
}// namespace win32

#endif