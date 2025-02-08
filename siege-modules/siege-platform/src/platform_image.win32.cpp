#include <siege/platform/image.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/win/com.hpp>
#include <siege/platform/win/file.hpp>
#include <exception>
#include <spanstream>
#include <memory>

#undef NDEBUG
#include <cassert>
#include <siege/platform/win/wic.hpp>
#include <VersionHelpers.h>

namespace siege::platform::bitmap
{
  using windows_bmp_data = siege::platform::bitmap::windows_bmp_data;

  void apply_palette(windows_bmp_data& bitmap)
  {
    if (!bitmap.indexes.empty() && !bitmap.colours.empty())
    {
      std::vector<platform::palette::colour> colours(bitmap.indexes.size());

      std::transform(bitmap.indexes.begin(), bitmap.indexes.end(), colours.begin(), [&](auto index) {
        return bitmap.colours.at(index);
      });

      bitmap.colours = colours;
      bitmap.indexes.clear();
    }
  }

  std::vector<std::any> load(win32::wic::bitmap_decoder& decoder)
  {
    auto count = decoder.frame_count();

    std::vector<std::any> frames;
    frames.reserve(count);

    for (auto i = 0u; i < count; ++i)
    {
      frames.emplace_back(decoder.frame(i));
    }

    return frames;
  }

  std::vector<std::any> load(std::span<std::byte> bytes)
  {
    win32::wic::bitmap_decoder decoder(bytes);
    return load(decoder);
  }

  std::vector<std::any> load(std::filesystem::path filename)
  {
    win32::wic::bitmap_decoder decoder(filename);
    return load(decoder);
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

      frames = load(std::span<std::byte>((std::byte*)span.data(), span.size()));
      return;
    }

    auto filename = siege::platform::get_stream_path(image_stream);

    if (filename)
    {
      frames = load(std::move(*filename));
      return;
    }

    auto size = siege::platform::get_stream_size(image_stream);

    std::vector<std::byte> data(size);

    auto pos = image_stream.tellg();
    image_stream.read((char*)data.data(), size);
    image_stream.seekg(pos, std::ios::beg);

    frames = load(data);
  }

  size platform_image::get_size(std::size_t frame) const noexcept
  {
    if (frames.empty())
    {
      return {};
    }

    if (frames.size() < frame)
    {
      return {};
    }

    const auto& item = frames[frame];
    if (item.type().hash_code() == typeid(win32::wic::bitmap).hash_code())
    {
      auto temp = std::any_cast<const win32::wic::bitmap&>(item).get_size();
      return size((int)temp.cx, (int)temp.cy);
    }
    else if (item.type().hash_code() == typeid(win32::wic::bitmap_source).hash_code())
    {
      auto temp = std::any_cast<const win32::wic::bitmap_source&>(item).get_size();
      return size((int)temp.cx, (int)temp.cy);
    }
    else if (item.type().hash_code() == typeid(windows_bmp_data).hash_code())
    {
      const auto& bitmap = std::any_cast<const windows_bmp_data&>(item);

      return size((int)bitmap.info.width, (int)bitmap.info.height);
    }

    return {};
  }

  std::size_t platform_image::frame_count() const noexcept
  {
    return frames.size();
  }

  std::size_t platform_image::convert(std::size_t frame, size size, int bits, std::span<std::byte> pixels) const noexcept
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

    auto final_result = size.width * size.height * bits / 8;

    if (pixels.size() < final_result)
    {
      return 0;
    }

    const auto& item = frames[frame];

    auto scale_bitmap = [size, pixels](const win32::wic::bitmap_source& bitmap) {
      auto mode = WICBitmapInterpolationModeFant;

      if (IsWindows10OrGreater())
      {
        mode = WICBitmapInterpolationModeHighQualityCubic;
      }

      auto source = bitmap.scale((std::uint32_t)size.width, (std::uint32_t)size.height, mode)
                      .convert(win32::wic::bitmap::to_format{
                        .format = GUID_WICPixelFormat32bppBGR });

      auto size = source.get_size();
      source.copy_pixels(size.cx * sizeof(std::int32_t), pixels);

      return size.cx * size.cy * sizeof(int32_t);
    };

    if (item.type().hash_code() == typeid(win32::wic::bitmap).hash_code())
    {
      return scale_bitmap(std::any_cast<const win32::wic::bitmap&>(item));
    }
    else if (item.type().hash_code() == typeid(win32::wic::bitmap_source).hash_code())
    {
      return scale_bitmap(std::any_cast<const win32::wic::bitmap_source&>(item));
    }
    else if (item.type().hash_code() == typeid(windows_bmp_data).hash_code())
    {
      const auto& bitmap = std::any_cast<const windows_bmp_data&>(item);

      if (bitmap.indexes.empty() && !bitmap.colours.empty() && bitmap.info.bit_depth == bits && bitmap.info.width == size.width && bitmap.info.height == size.height)
      {
        std::memcpy(pixels.data(), bitmap.colours.data(), sizeof(platform::palette::colour) * bitmap.colours.size());
        return sizeof(platform::palette::colour) * bitmap.colours.size();
      }
      else if (bitmap.indexes.empty() && !bitmap.colours.empty())
      {
        return scale_bitmap(win32::wic::bitmap(win32::wic::bitmap::from_memory{
          .width = bitmap.info.width,
          .height = bitmap.info.height,
          .format = GUID_WICPixelFormat32bppRGB,
          .stride = (std::uint32_t)bitmap.info.width * 4,
          .buffer = std::span<std::byte>(reinterpret_cast<std::byte*>(const_cast<platform::palette::colour*>(bitmap.colours.data())), bitmap.info.width * 4 * bitmap.info.height) }));
      }
    }

    return 0;
  }

}// namespace siege::platform::bitmap