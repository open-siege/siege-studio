#ifndef DARKSTARDTSCONVERTER_BITMAP_HPP
#define DARKSTARDTSCONVERTER_BITMAP_HPP

#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include "palette.hpp"
#include "endian_arithmetic.hpp"

namespace darkstar::bmp
{
  namespace endian = boost::endian;
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag to_tag(const std::array<std::uint8_t, 4> values)
  {
    file_tag result{};

    for (auto i = 0u; i < values.size(); i++)
    {
      result[i] = std::byte{ values[i] };
    }
    return result;
  }

  constexpr file_tag pbmp_tag = to_tag({ 'P', 'B', 'M', 'P' });

  constexpr file_tag pba_tag = to_tag({ 'P', 'B', 'M', 'A' });

  constexpr file_tag header_tag = to_tag({ 'h', 'e', 'a', 'd' });

  constexpr file_tag data_tag = to_tag({ 'd', 'a', 't', 'a' });

  constexpr file_tag detail_tag = to_tag({ 'D', 'E', 'T', 'L' });

  constexpr file_tag palette_tag = to_tag({ 'P', 'i', 'D', 'X' });

  constexpr std::array<std::byte, 2> windows_bmp_tag = { std::byte{66}, std::byte{77}}; // BM

  constexpr std::array<std::byte, 2> special_reserved_tag = { std::byte{0xF7}, std::byte{0xF5}}; // for palettes

  struct pbmp_header
  {
    endian::little_uint32_t version;
    endian::little_int32_t width;
    endian::little_int32_t height;
    endian::little_uint32_t bit_depth;
    endian::little_uint32_t flags;
  };

  struct windows_bmp_header
  {
    std::array<std::byte, 2> tag;
    endian::little_uint32_t file_size;
    endian::little_uint16_t reserved1;
    endian::little_uint16_t reserved2;
    endian::little_uint32_t offset;
  };

  struct windows_bmp_info
  {
    endian::little_uint32_t info_size;
    endian::little_int32_t width;
    endian::little_int32_t height;
    endian::little_uint16_t planes;
    endian::little_uint16_t bit_depth;
    endian::little_uint32_t compression;
    endian::little_uint32_t image_size;
    endian::little_int32_t x_pixels_per_metre;
    endian::little_int32_t y_pixels_per_metre;
    endian::little_uint32_t num_colours_used;
    endian::little_uint32_t num_important_colours;
  };

  struct pbmp_data
  {
    pbmp_header bmp_header;
    endian::little_uint32_t detail_levels;
    endian::little_uint32_t palette_index;
    std::vector<std::byte> pixels;
  };

  struct windows_bmp_data
  {
    windows_bmp_header header;
    windows_bmp_info info;
    std::vector<pal::colour> colours;
    std::vector<std::byte> pixels;
  };

  windows_bmp_data get_bmp_data(std::basic_ifstream<std::byte>& raw_data)
  {
    windows_bmp_header header{};
    raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

    if (header.tag != windows_bmp_tag)
    {
      throw std::invalid_argument("File data is not BMP based.");
    }

    windows_bmp_info info{};

    raw_data.read(reinterpret_cast<std::byte*>(&info), sizeof(info));

    int num_colours = 0;

    if (info.bit_depth == 4)
    {
      num_colours = 16;
    }
    else if (info.bit_depth == 8)
    {
      num_colours = 256;
    }

    std::vector<pal::colour> colours;
    colours.reserve(num_colours);

    for (auto i = 0; i < num_colours; ++i)
    {
      std::array<std::byte, 4> quad{};
      raw_data.read(quad.data(), sizeof(quad));
      colours.emplace_back(pal::colour{quad[2], quad[1], quad[0], quad[3]});
    }

    const auto num_pixels = info.width * info.height * (info.bit_depth / 8);

    std::vector<std::byte> pixels(num_pixels, std::byte{});

    raw_data.read(pixels.data(), pixels.size());

    return {
      header,
      info,
      colours,
      pixels
    };
  }

  void write_bmp_data(std::basic_ofstream<std::byte>& raw_data, std::int32_t width, std::int32_t height, const std::vector<pal::colour>& colours, const std::vector<std::byte>& pixels)
  {
    windows_bmp_header header{};
    header.tag = windows_bmp_tag;
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.offset = sizeof(header) + sizeof(windows_bmp_info) + int(colours.size()) * sizeof(pal::colour);

    windows_bmp_info info{0};
    info.info_size = sizeof(info);
    info.width = width;
    info.height = height;
    info.planes = 1;
    info.bit_depth = 8;
    info.compression = 0;
    info.image_size = width * height;

    const auto num_pixels = info.width * info.height * (info.bit_depth / 8);

    if (pixels.size() != num_pixels)
    {
      throw std::invalid_argument("The pixels vector does not have the correct number of pixels.");
    }

    header.file_size = header.offset + num_pixels;

    raw_data.write(reinterpret_cast<std::byte*>(&header), sizeof(header));
    raw_data.write(reinterpret_cast<std::byte*>(&info), sizeof(info));

    for (auto& colour : colours)
    {
      std::array<std::byte, 4> quad{ colour.blue, colour.green, colour.red, colour.flags };
      raw_data.write(quad.data(), sizeof(quad));
    }

    raw_data.write(pixels.data(), pixels.size());
  }

  pbmp_data get_pbmp_data(std::basic_ifstream<std::byte>& raw_data)
  {
    const auto start = std::size_t(raw_data.tellg());
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};

    raw_data.read(header.data(), sizeof(header));
    raw_data.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));

    if (header != pbmp_tag)
    {
      throw std::invalid_argument("File data is not PBMP based.");
    }

    pbmp_header bmp_header{};
    endian::little_uint32_t detail_levels{};
    endian::little_uint32_t palette_index{};
    std::vector<std::byte> pixels;

    auto end = start + file_size + sizeof(header) + sizeof(file_size) + sizeof(std::int32_t) + sizeof(std::array<std::int32_t, 6>);

    while (std::size_t(raw_data.tellg()) < end)
    {
      std::array<std::byte, 4> chunk_header{};
      endian::little_uint32_t chunk_size{};

      raw_data.read(chunk_header.data(), chunk_header.size());
      raw_data.read(reinterpret_cast<std::byte*>(&chunk_size), sizeof(chunk_size));

      if (chunk_header == header_tag)
      {
        raw_data.read(reinterpret_cast<std::byte*>(&bmp_header), sizeof(bmp_header));

        const auto num_pixels = bmp_header.width * bmp_header.height * (bmp_header.bit_depth / 8);
        pixels = std::vector<std::byte>(num_pixels, std::byte{});
      }
      else if (chunk_header == data_tag)
      {
        raw_data.read(pixels.data(), pixels.size());

        // PBMP files contain mip maps of the main image.
        // For now, we won't worry about them.
        raw_data.seekg(chunk_size - pixels.size(), std::ios::cur);
      }
      else if (chunk_header == detail_tag)
      {
        raw_data.read(reinterpret_cast<std::byte*>(&detail_levels), sizeof(detail_levels));
      }
      else if (chunk_header == palette_tag)
      {
        raw_data.read(reinterpret_cast<std::byte*>(&palette_index), sizeof(palette_index));
      }
      else
      {
        if (chunk_size == 0)
        {
          break;
        }
        raw_data.seekg(chunk_size, std::ios::cur);
      }
    }

    return {
      bmp_header,
      detail_levels,
      palette_index,
      pixels
    };
  }

  void write_pbmp_data(std::basic_ofstream<std::byte>& raw_data, std::int32_t width, std::int32_t height, const std::vector<pal::colour>& colours, const std::vector<std::byte>& pixels)
  {
    raw_data.write(pbmp_tag.data(), sizeof(pbmp_tag));

    auto file_size_pos = raw_data.tellp();

    endian::little_int32_t file_size = 0;
    raw_data.write(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));

    raw_data.write(header_tag.data(), sizeof(header_tag));
    endian::little_int32_t header_size = sizeof(pbmp_header);
    raw_data.write(reinterpret_cast<std::byte*>(&header_size), sizeof(header_size));
    pbmp_header header{};
    header.version = 3;
    header.width = width;
    header.height = height;
    header.bit_depth = 8;
    header.flags = 8;

    raw_data.write(reinterpret_cast<std::byte*>(&header), sizeof(header));

    auto pal_bytes = pal::write_pal_data(raw_data, colours);

    raw_data.write(data_tag.data(), sizeof(data_tag));
    header_size = std::int32_t(pixels.size());
    raw_data.write(reinterpret_cast<std::byte*>(&header_size), sizeof(header_size));
    raw_data.write(pixels.data(), pixels.size());

    raw_data.write(detail_tag.data(), sizeof(detail_tag));
    header_size = sizeof(std::int32_t);
    raw_data.write(reinterpret_cast<std::byte*>(&header_size), sizeof(header_size));
    endian::little_int32_t num_details = 1;
    raw_data.write(reinterpret_cast<std::byte*>(&num_details), sizeof(num_details));

    raw_data.write(palette_tag.data(), sizeof(palette_tag));
    header_size = sizeof(std::int32_t);
    raw_data.write(reinterpret_cast<std::byte*>(&header_size), sizeof(header_size));
    endian::little_int32_t palette_index = 1;
    raw_data.write(reinterpret_cast<std::byte*>(&palette_index), sizeof(palette_index));

    raw_data.seekp(file_size_pos, std::ios::beg);

    file_size = std::int32_t(sizeof(std::int32_t) + sizeof(pbmp_header) + pal_bytes + sizeof(std::array<std::int32_t, 2>) + pixels.size());
    raw_data.write(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));
  }

  std::vector<pbmp_data> get_pba_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::vector<pbmp_data> results;

    std::array<std::byte, 4> header{};
    endian::little_uint32_t count{};

    raw_data.read(header.data(), sizeof(header));
    raw_data.read(reinterpret_cast<std::byte*>(&count), sizeof(count));

    if (header != pba_tag)
    {
      throw std::invalid_argument("File data is not PBA based.");
    }

    raw_data.read(header.data(), sizeof(header));

    raw_data.read(reinterpret_cast<std::byte*>(&count), sizeof(count));

    // Count appears twice in the files. Or it means something else. Haven't seen a file where it is something else.
    raw_data.read(reinterpret_cast<std::byte*>(&count), sizeof(count));
    raw_data.read(reinterpret_cast<std::byte*>(&count), sizeof(count));

    results.reserve(count);

    for (auto i = 0u; i < count; ++i)
    {
      results.emplace_back(get_pbmp_data(raw_data));
    }

    return results;
  }
}

#endif//DARKSTARDTSCONVERTER_BITMAP_HPP
