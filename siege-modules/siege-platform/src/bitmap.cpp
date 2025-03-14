#include <set>
#include <map>
#include <cmath>
#include <siege/platform/bitmap.hpp>
#include <siege/platform/tagged_data.hpp>
#include <siege/platform/stream.hpp>

namespace siege::platform::bitmap
{
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag bmp_alt1_tag = platform::to_tag<4>({ 'B', 'M', 'P', ' ' });

  constexpr file_tag bmp_alt2_tag = platform::to_tag<4>({ 'B', 'M', 'P', ':' });

  constexpr std::array<std::byte, 2> windows_bmp_tag = { std::byte{ 66 }, std::byte{ 77 } };// BM

  bool is_microsoft_bmp(std::istream& raw_data)
  {
    platform::istream_pos_resetter resetter(raw_data);
    file_tag header{};
    platform::read(raw_data, reinterpret_cast<char*>(&header), sizeof(header));

    return header != bmp_alt1_tag && header != bmp_alt2_tag && header[0] == windows_bmp_tag[0] && header[1] == windows_bmp_tag[1];
  }

  template<typename AlignmentType, typename PixelType>
  [[maybe_unused]] std::size_t read_pixel_data(std::istream& raw_data, std::vector<PixelType>& raw_pixels, std::int32_t width, std::int32_t height, std::int32_t bit_depth)
  {
    const auto x_stride = width * bit_depth / 8;
    const auto padding = platform::get_padding_size(x_stride, sizeof(AlignmentType));

    if (padding == 0)
    {
      platform::read(raw_data, reinterpret_cast<char*>(raw_pixels.data()), raw_pixels.size() * sizeof(PixelType));
      return raw_pixels.size();
    }
    else
    {
      std::vector<std::byte> padding_bytes(padding);

      auto pos = 0u;

      auto start = raw_data.tellg();
      for (auto i = 0; i < height; ++i)
      {
        if (pos > raw_pixels.size())
        {
          break;
        }

        platform::read(raw_data, reinterpret_cast<char*>(raw_pixels.data()) + pos, x_stride);
        platform::read(raw_data, padding_bytes.data(), padding_bytes.size());
        pos += x_stride;
      }

      return raw_data.tellg() - start;
    }
  }

  windows_bmp_data get_bmp_data(std::istream& raw_data, bool auto_flip)
  {
    windows_bmp_header header{};
    platform::read(raw_data, reinterpret_cast<char*>(&header), sizeof(header));

    if (header.tag != windows_bmp_tag)
    {
      throw std::invalid_argument("File data is not BMP based.");
    }

    windows_bmp_info info{};

    platform::read(raw_data, reinterpret_cast<char*>(&info), sizeof(info));

    std::vector<palette::colour> colours;

    const auto num_pixels = info.width.value() * info.height;
    std::vector<std::int32_t> indexes;
    if (info.bit_depth <= 8)
    {
      indexes.reserve(num_pixels);
      int num_colours = static_cast<int>(std::pow(float(2), info.bit_depth));
      colours.reserve(num_colours);

      for (auto i = 0; i < num_colours; ++i)
      {
        std::array<std::byte, 4> quad{};
        platform::read(raw_data, quad.data(), sizeof(quad));
        colours.emplace_back(palette::colour{ quad[2], quad[1], quad[0], std::byte{ 255 } });
      }

      std::vector<std::byte> raw_pixels(num_pixels, std::byte{});

      read_pixel_data<std::int32_t>(raw_data, raw_pixels, info.width, info.height, info.bit_depth);

      std::transform(raw_pixels.begin(), raw_pixels.end(), std::back_inserter(indexes), [](auto value) {
        return static_cast<std::int32_t>(value);
      });
    }
    else if (info.bit_depth == 24)
    {
      colours.reserve(num_pixels);
      std::vector<std::array<std::byte, 3>> image_colours(num_pixels);
      read_pixel_data<std::int32_t>(raw_data, image_colours, info.width, info.height, info.bit_depth);

      std::transform(image_colours.begin(), image_colours.end(), std::back_inserter(colours), [](auto& colour) {
        return palette::colour{ colour[2], colour[1], colour[0], std::byte(0xFF) };
      });
    }

    if (auto_flip)
    {
      if (indexes.empty() && !colours.empty())
      {
        vertical_flip(colours, info.width);
      }
      else if (!indexes.empty())
      {
        vertical_flip(indexes, info.width);
      }
    }

    return {
      header,
      info,
      colours,
      indexes
    };
  }

  void write_bmp_data(std::ostream& raw_data, std::vector<palette::colour> colours, std::vector<std::byte> pixels, std::int32_t width, std::int32_t height, std::int32_t bit_depth, bool auto_flip)
  {
    if (auto_flip)
    {
      if (pixels.empty() && !colours.empty())
      {
        vertical_flip(colours, width);
      }
      else if (!pixels.empty())
      {
        vertical_flip(pixels, width);
      }
    }

    windows_bmp_header header{};
    header.tag = windows_bmp_tag;
    header.reserved1 = 0;
    header.reserved2 = 0;

    if (bit_depth <= 8)
    {
      header.offset = sizeof(header) + sizeof(windows_bmp_info) + int(colours.size()) * sizeof(palette::colour);
    }
    else
    {
      header.offset = sizeof(header) + sizeof(windows_bmp_info);
    }

    windows_bmp_info info{ 0 };
    info.info_size = sizeof(info);
    info.width = width;
    info.height = height;
    info.planes = 1;
    info.bit_depth = bit_depth;
    info.compression = 0;
    info.image_size = width * height;

    using AlignmentType = std::int32_t;
    const auto x_stride = width * bit_depth / 8;
    const auto padding = platform::get_padding_size(x_stride, sizeof(AlignmentType));

    const auto num_pixels = info.width.value() * info.height * (info.bit_depth / 8);


    if (info.bit_depth <= 8 && pixels.size() != num_pixels)
    {
      throw std::invalid_argument("The pixels vector does not have the correct number of pixels.");
    }
    else if (info.bit_depth > 8 && colours.size() != (info.width.value() * info.height))
    {
      throw std::invalid_argument("The colours vector does not have the correct number of pixels.");
    }

    header.file_size = header.offset + num_pixels;

    platform::write(raw_data, reinterpret_cast<const char*>(&header), sizeof(header));
    platform::write(raw_data, reinterpret_cast<const char*>(&info), sizeof(info));

    for (auto& colour : colours)
    {
      std::array<std::byte, 4> quad{ colour.blue, colour.green, colour.red, std::byte{ 0 } };
      platform::write(raw_data, quad.data(), sizeof(quad));
    }

    if (pixels.empty())
    {
      return;
    }

    if (padding == 0)
    {
      platform::write(raw_data, pixels.data(), pixels.size());
    }
    else
    {
      std::vector<std::byte> padding_bytes(padding, std::byte{ 0 });

      auto pos = 0u;

      for (auto i = 0; i < height; ++i)
      {
        if (pos > pixels.size())
        {
          break;
        }

        platform::write(raw_data, pixels.data() + pos, x_stride);
        platform::write(raw_data, padding_bytes.data(), padding_bytes.size());
        pos += x_stride;
      }
    }
  }

  template<typename IndexType>
  std::vector<IndexType> remap_bitmap(const std::vector<IndexType>& pixels,
    const std::vector<palette::colour>& original_colours,
    const std::vector<palette::colour>& other_colours,
    bool only_unique)
  {
    if (pixels.empty())
    {
      return pixels;
    }

    if (original_colours == other_colours)
    {
      return pixels;
    }

    std::vector<IndexType> results;
    results.reserve(pixels.size());
    std::map<std::size_t, std::size_t> palette_map;

    for (auto x = 0u; x < original_colours.size(); x++)
    {
      auto& colour = original_colours[x];
      auto result = std::find_if(other_colours.begin(), other_colours.end(), [&](auto& other) {
        return other.red == colour.red && other.green == colour.green && other.blue == colour.blue;
      });

      if (result != other_colours.end())
      {
        auto other_index = std::distance(other_colours.begin(), result);
        palette_map[x] = other_index;
      }
      else
      {
        std::map<double, std::size_t> distances;
        for (auto y = 0u; y < other_colours.size(); y++)
        {
          auto& other = other_colours[y];
          auto distance = palette::colour_distance(colour, other);
          distances[distance] = y;
        }

        if (only_unique)
        {
          for (auto distance : distances)
          {
            auto already_exists = std::find_if(palette_map.begin(), palette_map.end(), [&](auto& elem) {
              return elem.second == distance.second;
            });

            if (already_exists == palette_map.end())
            {
              palette_map[x] = distance.second;
              break;
            }
          }

          if (auto inserted = palette_map.find(x); inserted == palette_map.end())
          {
            palette_map[x] = distances.begin()->second;
          }
        }
        else
        {
          palette_map[x] = distances.begin()->second;
        }
      }
    }

    for (auto pixel : pixels)
    {
      auto new_index = palette_map[std::size_t(pixel)];
      results.emplace_back(IndexType(new_index));
    }

    return results;
  }

  std::vector<std::byte> remap_bitmap(const std::vector<std::byte>& pixels,
    const std::vector<palette::colour>& original_colours,
    const std::vector<palette::colour>& other_colours,
    bool only_unique)
  {
    return remap_bitmap<std::byte>(pixels, original_colours, other_colours, only_unique);
  }


  std::vector<std::int32_t> remap_bitmap(const std::vector<std::int32_t>& pixels,
    const std::vector<palette::colour>& original_colours,
    const std::vector<palette::colour>& other_colours,
    bool only_unique)
  {
    return remap_bitmap<std::int32_t>(pixels, original_colours, other_colours, only_unique);
  }
}// namespace siege::platform::bitmap
