#include <siege/content/bmp/image.hpp>
#include <siege/platform/win/core/com/base.hpp>
#include <siege/platform/win/core/file.hpp>
#include <exception>
#include <spanstream>
#include <memory>
#undef NDEBUG 
#include <cassert>
#include <wincodec.h>
#include <VersionHelpers.h>

namespace siege::content::bmp
{
    using wic_bitmap = win32::com::com_ptr<IWICBitmapSource>;

    auto& bitmap_factory()
    {
        thread_local win32::com::com_ptr factory = [] 
        {
            win32::com::com_ptr<IWICImagingFactory> temp;

            if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), temp.put_void()) != S_OK)
            {
                throw std::exception("Could not create imaging factory");
            }

            return temp;
        }();

        return *factory;
    }

    void apply_palette(windows_bmp_data& bitmap)
    {
        if (!bitmap.indexes.empty() && !bitmap.colours.empty())
        {
            std::vector<pal::colour> colours(bitmap.indexes.size());

            std::transform(bitmap.indexes.begin(), bitmap.indexes.end(), colours.begin(), [&](auto index) {
                return bitmap.colours.at(index);
            });

            bitmap.colours = colours;
            bitmap.indexes.clear();
        }
    }

    std::vector<std::any> load(std::filesystem::path filename)
    {
        win32::com::com_ptr<IWICBitmapDecoder> decoder;
      
        auto hresult = bitmap_factory().CreateDecoderFromFilename(
                filename.c_str(),            
                nullptr,        
                GENERIC_READ,   
                WICDecodeMetadataCacheOnDemand, 
                decoder.put()
            );

         
        std::uint32_t count = 0;
        decoder->GetFrameCount(&count);


        std::vector<std::any> frames;
        frames.reserve(count);

        for (auto i = 0u; i < count; ++i)
        {
            win32::com::com_ptr<IWICBitmapFrameDecode> frame;

            if (decoder->GetFrame(i, frame.put()) == S_OK)
            {
                frames.emplace_back(wic_bitmap(frame.as<IWICBitmapSource>()));            
            }
        }

        return frames;
    }

    platform_image::platform_image(std::span<windows_bmp_data> bitmaps)
    {
        frames.reserve(bitmaps.size());

        std::transform(bitmaps.begin(), bitmaps.end(), std::back_inserter(frames), [&](auto& value) {
            apply_palette(value);
            return std::move(value);    
        });
    }

    platform_image::platform_image(windows_bmp_data bitmap)
    {
        apply_palette(bitmap);

        frames.emplace_back(std::move(bitmap));
    }

    platform_image::platform_image(std::filesystem::path filename)
    {
        frames = load(std::move(filename));
    }

    platform_image::platform_image(std::istream& image_stream)
    {
        if (std::spanstream* span_stream = dynamic_cast<std::spanstream*>(&image_stream); span_stream != nullptr)
        {
            auto span = span_stream->rdbuf()->span();
            auto view = win32::file_view(span.data());

            auto filename = view.GetMappedFilename();
            view.release();

            if (!filename)
            {
                 throw std::invalid_argument("stream");
            }

            frames = load(std::move(*filename));
        } 
    }

    std::size_t platform_image::frame_count() const
    {
        return frames.size();
    }

    std::size_t platform_image::convert(std::size_t frame, std::pair<int, int> size, int bits, std::span<std::byte> pixels) const noexcept
    {
        if (frames.empty())
        {
            return 0;
        }

        if (frames.size() < frame)
        {
            return 0;
        }

        if (bits != 32)
        {
            return 0;
        }

        auto final_result = size.first * size.second * bits / 8;

        if (pixels.size() < final_result)
        {
            return 0;
        }

        const auto& item = frames[frame];

        auto scale_bitmap = [size, pixels](const wic_bitmap& bitmap) {
            win32::com::com_ptr<IWICBitmapScaler> scaler;
            assert(bitmap_factory().CreateBitmapScaler(scaler.put()) == S_OK);
            if (IsWindows10OrGreater())
            {
              scaler->Initialize(bitmap.get(), size.first, size.second, WICBitmapInterpolationModeHighQualityCubic);
            }
            else
            {
              scaler->Initialize(bitmap.get(), size.first, size.second, WICBitmapInterpolationModeFant);
            }

            win32::com::com_ptr<IWICFormatConverter> converter;
            assert(bitmap_factory().CreateFormatConverter(converter.put()) == S_OK);
            converter->Initialize(
                        scaler.get(),             
                        GUID_WICPixelFormat32bppBGR, 
                        WICBitmapDitherTypeNone, 
                        win32::com::com_ptr<IWICPalette>().get(),
                        0.f,    
                        WICBitmapPaletteTypeCustom
            );

            std::uint32_t width = 0;
            std::uint32_t height = 0;

            assert(converter->GetSize(&width, &height) == S_OK);
            assert(converter->CopyPixels(nullptr, width * sizeof(std::int32_t), height * width * sizeof(std::int32_t), reinterpret_cast<BYTE*>(pixels.data())) == S_OK);

            return height * width * sizeof(int32_t);
        };

        if (item.type().hash_code() == typeid(wic_bitmap).hash_code())
        {
            return scale_bitmap(std::any_cast<const wic_bitmap&>(item));   
        }
        else if (item.type().hash_code() == typeid(windows_bmp_data).hash_code())
        {
            const auto& bitmap = std::any_cast<const windows_bmp_data&>(item);
            
            if (bitmap.indexes.empty() && !bitmap.colours.empty() && 
                bitmap.info.bit_depth == bits && 
                bitmap.info.width == size.first &&
                bitmap.info.height == size.second)
            {
                std::memcpy(pixels.data(), bitmap.colours.data(), sizeof(pal::colour) * bitmap.colours.size());
                return sizeof(pal::colour) * bitmap.colours.size();
            }
            else if (bitmap.indexes.empty() && !bitmap.colours.empty())
            {
                win32::com::com_ptr<IWICBitmap> wic_bitmap;

                assert(bitmap_factory().CreateBitmapFromMemory(bitmap.info.width, 
                            bitmap.info.height, 
                            GUID_WICPixelFormat32bppRGB, 
                            bitmap.info.width * 4,
                            bitmap.info.width * 4 * bitmap.info.height,
                            reinterpret_cast<BYTE*>(const_cast<pal::colour*>(bitmap.colours.data())),
                            wic_bitmap.put()
                    ) == S_OK);
            
                return scale_bitmap(wic_bitmap.as<IWICBitmapSource>());
            }    
        }

        return 0;                
    }

}