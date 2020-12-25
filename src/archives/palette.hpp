#ifndef DARKSTARDTSCONVERTER_PALETTE_HPP
#define DARKSTARDTSCONVERTER_PALETTE_HPP

#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "endian_arithmetic.hpp"

namespace darkstar::pal
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

  constexpr file_tag riff_tag = to_tag({ 'R', 'I', 'F', 'F' });

  constexpr file_tag pal_tag = to_tag({ 'P', 'A', 'L', ' ' });

  constexpr file_tag data_tag = to_tag({ 'd', 'a', 't', 'a' });

  constexpr file_tag ppl_tag = to_tag({ 'P', 'L', '9', '8' });

  struct colour
  {
    std::byte red;
    std::byte green;
    std::byte blue;
    std::byte flags;
  };

  struct palette_header
  {
    endian::big_int16_t version;
    endian::little_int16_t colour_count;
  };

  struct palette_info
  {
    endian::little_int32_t palette_count;
    endian::little_int32_t shade_shift;
    endian::little_int32_t haze_level;
    colour haze_colour;
    std::array<std::byte, 32> allowed_matches;
  };

  struct palette
  {
    std::array<colour, 256> colours;
    endian::little_uint32_t index;
    endian::little_uint32_t type;
  };

  std::vector<colour> get_pal_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};
    std::array<std::byte, 4> sub_header{};

    raw_data.read(header.data(), sizeof(header));
    raw_data.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));

    const auto start = std::size_t(raw_data.tellg());

    raw_data.read(sub_header.data(), sizeof(sub_header));

    if (header != riff_tag)
    {
      throw std::invalid_argument("File data is not RIFF based.");
    }

    if (sub_header != pal_tag)
    {
      throw std::invalid_argument("File data is RIFF based but is not a PAL file.");
    }

    std::vector<colour> colours;

    const auto end = start + file_size + sizeof(header) + sizeof(file_size);

    while (std::size_t(raw_data.tellg()) < end)
    {
      std::array<std::byte, 4> chunk_header{};
      endian::little_uint32_t chunk_size{};

      raw_data.read(chunk_header.data(), chunk_header.size());
      raw_data.read(reinterpret_cast<std::byte*>(&chunk_size), sizeof(chunk_size));

      if (chunk_header == data_tag)
      {
        palette_header pal_header{};
        raw_data.read(reinterpret_cast<std::byte*>(&pal_header), sizeof(pal_header));

        colours.reserve(pal_header.colour_count);

        for (auto i = 0; i < pal_header.colour_count; ++i)
        {
          colour colour{};
          raw_data.read(reinterpret_cast<std::byte*>(&colour), sizeof(colour));
          colours.emplace_back(colour);
        }

        break;
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

    return colours;
  }

  std::int32_t write_pal_data(std::basic_ofstream<std::byte>& raw_data, const std::vector<colour>& colours)
  {
    raw_data.write(riff_tag.data(), sizeof(riff_tag));

    endian::little_int32_t file_size = static_cast<int32_t>(sizeof(std::array<std::int32_t, 3>) + sizeof(palette_header) + sizeof(colour) * colours.size());
    raw_data.write(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));
    raw_data.write(pal_tag.data(), sizeof(pal_tag));

    raw_data.write(data_tag.data(), sizeof(data_tag));

    endian::little_int32_t data_size = static_cast<int32_t>(sizeof(palette_header) + sizeof(colour) * colours.size());
    raw_data.write(reinterpret_cast<std::byte*>(&data_size), sizeof(data_size));

    palette_header header{};
    header.version = 3;
    header.colour_count = static_cast<std::int16_t>(colours.size());

    raw_data.write(reinterpret_cast<std::byte*>(&header), sizeof(header));
    raw_data.write(reinterpret_cast<const std::byte*>(colours.data()), sizeof(colour) * colours.size());

    return sizeof(riff_tag) + sizeof(file_size) + file_size;
  }

  std::vector<palette> get_ppl_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};
    palette_info info{};

    raw_data.read(header.data(), sizeof(header));

    if (header != ppl_tag)
    {
      throw std::invalid_argument("File data is not PPL as expected.");
    }

    raw_data.read(reinterpret_cast<std::byte*>(&info), sizeof(info));

    std::vector<palette> results(info.palette_count);

    raw_data.read(reinterpret_cast<std::byte*>(results.data()), results.size() * sizeof(palette));

    return results;
  }
}// namespace darkstar::pal

#endif//DARKSTARDTSCONVERTER_PALETTE_HPP
