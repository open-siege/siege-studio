#include <siege/content/bmp/image.hpp>
#include <siege/platform/win/core/com.hpp>
#include <exception>
#include <spanstream>
#include <memory>
#include <cassert>
#include <psapi.h>
#include <wincodec.h>

namespace siege::content::bmp
{
    using wic_bitmap = win32::com::com_ptr<IWICBitmapSource>;

    auto& bitmap_factory()
    {
        thread_local std::unique_ptr<IWICImagingFactory, void(*)(IWICImagingFactory*)> factory = [] {
           
                    IWICImagingFactory* temp = nullptr;

                    if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&temp)) != S_OK)
                    {
                        throw std::exception("Could not create imaging factory");
                    }

                    return std::unique_ptr<IWICImagingFactory, void(*)(IWICImagingFactory*)>(temp, [](auto* self) { self->Release(); } );
                }();

        return *factory;
    }

    platform_image::platform_image(std::span<windows_bmp_data> bitmaps)
    {
        frames.reserve(bitmaps.size());

        std::transform(bitmaps.begin(), bitmaps.end(), std::back_inserter(frames), [&](auto& value) {
            return std::move(value);    
        });
    }

    platform_image::platform_image(windows_bmp_data bitmap)
    {
        frames.emplace_back(std::move(bitmap));
    }

    platform_image::platform_image(std::filesystem::path filename)
    {
        load(std::move(filename));
    }

    platform_image::platform_image(std::istream& image_stream)
    {
        if (std::spanstream* span_stream = dynamic_cast<std::spanstream*>(&image_stream); span_stream != nullptr)
        {
            auto span = span_stream->rdbuf()->span();
            std::wstring filename(255, L'\0');
            auto size = ::GetMappedFileNameW(::GetCurrentProcess(), span.data(), filename.data(), filename.size());

            if (size == 0)
            {
                 throw std::invalid_argument("stream");
            }

            std::wstring drive = L"A:";

            std::wstring buffer(32, L'\0');

            for (auto i = drive[0]; i <= L'Z'; ++i)
            {
                    drive[0] = i;

                    auto vol_size = ::QueryDosDeviceW(drive.c_str(), buffer.data(), buffer.size());

                    if (vol_size != 0)
                    {
                       buffer = buffer.c_str();

                       auto index = filename.find(buffer, 0);

                       if (index == 0)
                       {
                           filename = filename.replace(0, buffer.size(), drive);              
                           break;
                       }
                    }
            }

            load(filename.c_str());
        } 
    }

    void platform_image::load(std::filesystem::path filename)
    {
        win32::com::com_ptr<IWICBitmapDecoder> decoder(nullptr);
      
        auto hresult = bitmap_factory().CreateDecoderFromFilename(
                filename.c_str(),            
                nullptr,        
                GENERIC_READ,   
                WICDecodeMetadataCacheOnDemand, 
                decoder.put()
            );

         
        std::uint32_t count = 0;
        decoder->GetFrameCount(&count);

        for (auto i = 0u; i < count; ++i)
        {
            win32::com::com_ptr<IWICBitmapFrameDecode> frame;

            if (decoder->GetFrame(i, frame.put()) == S_OK)
            {
                frames.emplace_back(wic_bitmap(frame.as<IWICBitmapSource>()));            
            }
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

        if (item.type().hash_code() == typeid(wic_bitmap).hash_code())
        {
            const auto& bitmap = std::any_cast<const wic_bitmap&>(item);
            
            IWICBitmapScaler *scaler = nullptr;
            assert(bitmap_factory().CreateBitmapScaler(&scaler) == S_OK);
            scaler->Initialize(bitmap.get(), size.first, size.second, WICBitmapInterpolationModeFant);

            IWICFormatConverter *converter = nullptr;
            assert(bitmap_factory().CreateFormatConverter(&converter) == S_OK);
            converter->Initialize(
                        scaler,             
                        GUID_WICPixelFormat32bppBGR, 
                        WICBitmapDitherTypeNone, 
                        static_cast<IWICPalette*>(nullptr),
                        0.f,    
                        WICBitmapPaletteTypeCustom
            );

            std::uint32_t width = 0;
            std::uint32_t height = 0;

            assert(converter->GetSize(&width, &height) == S_OK);
            assert(converter->CopyPixels(nullptr, width * sizeof(std::int32_t), height * width * sizeof(std::int32_t), reinterpret_cast<BYTE*>(pixels.data())) == S_OK);

            scaler->Release();
            converter->Release();

            return height * width * sizeof(int32_t);
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
            
            }
            
            // TODO either copy or scale CreateBitmapFromMemory

        }


        return 0;                
    }

}