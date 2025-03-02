#ifndef WIN_DESKTOP_WIC_HPP
#define WIN_DESKTOP_WIC_HPP

#include <vector>
#include <cstdint>
#include <span>
#include <siege/platform/win/com.hpp>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/drawing.hpp>
#include <wincodec.h>

namespace win32::wic
{
  using namespace win32::com;

  struct pixel_format : GUID
  {
    pixel_format(GUID guid) : GUID(std::move(guid))
    {
    }

    inline static auto indexed_1bpp = GUID_WICPixelFormat1bppIndexed;
    inline static auto indexed_2bpp = GUID_WICPixelFormat2bppIndexed;
    inline static auto indexed_4bpp = GUID_WICPixelFormat4bppIndexed;
    inline static auto indexed_8bpp = GUID_WICPixelFormat8bppIndexed;
    inline static auto bgr555_16bpp = GUID_WICPixelFormat16bppBGR555;
    inline static auto bgra555_16bpp = GUID_WICPixelFormat16bppBGRA5551;
    inline static auto bgr565_16bpp = GUID_WICPixelFormat16bppBGR565;
    inline static auto rgb_24bpp = GUID_WICPixelFormat24bppRGB;
    inline static auto rgb_32bpp = GUID_WICPixelFormat32bppRGB;
    inline static auto rgba_32bpp = GUID_WICPixelFormat32bppRGBA;
    inline static auto prgba_32bpp = GUID_WICPixelFormat32bppPRGBA;
    inline static auto bgr_24bpp = GUID_WICPixelFormat24bppBGR;
    inline static auto bgr_32bpp = GUID_WICPixelFormat32bppBGR;
    inline static auto bgra_32bpp = GUID_WICPixelFormat32bppBGRA;
    inline static auto pbgra_32bpp = GUID_WICPixelFormat32bppPBGRA;
  };

  using cache_create_option = WICBitmapCreateCacheOption;
  using alpha_channel_option = WICBitmapAlphaChannelOption;
  using section_access_level = WICSectionAccessLevel;
  using interpolation_mode = WICBitmapInterpolationMode;
  using transform_options = WICBitmapTransformOptions;
  using decode_options = WICDecodeOptions;
  using dither_type = WICBitmapDitherType;
  using palette_type = WICBitmapPaletteType;

  class bitmap_factory
  {
    bitmap_factory() = delete;
    bitmap_factory(const bitmap_factory&) = delete;
    bitmap_factory(bitmap_factory&&) = delete;
    bitmap_factory operator=(const bitmap_factory&) = delete;
    bitmap_factory operator=(bitmap_factory&&) = delete;

  public:
    static auto& instance()
    {
      thread_local win32::com::com_ptr factory = [] {
        win32::com::com_ptr<IWICImagingFactory> temp;

        if (::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), temp.put_void()) != S_OK)
        {
          throw std::exception("Could not create imaging factory");
        }

        return temp;
      }();

      return *factory;
    }
  };

  class bitmap_source;

  class palette
  {
  public:
    static palette null()
    {
      return palette();
    }

    palette(bitmap_source& source, std::uint32_t colour_count = 256, bool add_transparent_color = false);

    palette(std::span<color> colors)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreatePalette(instance.put()));

      std::vector<WICColor> temp;
      temp.resize(colors.size());

      std::transform(colors.begin(), colors.end(), temp.begin(), [](auto value) { return value.to_wic_color(); });
      hresult_throw_on_error(instance->InitializeCustom(temp.data(), (UINT)colors.size()));
    }

    palette(WICBitmapPaletteType type, bool add_transparent_color = false)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreatePalette(instance.put()));
      hresult_throw_on_error(instance->InitializePredefined(type, add_transparent_color ? TRUE : FALSE));
    }

    palette(const palette& other)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreatePalette(instance.put()));

      if (other.instance)
      {
        hresult_throw_on_error(instance->InitializeFromPalette(other.instance.get()));
      }
    }

    bool has_alpha()
    {
      BOOL result;
      hresult_throw_on_error(instance->HasAlpha(&result));
      return result == TRUE;
    }

    bool is_black_white()
    {
      BOOL result;
      hresult_throw_on_error(instance->IsBlackWhite(&result));
      return result == TRUE;
    }

    bool is_grayscale()
    {
      BOOL result;
      hresult_throw_on_error(instance->IsGrayscale(&result));
      return result == TRUE;
    }

    WICBitmapPaletteType get_type()
    {
      WICBitmapPaletteType result;
      hresult_throw_on_error(instance->GetType(&result));
      return result;
    }

    std::uint32_t get_color_count()
    {
      std::uint32_t count;
      hresult_throw_on_error(instance->GetColorCount(&count));
      return count;
    }

    std::vector<color> get_colors()
    {
      std::vector<WICColor> wic_colors;
      wic_colors.resize(get_color_count());
      hresult_throw_on_error(instance->GetColors((UINT)wic_colors.size(), wic_colors.data(), nullptr));

      std::vector<color> colors;
      colors.resize(wic_colors.size());
      std::transform(wic_colors.begin(), wic_colors.end(), colors.begin(), [](WICColor value) { return color::from_wic_color(value); });
      return colors;
    }

    win32::com::com_ptr<IWICPalette> handle()
    {
      return instance.as<IWICPalette>();
    }

  private:
    palette()
    {
    }
    win32::com::com_ptr<IWICPalette> instance;
  };

  class bitmap_source
  {

  public:
    bitmap_source(bitmap_source&&) = default;
    bitmap_source(const bitmap_source&) = default;
    bitmap_source& operator=(const bitmap_source&) = default;
    bitmap_source& operator=(bitmap_source&&) = default;

    bitmap_source(com::com_ptr<IWICBitmapSource> instance) : instance(std::move(instance))
    {
    }

    struct to_format
    {
      pixel_format format;
      dither_type dither_type = dither_type::WICBitmapDitherTypeNone;
      palette palette = palette::null();
      double alpha_threshold_percent;
      palette_type palette_type = palette_type::WICBitmapPaletteTypeCustom;
    };


    bitmap_source convert(to_format options) const
    {
      win32::com::com_ptr<IWICFormatConverter> converter;
      hresult_throw_on_error(bitmap_factory::instance().CreateFormatConverter(converter.put()));
      auto handle = options.palette.handle();
      hresult_throw_on_error(converter->Initialize(instance.get(), options.format, options.dither_type, handle.get(), options.alpha_threshold_percent, options.palette_type));


      return bitmap_source(converter.as<IWICBitmapSource>());
    }

    pixel_format get_pixel_format() const
    {
      GUID format;
      hresult_throw_on_error(instance->GetPixelFormat(&format));
      return pixel_format(format);
    }

    bitmap_source scale(std::uint32_t width, std::uint32_t height, interpolation_mode mode) const
    {
      win32::com::com_ptr<IWICBitmapScaler> scaler;
      hresult_throw_on_error(bitmap_factory::instance().CreateBitmapScaler(scaler.put()));
      hresult_throw_on_error(scaler->Initialize(instance.get(), width, height, mode));

      return bitmap_source(scaler.as<IWICBitmapSource>());
    }

    bitmap_source clip(WICRect rect) const
    {
      win32::com::com_ptr<IWICBitmapClipper> clipper;
      hresult_throw_on_error(bitmap_factory::instance().CreateBitmapClipper(clipper.put()));

      hresult_throw_on_error(clipper->Initialize(instance.get(), &rect));

      return bitmap_source(clipper.as<IWICBitmapSource>());
    }

    bitmap_source flip(transform_options options) const
    {
      win32::com::com_ptr<IWICBitmapFlipRotator> flipper;
      hresult_throw_on_error(bitmap_factory::instance().CreateBitmapFlipRotator(flipper.put()));

      hresult_throw_on_error(flipper->Initialize(instance.get(), options));

      return bitmap_source(flipper.as<IWICBitmapSource>());
    }

    SIZE get_size() const
    {
      UINT width;
      UINT height;
      hresult_throw_on_error(instance->GetSize(&width, &height));

      return SIZE{ .cx = (LONG)width, .cy = (LONG)height };
    }

    void copy_pixels(std::uint32_t stride, std::span<std::byte> buffer, std::optional<WICRect> source = std::nullopt)
    {
      if (source)
      {
        hresult_throw_on_error(instance->CopyPixels(&*source, stride, (UINT)buffer.size(), (BYTE*)buffer.data()));
      }
      else
      {

        hresult_throw_on_error(instance->CopyPixels(nullptr, stride, (UINT)buffer.size(), (BYTE*)buffer.data()));
      }
    }

    template<typename THandle = IWICBitmapSource>
    win32::com::com_ptr<THandle> handle()
    {
      return instance.as<THandle>();
    }

  protected:
    bitmap_source()
    {
    }

    win32::com::com_ptr<IWICBitmapSource> instance;
  };

  inline palette::palette(bitmap_source& source, std::uint32_t colour_count, bool add_transparent_color)
  {
    hresult_throw_on_error(bitmap_factory::instance().CreatePalette(instance.put()));
    auto handle = source.handle();
    hresult_throw_on_error(instance->InitializeFromBitmap(handle.get(), colour_count, add_transparent_color ? TRUE : FALSE));
  }

  class bitmap_encoder;

  class bitmap_frame_encode
  {
  public:
    friend class bitmap_encoder;

    void write_source(bitmap_source& source, std::optional<WICRect> rect = std::nullopt)
    {
      auto handle = source.handle();
      hresult_throw_on_error(instance->WriteSource(handle.get(), rect ? &*rect : nullptr));
    }

    void write_pixels(std::uint32_t line_count, std::uint32_t stride, std::span<std::byte> buffer)
    {
      hresult_throw_on_error(instance->WritePixels(line_count, stride, (UINT)buffer.size(), (BYTE*)buffer.data()));
    }

    void commit()
    {
      hresult_throw_on_error(instance->Commit());
    }

  private:
    bitmap_frame_encode(IWICBitmapEncoder& encoder) : instance([&] {
                                                        com_ptr<IWICBitmapFrameEncode> temp;
                                                        hresult_throw_on_error(encoder.CreateNewFrame(temp.put(), nullptr));
                                                        hresult_throw_on_error(temp->Initialize(nullptr));
                                                        return temp;
                                                      }())
    {
    }
    win32::com::com_ptr<IWICBitmapFrameEncode> instance;
  };

  class bitmap_encoder
  {
  public:
    virtual bool supports_multiple_frames() const
    {
      return false;
    }

    bitmap_frame_encode create_new_frame()
    {
      return bitmap_frame_encode(*instance);
    }

    void commit()
    {
      hresult_throw_on_error(instance->Commit());
    }

  protected:
    bitmap_encoder(GUID format, std::filesystem::path file)
    {
      auto& factory = bitmap_factory::instance();
      hresult_throw_on_error(factory.CreateEncoder(format, nullptr, instance.put()));
      com_ptr<IWICStream> stream;
      hresult_throw_on_error(bitmap_factory::instance().CreateStream(stream.put()));
      hresult_throw_on_error(stream->InitializeFromFilename(file.c_str(), GENERIC_WRITE));
      hresult_throw_on_error(instance->Initialize(stream.release(), WICBitmapEncoderNoCache));
    }

    bitmap_encoder(GUID format, std::span<std::byte> buffer)
    {
      auto& factory = bitmap_factory::instance();
      hresult_throw_on_error(factory.CreateEncoder(format, nullptr, instance.put()));
      com_ptr<IWICStream> stream;
      hresult_throw_on_error(bitmap_factory::instance().CreateStream(stream.put()));
      hresult_throw_on_error(stream->InitializeFromMemory((BYTE*)buffer.data(), (DWORD)buffer.size()));
      hresult_throw_on_error(instance->Initialize(stream.release(), WICBitmapEncoderNoCache));
    }

    win32::com::com_ptr<IWICBitmapEncoder> instance;
  };

  class multi_frame_encoder : public bitmap_encoder
  {
  public:
    bool supports_multiple_frames() const override
    {
      return true;
    }

    using bitmap_encoder::bitmap_encoder;
  };

  struct bmp_bitmap_encoder : bitmap_encoder
  {
    bmp_bitmap_encoder(std::filesystem::path file) : bitmap_encoder(GUID_ContainerFormatBmp, std::move(file))
    {
    }

    bmp_bitmap_encoder(std::span<std::byte> buffer) : bitmap_encoder(GUID_ContainerFormatBmp, buffer)
    {
    }
  };

  struct png_bitmap_encoder : bitmap_encoder
  {
    png_bitmap_encoder(std::filesystem::path file) : bitmap_encoder(GUID_ContainerFormatPng, std::move(file))
    {
    }

    png_bitmap_encoder(std::span<std::byte> buffer) : bitmap_encoder(GUID_ContainerFormatPng, buffer)
    {
    }
  };

  struct jpg_bitmap_encoder : bitmap_encoder
  {
    jpg_bitmap_encoder(std::filesystem::path file) : bitmap_encoder(GUID_ContainerFormatJpeg, std::move(file))
    {
    }

    jpg_bitmap_encoder(std::span<std::byte> buffer) : bitmap_encoder(GUID_ContainerFormatJpeg, buffer)
    {
    }
  };

  struct gif_bitmap_encoder : multi_frame_encoder
  {
    gif_bitmap_encoder(std::filesystem::path file) : multi_frame_encoder(GUID_ContainerFormatGif, std::move(file))
    {
    }

    gif_bitmap_encoder(std::span<std::byte> buffer) : multi_frame_encoder(GUID_ContainerFormatGif, buffer)
    {
    }
  };

  struct tiff_bitmap_encoder : multi_frame_encoder
  {
    tiff_bitmap_encoder(std::filesystem::path file) : multi_frame_encoder(GUID_ContainerFormatTiff, std::move(file))
    {
    }

    tiff_bitmap_encoder(std::span<std::byte> buffer) : multi_frame_encoder(GUID_ContainerFormatTiff, buffer)
    {
    }
  };

  struct dds_bitmap_encoder : multi_frame_encoder
  {
    dds_bitmap_encoder(std::filesystem::path file) : multi_frame_encoder(GUID_ContainerFormatDds, std::move(file))
    {
    }

    dds_bitmap_encoder(std::span<std::byte> buffer) : multi_frame_encoder(GUID_ContainerFormatDds, buffer)
    {
    }
  };


  class bitmap_decoder
  {
  public:
    bitmap_decoder(win32::file file, decode_options options = decode_options::WICDecodeMetadataCacheOnDemand)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreateDecoderFromFileHandle((ULONG_PTR)file.get(), nullptr, options, instance.put()));
    }

    bitmap_decoder(std::filesystem::path path, decode_options options = decode_options::WICDecodeMetadataCacheOnDemand)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, options, instance.put()));
    }

    bitmap_decoder(std::span<std::byte> data, bool deep_copy = false, decode_options options = decode_options::WICDecodeMetadataCacheOnDemand)
    {
      auto view = win32::file_view(data.data());

      auto filename = view.GetMappedFilename();
      view.release();

      if (filename)
      {
        hresult_throw_on_error(bitmap_factory::instance().CreateDecoderFromFilename(filename->c_str(), nullptr, GENERIC_READ, options, instance.put()));
      }
      else if (deep_copy)
      {
        com::com_ptr<IStream> mem_stream;
        hresult_throw_on_error(::CreateStreamOnHGlobal(::GlobalAlloc(GMEM_MOVEABLE, data.size()), TRUE, mem_stream.put()));
        hresult_throw_on_error(mem_stream->Write(data.data(), data.size(), nullptr));
        hresult_throw_on_error(mem_stream->Seek(LARGE_INTEGER{}, STREAM_SEEK_SET, nullptr));
        hresult_throw_on_error(bitmap_factory::instance().CreateDecoderFromStream(mem_stream.get(), nullptr, options, instance.put()));
      }
      else
      {
        com::com_ptr<IWICStream> mem_stream;
        hresult_throw_on_error(bitmap_factory::instance().CreateStream(mem_stream.put()));
        hresult_throw_on_error(mem_stream->InitializeFromMemory((BYTE*)data.data(), (DWORD)data.size()));
        hresult_throw_on_error(bitmap_factory::instance().CreateDecoderFromStream(mem_stream.get(), nullptr, options, instance.put()));
      }
    }

    win32::com::com_ptr<IWICBitmapDecoder> handle()
    {
      return instance.as<IWICBitmapDecoder>();
    }

    std::size_t frame_count()
    {
      UINT count;
      hresult_throw_on_error(instance->GetFrameCount(&count));

      return count;
    }

    bitmap_source frame(std::size_t index)
    {
      com::com_ptr<IWICBitmapFrameDecode> frame;
      hresult_throw_on_error(instance->GetFrame((UINT)index, frame.put()));

      return bitmap_source(frame.as<IWICBitmapSource>());
    }

  private:
    win32::com::com_ptr<IWICBitmapDecoder> instance;
  };

  class bitmap : public bitmap_source
  {
  public:
    using bitmap_source::bitmap_source;

    bitmap(bitmap_source source, cache_create_option options = cache_create_option::WICBitmapCacheOnLoad)
    {
      auto handle = source.handle();
      hresult_throw_on_error(bitmap_factory::instance().CreateBitmapFromSource(handle.get(), options, (IWICBitmap**)instance.put()));
    }

    bitmap(std::uint32_t width, std::uint32_t height, pixel_format format, cache_create_option options = cache_create_option::WICBitmapCacheOnLoad)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreateBitmap(width, height, format, options, (IWICBitmap**)instance.put()));
    }

    struct from_memory
    {
      std::uint32_t width;
      std::uint32_t height;
      pixel_format format;
      std::uint32_t stride;
      std::span<std::byte> buffer;
    };

    bitmap(from_memory options)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreateBitmapFromMemory(options.width, options.height, options.format, options.stride, options.buffer.size(), (BYTE*)options.buffer.data(), (IWICBitmap**)instance.put()));
    }

    bitmap(const gdi::icon& icon)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreateBitmapFromHICON(icon, (IWICBitmap**)instance.put()));
    }

    bitmap(gdi::icon_ref icon)
    {
      hresult_throw_on_error(bitmap_factory::instance().CreateBitmapFromHICON(icon.get(), (IWICBitmap**)instance.put()));
    }

    bitmap(const gdi::bitmap& bitmap, alpha_channel_option options) : bitmap((HBITMAP)bitmap, options)
    {
    }

    bitmap(gdi::bitmap_ref bitmap, alpha_channel_option options) : bitmap((HBITMAP)bitmap, options)
    {
    }

    void set_palette(palette& palette)
    {
      auto handle = palette.handle();
      hresult_throw_on_error(((IWICBitmap*)instance.get())->SetPalette(handle.get()));
    }

  private:
    bitmap(HBITMAP bitmap, alpha_channel_option options)
    {
      DIBSECTION section{};
      if (::GetObjectW(bitmap, sizeof(section), &section) > 0 && section.dshSection && (section.dsBm.bmBitsPixel == 32 || section.dsBm.bmBitsPixel == 24))
      {
        auto format = section.dsBm.bmBitsPixel == 32 ? options & alpha_channel_option::WICBitmapUsePremultipliedAlpha ? pixel_format::pbgra_32bpp : pixel_format::bgra_32bpp : pixel_format::bgr_24bpp;
        hresult_throw_on_error(WICCreateBitmapFromSectionEx((UINT)section.dsBm.bmWidth, (UINT)section.dsBm.bmHeight, format, section.dshSection, section.dsBm.bmWidthBytes, section.dsOffset, WICSectionAccessLevelReadWrite, (IWICBitmap**)instance.put()));
      }
      else
      {
        hresult_throw_on_error(bitmap_factory::instance().CreateBitmapFromHBITMAP(bitmap, nullptr, options, (IWICBitmap**)instance.put()));
      }
    }

  public:
    struct from_section
    {
      std::uint32_t width;
      std::uint32_t height;
      pixel_format format;
      HANDLE section;
      std::uint32_t stride;
      std::uint32_t offset;
    };

    bitmap(from_section options)
    {
      hresult_throw_on_error(WICCreateBitmapFromSection(options.width, options.height, options.format, options.section, options.stride, options.offset, (IWICBitmap**)instance.put()));
    }

    struct from_section_ex
    {
      std::uint32_t width;
      std::uint32_t height;
      pixel_format format;
      HANDLE section;
      std::uint32_t stride;
      std::uint32_t offset;
      section_access_level access_level;
    };

    bitmap(from_section_ex options)
    {
      hresult_throw_on_error(WICCreateBitmapFromSectionEx(options.width, options.height, options.format, options.section, options.stride, options.offset, options.access_level, (IWICBitmap**)instance.put()));
    }
  };
}// namespace win32::wic

#endif