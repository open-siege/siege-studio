#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/core/com/base.hpp>
#include <wincodec.h>
#include <VersionHelpers.h>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);

  auto& bitmap_factory()
  {
    thread_local win32::com::com_ptr factory = [] {
      win32::com::com_ptr<IWICImagingFactory> temp;

      if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), temp.put_void()) != S_OK)
      {
        throw std::exception("Could not create imaging factory");
      }

      return temp;
    }();

    return *factory;
  }

  struct cache_info
  {
    HBITMAP bitmap = nullptr;
    std::span<RGBQUAD> pixels;
  };

  struct image_cache
  {
    std::map<std::wstring, cache_info> layer_masks;
    std::map<HBITMAP, HDC> dc_cache;

    std::map<HBITMAP, std::span<RGBQUAD>> mask_pixels;
    std::map<HBITMAP, std::span<RGBQUAD>> target_pixels;

    HBITMAP default_bitmap;
  };

  image_cache& get_image_cache()
  {
    thread_local image_cache cache;
    return cache;
  }

  gdi::bitmap_ref create_layer_mask(::SIZE size, gdi::font_ref font, std::wstring text)
  {
    std::wstring key;
    key.reserve(3 + text.size());

    key.append(1, size.cx);
    key.append(1, size.cy);
    key.append(1, (wchar_t)font.get());

    auto screen_dc = gdi::drawing_context::from_screen();
    gdi::memory_drawing_context temp_dc(gdi::drawing_context_ref(screen_dc.get()));
    screen_dc.reset();

    SelectObject(temp_dc, font);
    SIZE font_size{};
    ::GetTextExtentPoint32W(temp_dc, text.data(), text.size(), &font_size);

    key.append(1, font_size.cx);
    key.append(1, font_size.cy);
    key.append(text);

    auto& cache = get_image_cache();
    auto existing = cache.layer_masks.find(key);

    if (existing != cache.layer_masks.end())
    {
      return gdi::bitmap_ref(existing->second.bitmap);
    }

    BITMAPINFO info{
      .bmiHeader{
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = LONG(font_size.cx),
        .biHeight = LONG(font_size.cy),
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB }
    };
    void* pixels = nullptr;
    auto temp_bitmap = ::CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    assert(temp_bitmap);

    auto old_bitmap = (HBITMAP)SelectObject(temp_dc, temp_bitmap);

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
    SelectObject(temp_dc, old_bitmap);

    std::span<RGBQUAD> colors((RGBQUAD*)pixels, font_size.cx * font_size.cy);

    for (auto& color : colors)
    {
      color.rgbReserved = color.rgbRed;
    }

    win32::com::com_ptr<IWICBitmap> downscaled_mask;

    assert(bitmap_factory().CreateBitmapFromHBITMAP(temp_bitmap, nullptr, WICBitmapAlphaChannelOption::WICBitmapUseAlpha, downscaled_mask.put()) == S_OK);

    win32::com::com_ptr<IWICBitmapScaler> scaler;
    assert(bitmap_factory().CreateBitmapScaler(scaler.put()) == S_OK);

    auto resampling_mode = IsWindows10OrGreater() ? WICBitmapInterpolationModeHighQualityCubic : WICBitmapInterpolationModeFant;
    scaler->Initialize(downscaled_mask.get(), size.cx, size.cy, resampling_mode);

    BITMAPINFO final_info{
      .bmiHeader{
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = LONG(size.cx),
        .biHeight = LONG(size.cy),
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB }
    };
    void* final_pixels = nullptr;
    cache_info mask_cache{};

    mask_cache.bitmap = ::CreateDIBSection(nullptr, &final_info, DIB_RGB_COLORS, &final_pixels, nullptr, 0);

    auto stride = size.cx * sizeof(std::int32_t);

    assert(scaler->CopyPixels(nullptr, stride, size.cy * stride * sizeof(std::int32_t), reinterpret_cast<BYTE*>(final_pixels)) == S_OK);
    mask_cache.pixels = std::span<RGBQUAD>((RGBQUAD*)final_pixels, size.cx * size.cy);


    cache.layer_masks.emplace(key, mask_cache);
    cache.mask_pixels.emplace(mask_cache.bitmap, mask_cache.pixels);

    DeleteObject(temp_bitmap);

    return gdi::bitmap_ref(mask_cache.bitmap);
  }

  gdi::bitmap_ref create_layer_mask(SIZE size, int scale, std::move_only_function<void(gdi::drawing_context_ref, int)> painter)
  {

    std::wstring key;
    key.reserve(3 + 16);
    key.push_back(wchar_t(size.cx));
    key.push_back(wchar_t(size.cy));
    key.push_back(wchar_t(scale));

    auto& cache = get_image_cache();

    auto screen_dc = gdi::drawing_context::from_screen();
    gdi::memory_drawing_context path_dc(gdi::drawing_context_ref(screen_dc.get()));

    BeginPath(path_dc);
    painter(win32::gdi::drawing_context_ref(path_dc), scale);
    EndPath(path_dc);

    std::array<POINT, 16> points{};
    std::array<BYTE, 16> commands{};
    GetPath(path_dc, points.data(), commands.data(), points.size());
    AbortPath(path_dc);

    for (auto command : commands)
    {
      key.push_back(wchar_t(command));
    }

    auto existing = cache.layer_masks.find(key);

    if (existing != cache.layer_masks.end())
    {
      return gdi::bitmap_ref(existing->second.bitmap);
    }

    BITMAPINFO info{
      .bmiHeader{
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = LONG(size.cx * scale),
        .biHeight = LONG(size.cy * scale),
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB }
    };
    void* pixels = nullptr;
    auto temp_bitmap = ::CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);

    gdi::memory_drawing_context temp_dc(gdi::drawing_context_ref(screen_dc.get()));

    auto old_bitmap = (HBITMAP)SelectObject(temp_dc, temp_bitmap);
    RECT temp_rect{ .left = 0, .top = 0, .right = size.cx * scale, .bottom = size.cy * scale };
    COLORREF temp_color = RGB(0, 0, 0);
    FillRect(temp_dc, &temp_rect, get_solid_brush(temp_color));

    SelectObject(temp_dc, get_solid_brush(RGB(255, 255, 255)));

    BeginPath(temp_dc);
    painter(win32::gdi::drawing_context_ref(temp_dc), scale);
    EndPath(temp_dc);

    // TODO see how we can use the path for caching
    FillPath(temp_dc);
    temp_dc.reset();

    std::span<RGBQUAD> colors((RGBQUAD*)pixels, size.cx * scale * size.cy * scale);
    for (auto& color : colors)
    {
      color.rgbReserved = color.rgbRed;
    }

    win32::com::com_ptr<IWICBitmap> downscaled_mask;

    assert(bitmap_factory().CreateBitmapFromHBITMAP(temp_bitmap, nullptr, WICBitmapAlphaChannelOption::WICBitmapUseAlpha, downscaled_mask.put()) == S_OK);

    win32::com::com_ptr<IWICBitmapScaler> scaler;
    assert(bitmap_factory().CreateBitmapScaler(scaler.put()) == S_OK);

    auto resampling_mode = IsWindows10OrGreater() ? WICBitmapInterpolationModeHighQualityCubic : WICBitmapInterpolationModeFant;
    scaler->Initialize(downscaled_mask.get(), size.cx, size.cy, resampling_mode);

    BITMAPINFO final_info{
      .bmiHeader{
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = LONG(size.cx),
        .biHeight = LONG(size.cy),
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB }
    };
    void* final_pixels = nullptr;
    cache_info mask_cache{};

    mask_cache.bitmap = ::CreateDIBSection(nullptr, &final_info, DIB_RGB_COLORS, &final_pixels, nullptr, 0);

    auto stride = size.cx * sizeof(std::int32_t);

    assert(scaler->CopyPixels(nullptr, stride, size.cy * stride * sizeof(std::int32_t), reinterpret_cast<BYTE*>(final_pixels)) == S_OK);
    mask_cache.pixels = std::span<RGBQUAD>((RGBQUAD*)final_pixels, size.cx * size.cy);

    cache.layer_masks.emplace(key, mask_cache);
    cache.mask_pixels.emplace(mask_cache.bitmap, mask_cache.pixels);

    // TODO find out if the bitmap is copied or if a reference is kept to it
    DeleteObject(temp_bitmap);

    return gdi::bitmap_ref(mask_cache.bitmap);
  }

  win32::gdi::drawing_context_ref apply_layer_mask(win32::gdi::drawing_context_ref source, gdi::bitmap_ref bitmap)
  {
    auto& cache = get_image_cache();
    auto existing = cache.dc_cache.find(bitmap);

    BITMAP mask_info{};
    GetObjectW(bitmap, sizeof(BITMAP), &mask_info);

    if (existing == cache.dc_cache.end())
    {
      auto temp_dc = CreateCompatibleDC(source);

      BITMAPINFO info{
        .bmiHeader{
          .biSize = sizeof(BITMAPINFOHEADER),
          .biWidth = mask_info.bmWidth,
          .biHeight = mask_info.bmHeight,
          .biPlanes = 1,
          .biBitCount = 32,
          .biCompression = BI_RGB }
      };
      void* pixels = nullptr;
      auto temp_bitmap = ::CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);

      cache.target_pixels.emplace(bitmap, std::span<RGBQUAD>((RGBQUAD*)pixels, mask_info.bmWidth * mask_info.bmHeight));

      auto old_bitmap = SelectObject(temp_dc, temp_bitmap);

      if (!cache.default_bitmap)
      {
        cache.default_bitmap = (HBITMAP)old_bitmap;
      }

      existing = cache.dc_cache.emplace(bitmap, temp_dc).first;
    }

    BitBlt(existing->second, 0, 0, mask_info.bmWidth, mask_info.bmHeight, source, 0, 0, SRCCOPY);

    auto mask_pixels = cache.mask_pixels[bitmap];
    auto control_pixels = cache.target_pixels[bitmap];

    for (auto i = 0u; i < mask_pixels.size(); ++i)
    {
      control_pixels[i].rgbReserved = mask_pixels[i].rgbReserved;
    }

    return win32::gdi::drawing_context_ref(existing->second);
  }

  gdi::icon create_icon(::SIZE size, ::RGBQUAD solid_color, gdi::bitmap_ref mask)
  {
    std::vector<RGBQUAD> pixels;
    pixels.resize(size.cx * size.cy);

    auto& cache = get_image_cache();
    auto mask_pixels = cache.mask_pixels[mask];

    std::size_t index = 0;
    std::transform(pixels.begin(), pixels.end(), pixels.begin(), [solid_color, &mask, &index, &mask_pixels](RGBQUAD temp) {
      std::memcpy(&temp, &solid_color, sizeof(solid_color));
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

}// namespace win32