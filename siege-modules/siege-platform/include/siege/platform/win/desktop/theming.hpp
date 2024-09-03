#ifndef OPEN_SIEGE_THEMING_HPP
#define OPEN_SIEGE_THEMING_HPP

#include <siege/platform/win/core/module.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <map>
#include <string>
#include <optional>
#include <uxtheme.h>

namespace win32
{
  gdi::bitmap_ref create_layer_mask(::SIZE size, gdi::font_ref font, std::wstring text);
  gdi::bitmap_ref create_layer_mask(::SIZE size, int scale, std::move_only_function<void(gdi::drawing_context_ref, int)> painter);
  gdi::icon create_icon(::SIZE size, ::RGBQUAD solid_color, gdi::bitmap_ref mask);
  gdi::drawing_context_ref apply_layer_mask(gdi::drawing_context_ref source, gdi::bitmap_ref bitmap);
  gdi::font_ref load_font(::LOGFONTW font_info);
  gdi::brush_ref get_solid_brush(COLORREF color);

  bool is_dark_theme();
  void set_is_dark_theme(bool);
  COLORREF get_color_for_window(win32::window_ref, std::wstring_view);
  std::optional<COLORREF> set_color_for_window(win32::window_ref, std::wstring_view, std::optional<COLORREF> color);
  std::optional<SIZE> get_font_size_for_string(gdi::font_ref, std::wstring_view);

  enum struct segoe_fluent_icons : wchar_t
  {
    global_navigation_button = 0xe700,
    chevron_up = 0xE70D,
    chevron_down = 0xE70E,
    edit = 0xE70F,
    add = 0xE710,
    cancel = 0xE711,
    more = 0xE712,
    setting = 0xE713,
    filter = 0xE71C,
    zoom = 0xE71E,
    zoom_out = 0xE71F,
    zoom_in = 0xE8A3,
    open_in_new_window = 0xE8A7,
    open_file = 0xE8E5,
    folder_open = 0xE838,
    rotate = 0xE7AD,
    stop = 0xE71A,
    play = 0xE768,
    pause = 0xE769,
    repeat_all = 0xE8EE,
    grid_view = 0xF0E2,
    group_list = 0xF168,
    toggle_filled = 0xEC11,
    toggle_border = 0xEC12,
    slider_thumb = 0xEC13,
    toggle_thumb = 0xEC14,
    file_explorer = 0xEC50,
    scroll_up_down = 0xEC8F,
    radio_btn_off = 0xecca,
    radio_btn_on = 0xeccb,
    radio_bullet_2 = 0xECCC,
    radio_bullet = 0xE915,
    pan_mode = 0xece9,
    image_export = 0xee71,
    picture = 0xe8b9
  };

  win32::image_list create_icon_list(std::span<segoe_fluent_icons> icons, SIZE icon_size, std::optional<::RGBQUAD> color = std::nullopt);

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
    struct button
    {
      inline static auto bk_color = std::wstring(win32::button::class_name) + L"." + L"BkColor";
      inline static auto hot_bk_color = std::wstring(win32::button::class_name) + L"." + L"HotBkColor";
      inline static auto focus_bk_color = std::wstring(win32::button::class_name) + L"." + L"FocusBkColor";
      inline static auto pushed_bk_color = std::wstring(win32::button::class_name) + L"." + L"PushedBkColor";
      inline static auto text_color = std::wstring(win32::button::class_name) + L"." + L"TextColor";
      inline static auto line_color = std::wstring(win32::button::class_name) + L"." + L"LineColor";

      inline static auto props = std::array<std::wstring_view, 3>{
        bk_color,
        text_color,
        line_color
      };
    };

    struct edit
    {
      inline static auto bk_color = std::wstring(win32::edit::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::edit::class_name) + L"." + L"TextColor";
      inline static auto line_color = std::wstring(win32::edit::class_name) + L"." + L"LineColor";

      inline static auto props = std::array<std::wstring_view, 3>{
        bk_color,
        text_color,
        line_color
      };
    };

    struct track_bar
    {
      inline static auto bk_color = std::wstring(win32::header::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::header::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::header::class_name) + L"." + L"TextBkColor";

      inline static auto props = std::array<std::wstring_view, 3>{
        bk_color,
        text_color,
        text_bk_color
      };
    };

    struct combo_box
    {
      inline static auto bk_color = std::wstring(win32::combo_box::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::combo_box::class_name) + L"." + L"TextColor";
      inline static auto line_color = std::wstring(win32::combo_box::class_name) + L"." + L"LineColor";

      inline static auto props = std::array<std::wstring_view, 3>{
        bk_color,
        text_color,
        line_color
      };
    };

    struct combo_box_ex
    {
      inline static auto bk_color = std::wstring(win32::combo_box_ex::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::combo_box_ex::class_name) + L"." + L"TextColor";
      inline static auto line_color = std::wstring(win32::combo_box_ex::class_name) + L"." + L"LineColor";

      inline static auto props = std::array<std::wstring_view, 3>{
        bk_color,
        text_color,
        line_color
      };
    };


    struct header
    {
      inline static auto bk_color = std::wstring(win32::header::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::header::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::header::class_name) + L"." + L"TextBkColor";
      inline static auto text_highlight_color = std::wstring(win32::header::class_name) + L"." + L"TextHighlightColor";

      inline static auto props = std::array<std::wstring_view, 4>{
        bk_color,
        text_color,
        text_bk_color,
        text_highlight_color
      };
    };

    struct tree_view
    {
      inline static auto bk_color = std::wstring(win32::tree_view::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::tree_view::class_name) + L"." + L"TextColor";
      inline static auto line_color = std::wstring(win32::tree_view::class_name) + L"." + L"LineColor";

      inline static auto props = std::array<std::wstring_view, 3>{
        bk_color,
        text_color,
        line_color
      };
    };

    struct list_view
    {
      inline static auto bk_color = std::wstring(win32::list_view::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::list_view::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::list_view::class_name) + L"." + L"TextBkColor";
      inline static auto outline_color = std::wstring(win32::list_view::class_name) + L"." + L"OutlineColor";

      inline static auto props = std::array<std::wstring_view, 4>{
        bk_color,
        text_color,
        text_bk_color,
        outline_color
      };
    };

    struct tool_bar
    {
      inline static auto bk_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BkColor";
      inline static auto btn_highlight_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnHighlightColor";
      inline static auto btn_shadow_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnShadowColor";
      inline static auto btn_face_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnFaceColor";
      inline static auto text_color = std::wstring(win32::tool_bar::class_name) + L"." + L"TextColor";
      inline static auto text_highlight_color = std::wstring(win32::tool_bar::class_name) + L"." + L"TextHighlightColor";
      inline static auto mark_color = std::wstring(win32::tool_bar::class_name) + L"." + L"MarkColor";

      inline static auto props = std::array<std::wstring_view, 7>{
        bk_color,
        btn_highlight_color,
        btn_shadow_color,
        btn_face_color,
        text_color,
        text_highlight_color,
        mark_color
      };
    };

    struct list_box
    {
      inline static auto bk_color = std::wstring(win32::list_box::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::list_box::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::list_box::class_name) + L"." + L"TextBkColor";
      inline static auto text_highlight_color = std::wstring(win32::list_box::class_name) + L"." + L"TextHighlightColor";

      inline static auto props = std::array<std::wstring_view, 4>{
        bk_color,
        text_color,
        text_bk_color,
        text_highlight_color
      };
    };


    struct tab_control
    {
      inline static auto bk_color = std::wstring(win32::tab_control::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::tab_control::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::tab_control::class_name) + L"." + L"TextBkColor";
      inline static auto text_highlight_color = std::wstring(win32::tab_control::class_name) + L"." + L"TextHighlightColor";

      inline static auto props = std::array<std::wstring_view, 4>{
        bk_color,
        text_color,
        text_bk_color,
        text_highlight_color
      };
    };

    struct static_control
    {
      inline static auto bk_color = std::wstring(win32::static_control::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::static_control::class_name) + L"." + L"TextColor";

      inline static auto props = std::array<std::wstring_view, 2>{
        bk_color,
        text_color
      };
    };

    struct menu
    {
      constexpr static auto bk_color = std::wstring_view(L"Menu.BkColor");
      constexpr static auto text_color = std::wstring_view(L"Menu.TextColor");
      constexpr static auto text_highlight_color = std::wstring_view(L"Menu.TextHighlightColor");

      inline static auto props = std::array<std::wstring_view, 3>{
        bk_color,
        text_color,
        text_highlight_color
      };
    };

    struct window
    {
      constexpr static auto bk_color = std::wstring_view(L"Window.BkColor");

      inline static auto props = std::array<std::wstring_view, 1>{
        bk_color
      };
    };
  };

  // user controls
  void apply_theme(win32::window_ref& control);
  void apply_theme(win32::button& control);
  void apply_theme(win32::edit& control);
  void apply_theme(win32::static_control& control);
  void apply_theme(win32::list_box& control);

  // common controls
  void apply_theme(win32::tree_view& control);
  void apply_theme(win32::list_view& control);
  void apply_theme(win32::tab_control& control);
  void apply_theme(win32::header& control);
  void apply_theme(win32::tool_bar& control);
  void apply_theme(win32::track_bar& control);
}// namespace win32

#endif