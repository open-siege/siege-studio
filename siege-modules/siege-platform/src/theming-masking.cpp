#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/core/com/base.hpp>
#include <siege/platform/win/desktop/wic.hpp>
#include <siege/platform/win/desktop/direct_2d.hpp>
#include <execution>
#include <algorithm>
#include <wincodec.h>
#include <VersionHelpers.h>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);

  win32::image_list create_icon_list(std::span<segoe_fluent_icons> icons, SIZE icon_size, std::optional<::RGBQUAD> optional_color)
  {
    win32::gdi::bitmap target(icon_size);

    win32::wic::bitmap temp(target, win32::wic::alpha_channel_option::WICBitmapUsePremultipliedAlpha);

    win32::direct2d::wic_bitmap_render_target render_target(temp, D2D1_RENDER_TARGET_PROPERTIES{
      .type = D2D1_RENDER_TARGET_TYPE_SOFTWARE,
      .pixelFormat = D2D1::PixelFormat(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        D2D1_ALPHA_MODE_PREMULTIPLIED),
      .usage = D2D1_RENDER_TARGET_USAGE_NONE,
      .minLevel = D2D1_FEATURE_LEVEL_DEFAULT });

    win32::directwrite::text_format format({ .family_name = L"Segoe MDL2 Assets", .size = (FLOAT)icon_size.cx });

    D2D1::ColorF text_color(0, 0, 0);

    if (optional_color)
    {
      // RGB produces a BGR int, so swapping B and R gets us an RGB int
      text_color = D2D1::ColorF(RGB(optional_color->rgbBlue, optional_color->rgbGreen, optional_color->rgbRed));
    }
    else
    {
      auto theme_color = win32::get_color_for_window(nullptr, win32::properties::button::text_color);
      // RGB produces a BGR int, so swapping B and R gets us an RGB int
      text_color = D2D1::ColorF(RGB(GetBValue(theme_color), GetGValue(theme_color), GetRValue(theme_color)));
    }

    auto brush = render_target.create_solid_color_brush(text_color);
    std::wstring icon_text;

    win32::image_list image_list(ImageList_Create(icon_size.cx, icon_size.cy, ILC_COLOR32, icons.size(), icons.size()));

    for (auto icon : icons)
    {
      render_target.begin_draw();
      icon_text.assign(1, (wchar_t)icon);
      render_target.clear(D2D1::ColorF(0, 0, 0, 0));
      render_target.draw_text(icon_text, format, D2D1::RectF(0, 0, (float)icon_size.cx, (float)icon_size.cy), brush);
      render_target.end_draw();
      // TODO turn into a d2d transform
      ImageList_Add(image_list, target, nullptr);
    }

    return image_list;
  }

}// namespace win32