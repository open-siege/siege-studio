#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/com.hpp>
#include <siege/platform/win/wic.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <execution>
#include <algorithm>
#include <wincodec.h>
#include <VersionHelpers.h>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);

  
  gdi::bitmap create_layer_mask(::SIZE size, gdi::font_ref font, std::wstring text)
  {
    auto screen_dc = gdi::drawing_context::from_screen();
    gdi::memory_drawing_context temp_dc(gdi::drawing_context_ref(screen_dc.get()));
    screen_dc.reset();

    SelectObject(temp_dc, font);
    SIZE font_size{};
    ::GetTextExtentPoint32W(temp_dc, text.data(), text.size(), &font_size);


    win32::gdi::bitmap temp_bitmap{ font_size };

    SelectObject(temp_dc, temp_bitmap.get());

    RECT text_rect{
      .right = font_size.cx,
      .bottom = font_size.cy
    };

    COLORREF background_color = RGB(255, 255, 255);
    COLORREF text_color = RGB(0, 0, 0);

    SetTextColor(temp_dc, text_color);
    SelectObject(temp_dc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(temp_dc, text_color);

    SetBkColor(temp_dc, background_color);
    SelectObject(temp_dc, GetStockObject(DC_PEN));
    SetDCPenColor(temp_dc, background_color);

    ::DrawTextW(temp_dc, text.data(), text.size(), &text_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    auto pixels = temp_bitmap.get_pixels();
    for (auto& color : pixels)
    {
      color.rgbReserved = color.rgbRed;
    }

    auto resampling_mode = IsWindows10OrGreater() ? WICBitmapInterpolationModeHighQualityCubic : WICBitmapInterpolationModeFant;

    win32::gdi::bitmap mask_cache{ size };

    auto mask_cache_pixels = mask_cache.get_pixels();

    auto stride = size.cx * sizeof(std::int32_t);

    win32::wic::bitmap(temp_bitmap, win32::wic::alpha_channel_option::WICBitmapUseAlpha)
      .scale(size.cx, size.cy, resampling_mode)
      .copy_pixels(stride, std::span<std::byte>((std::byte*)mask_cache_pixels.data(), size.cy * stride * sizeof(std::int32_t)));

    return mask_cache;
  }

  gdi::icon create_icon(::SIZE size, ::RGBQUAD solid_color, gdi::bitmap_ref mask)
  {
    std::vector<RGBQUAD> pixels;
    pixels.resize(size.cx * size.cy, solid_color);

    auto mask_pixels = mask.get_pixels();

    std::size_t index = 0;

    std::transform(pixels.begin(), pixels.end(), pixels.begin(), [&index, &mask_pixels](RGBQUAD temp) {
      temp.rgbReserved = 255 - mask_pixels[index++].rgbReserved;
      return temp;
    });

    ::ICONINFO info{
      .fIcon = TRUE,
      .hbmMask = ::CreateBitmap(size.cx, size.cy, 1, 1, nullptr),
      .hbmColor = ::CreateBitmap(size.cx, size.cy, 1, 32, pixels.data()),
    };

    auto icon = gdi::icon(::CreateIconIndirect(&info));

    DeleteObject(info.hbmColor);
    DeleteObject(info.hbmMask);
    return icon;
  }

  win32::image_list create_icon_list(std::span<segoe_fluent_icons> icons, SIZE icon_size, std::optional<::RGBQUAD> optional_color)
  {
    auto font_icon = win32::load_font(LOGFONTW{
      .lfHeight = -1024,
      .lfClipPrecision = CLIP_DEFAULT_PRECIS,
      .lfQuality = NONANTIALIASED_QUALITY,
      .lfFaceName = L"Segoe MDL2 Assets" });

    win32::image_list image_list(ImageList_Create(icon_size.cx, icon_size.cy, ILC_COLOR32, icons.size(), icons.size()));

    RGBQUAD color{};

    if (optional_color)
    {
      color = *optional_color;
    }
    else
    {
      auto theme_color = win32::get_color_for_window(nullptr, win32::properties::button::text_color);
      color = { .rgbBlue = GetBValue(theme_color),
        .rgbGreen = GetGValue(theme_color),
        .rgbRed = GetRValue(theme_color) };
    }

    std::vector<std::pair<int, segoe_fluent_icons>> icon_indices;
    icon_indices.reserve(icons.size());
    std::transform(icons.begin(), icons.end(), std::back_inserter(icon_indices), [i = 0](auto icon) mutable {
      return std::make_pair(i++, icon);
    });

    std::vector<win32::gdi::icon> final_icons;
    final_icons.resize(icons.size());

    std::for_each(std::execution::par_unseq, icon_indices.begin(), icon_indices.end(), [&](auto& icon) {
      auto mask = win32::create_layer_mask(icon_size, win32::gdi::font_ref(font_icon.get()), std::wstring(1, (wchar_t)icon.second));
      final_icons[icon.first] = win32::create_icon(icon_size, color, mask.ref());
    });

    std::for_each(final_icons.begin(), final_icons.end(), [&](auto& icon) {
      ImageList_ReplaceIcon(image_list.get(), -1, icon);
    });

    return image_list;
  }

}// namespace win32