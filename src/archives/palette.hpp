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
    endian::little_int16_t version;
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
    std::array<colour, 256> shade_map;
    std::array<colour, 256> haze_map;
    std::array<colour, 256> trans_map;

    endian::little_uint32_t index;
    endian::little_uint32_t type;
  };


  void get_pal_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};
    std::array<std::byte, 4> sub_header{};

    raw_data.read(header.data(), sizeof(header));
    raw_data.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));
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

    while (!raw_data.eof())
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

    for (auto& colour : colours)
    {
      std::cout << "#"
                << std::setfill('0')
                << std::setw(2)
                << std::hex
                << int(colour.red)
                << std::setw(2)
                << int(colour.green)
                << std::setw(2)
                << int(colour.blue)
                << '\n';
    }
  }

  void get_ppl_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};
    endian::little_int32_t num_palettes{};

    raw_data.read(header.data(), sizeof(header));

    if (header != ppl_tag)
    {
      throw std::invalid_argument("File data is not PPL as expected.");
    }

    raw_data.read(reinterpret_cast<std::byte*>(&num_palettes), sizeof(num_palettes));
  }
}// namespace darkstar::pal

#endif//DARKSTARDTSCONVERTER_PALETTE_HPP
