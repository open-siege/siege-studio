#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/core/com/base.hpp>
#include <wincodec.h>
#include <VersionHelpers.h>

namespace win32
{
  HBRUSH get_solid_brush(COLORREF color);

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
    std::map<std::array<int, 3>, cache_info> layer_masks;
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


  HBITMAP create_layer_mask(SIZE size, int scale, std::move_only_function<void(HDC, int)> painter)
  {
    static auto screen_dc = GetDC(nullptr);

    std::array<int, 3> key{ { size.cx, size.cy, scale } };

    auto& cache = get_image_cache();
    auto existing = cache.layer_masks.find(key);

    if (existing != cache.layer_masks.end())
    {
      return existing->second.bitmap;
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
    auto temp_bitmap = ::CreateDIBSection(screen_dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);

    auto temp_dc = CreateCompatibleDC(screen_dc);
    auto old_bitmap = (HBITMAP)SelectObject(temp_dc, temp_bitmap);
    RECT temp_rect{ .left = 0, .top = 0, .right = size.cx * scale, .bottom = size.cy * scale };
    COLORREF temp_color = RGB(0, 0, 0);
    FillRect(temp_dc, &temp_rect, get_solid_brush(temp_color));

    SelectObject(temp_dc, get_solid_brush(RGB(255, 255, 255)));

    BeginPath(temp_dc);
    painter(temp_dc, scale);
    EndPath(temp_dc);

    // TODO see how we can use the path for caching
    FillPath(temp_dc);

    std::span<RGBQUAD> colors((RGBQUAD*)pixels, size.cx * scale * size.cy * scale);
    for (auto& color : colors)
    {
      if (std::memcmp(&temp_color, &color, sizeof(COLORREF)) != 0)
      {
        color.rgbReserved = 0xff;
      }
    }

    win32::com::com_ptr<IWICBitmap> downscaled_mask;

    SelectObject(temp_dc, old_bitmap);
    DeleteDC(temp_dc);

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

    mask_cache.bitmap = ::CreateDIBSection(screen_dc, &final_info, DIB_RGB_COLORS, &final_pixels, nullptr, 0);

    auto stride = size.cx * sizeof(std::int32_t);

    assert(scaler->CopyPixels(nullptr, stride, size.cy * stride * sizeof(std::int32_t), reinterpret_cast<BYTE*>(final_pixels)) == S_OK);
    mask_cache.pixels = std::span<RGBQUAD>((RGBQUAD*)final_pixels, size.cx * size.cy);

    cache.layer_masks.emplace(key, mask_cache);
    cache.mask_pixels.emplace(mask_cache.bitmap, mask_cache.pixels);

    // TODO find out if the bitmap is copied or if a reference is kept to it
    DeleteObject(temp_bitmap);

    return mask_cache.bitmap;
  }

  HDC apply_layer_mask(HDC source, HBITMAP bitmap)
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
      auto temp_bitmap = ::CreateDIBSection(source, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);

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

    return existing->second;
  }

}// namespace win32