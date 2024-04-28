#include "bmp_controller.hpp"
#include <siege/content/bmp/bitmap.hpp>
#include <siege/content/bmp/image.hpp>
#include <spanstream>
#include <psapi.h>
#include <wincodec.h>

namespace siege::views
{
  bool bmp_controller::is_bmp(std::istream& image_stream) noexcept
  {
      return siege::content::bmp::is_earthsiege_bmp(image_stream)
           || siege::content::bmp::is_microsoft_bmp(image_stream)
          || siege::content::bmp::is_phoenix_bmp(image_stream)
          || siege::content::bmp::is_jpg(image_stream)
          || siege::content::bmp::is_png(image_stream);
  }

  std::size_t bmp_controller::load_bitmap(std::istream& image_stream) noexcept
  {
      using namespace siege::content;

      auto start = image_stream.tellg();

      try
      {
          if (bmp::is_microsoft_bmp(image_stream))
          {
                original_images.emplace_back(bmp::get_bmp_data(image_stream));
                return 1;        
       

          }
          else if (bmp::is_phoenix_bmp(image_stream))
          {
          }
          else if (bmp::is_earthsiege_bmp(image_stream))
          {
          }
          else if (std::spanstream* span_stream = dynamic_cast<std::spanstream*>(&image_stream); span_stream != nullptr)
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

            thread_local std::unique_ptr<IWICImagingFactory, void(*)(IWICImagingFactory*)> factory = [] {
           
                IWICImagingFactory* temp = nullptr;

                if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&temp)) != S_OK)
                {
                    throw std::exception("Could not create imaging factory");
                }

                return std::unique_ptr<IWICImagingFactory, void(*)(IWICImagingFactory*)>(temp, [](auto* self) { self->Release(); } );
            }();

            IWICBitmapDecoder *decoder = nullptr;
       
            auto hresult = factory->CreateDecoderFromFilename(
                filename.c_str(),            
                nullptr,        
                GENERIC_READ,   
                WICDecodeMetadataCacheOnDemand, 
                &decoder
            );

         
            std::uint32_t count = 0;
            decoder->GetFrameCount(&count);

            for (auto i = 0u; i < count; ++i)
            {
                IWICBitmapFrameDecode *frame = nullptr;
                assert(decoder->GetFrame(i, &frame) == S_OK);

                IWICFormatConverter *converter = nullptr;

                assert(factory->CreateFormatConverter(&converter) == S_OK);

                converter->Initialize(
                    frame,                         // Input bitmap to convert
                    GUID_WICPixelFormat32bppBGR,     // Destination pixel format
                    WICBitmapDitherTypeNone,         // Specified dither patterm
                    NULL,                            // Specify a particular palette 
                    0.f,                             // Alpha threshold
                    WICBitmapPaletteTypeCustom       // Palette translation type
                    );

                std::uint32_t width = 0;
                std::uint32_t height = 0;

                assert(converter->GetSize(&width, &height) == S_OK);

                BITMAPINFO info{
                    .bmiHeader {
                        .biSize = sizeof(BITMAPINFOHEADER),
                        .biWidth = LONG(width),
                        .biHeight = LONG(height),
                        .biPlanes = 1,
                        .biBitCount = 32,
                        .biCompression = BI_RGB
                    }
                };
                auto wnd_dc = ::GetDC(nullptr);

                void* pixels = nullptr;
                auto handle = ::CreateDIBSection(wnd_dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);

                assert(converter->CopyPixels(nullptr, width * sizeof(std::int32_t), height * width * sizeof(std::int32_t), reinterpret_cast<BYTE*>(pixels)) == S_OK);

                converter->Release();
                frame->Release();

                original_images.emplace_back(handle);
            }
            decoder->Release();

            return count;
          }
      }
      catch(std::exception& ex)
      {
        OutputDebugStringA(ex.what());
      }

     return 0;
  }

  content::bmp::platform_bitmap& bmp_controller::get_bitmap(std::size_t index) noexcept
  {
    assert(original_images.size() > 0);

    return original_images[index];
  }
}