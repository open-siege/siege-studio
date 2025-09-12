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

  std::vector<win32::wic::bitmap_source> load(win32::wic::bitmap_decoder& decoder)
  {
    auto count = decoder.frame_count();

    std::vector<win32::wic::bitmap_source> frames;
    frames.reserve(count);

    for (auto i = 0u; i < count; ++i)
    {
      frames.emplace_back(decoder.frame(i));
    }

    return frames;
  }

  std::vector<win32::wic::bitmap_source> load(std::span<std::byte> bytes, bool deep_copy)
  {
    win32::wic::bitmap_decoder decoder(bytes, deep_copy);
    return load(decoder);
  }

  std::vector<win32::wic::bitmap_source> load(std::filesystem::path filename)
  {
    win32::wic::bitmap_decoder decoder(filename);
    return load(decoder);
  }

  win32::wic::bitmap_source convert(const windows_bmp_data& bitmap)
  {
    if (!bitmap.indexes.empty() && !bitmap.colours.empty())
    {
      std::vector<win32::color> colors;
      colors.resize(bitmap.colours.size());

      std::transform(bitmap.colours.begin(), bitmap.colours.end(), colors.begin(), [](auto color) {
        return RGBQUAD{ .rgbBlue = (BYTE)color.blue, .rgbGreen = (BYTE)color.green, .rgbRed = (BYTE)color.red, .rgbReserved = (BYTE)color.flags };
      });

      win32::wic::palette palette(colors);

      std::vector<std::byte> indexes;
      indexes.resize(bitmap.indexes.size());

      std::transform(bitmap.indexes.begin(), bitmap.indexes.end(), indexes.begin(), [](auto index) {
        return static_cast<std::byte>(static_cast<std::uint8_t>(index));
      });
      win32::wic::bitmap result(win32::wic::bitmap::from_memory{
        .width = bitmap.info.width,
        .height = bitmap.info.height,
        .format = win32::wic::pixel_format::indexed_8bpp,
        .stride = bitmap.info.width,
        .buffer = indexes });
      result.set_palette(palette);

      return result;
    }
    else if (!bitmap.colours.empty() && bitmap.info.bit_depth == 32)
    {
      win32::wic::bitmap result(win32::wic::bitmap::from_memory{
        .width = bitmap.info.width,
        .height = bitmap.info.height,
        .format = win32::wic::pixel_format::bgra_32bpp,
        .stride = bitmap.info.width * sizeof(std::uint32_t),
        .buffer = std::span<std::byte>((std::byte*)bitmap.colours.data(), bitmap.info.width * sizeof(std::uint32_t) * bitmap.info.height) });
      return result;
    }

    throw std::invalid_argument("Bad bitmap format provided");
  }

  platform_image::platform_image(std::span<windows_bmp_data> bitmaps)
  {
    frames.reserve(bitmaps.size());

    for (auto& bitmap : bitmaps)
    {
      frames.emplace_back(convert(bitmap));
    }
  }

  platform_image::platform_image(windows_bmp_data bitmap)
  {
    frames.emplace_back(convert(bitmap));
  }

  platform_image::platform_image(std::filesystem::path filename)
  {
    frames = load(std::move(filename));
  }

  platform_image::platform_image(std::istream& image_stream, bool deep_copy)
  {
    if (std::spanstream* span_stream = dynamic_cast<std::spanstream*>(&image_stream); span_stream != nullptr)
    {
      auto span = span_stream->rdbuf()->span();

      frames = load(std::span<std::byte>((std::byte*)span.data(), span.size()), deep_copy);
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

    frames = load(data, deep_copy);
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
    auto temp = item.get_size();
    return size((int)temp.cx, (int)temp.cy);
  }

  std::size_t platform_image::frame_count() const noexcept
  {
    return frames.size();
  }
}// namespace siege::platform::bitmap