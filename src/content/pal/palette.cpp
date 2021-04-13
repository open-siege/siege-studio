#include "palette.hpp"

namespace studio::content::pal
{
  constexpr file_tag riff_tag = shared::to_tag<4>({ 'R', 'I', 'F', 'F' });

  constexpr file_tag pal_tag = shared::to_tag<4>({ 'P', 'A', 'L', ' ' });

  constexpr file_tag data_tag = shared::to_tag<4>({ 'd', 'a', 't', 'a' });

  constexpr file_tag ppl_tag = shared::to_tag<4>({ 'P', 'L', '9', '8' });

  constexpr file_tag dpl_tag = shared::to_tag<4>({ 0x0f, 0x00, 0x28, 0x00 });


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

  struct earthsiege_palette_header
  {
    endian::little_int32_t total_size;
    endian::little_int32_t colour_count;
  };

  struct fixed_palette
  {
    std::array<colour, 256> colours;
    endian::little_uint32_t index;
    endian::little_uint32_t type;
  };

  bool is_microsoft_pal(std::basic_istream<std::byte>& raw_data)
  {
    const auto start = raw_data.tellg();
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};
    std::array<std::byte, 4> sub_header{};

    raw_data.read(header.data(), sizeof(header));
    raw_data.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));
    raw_data.read(sub_header.data(), sizeof(sub_header));

    raw_data.seekg(start, std::ios::beg);

    return header == riff_tag && sub_header == pal_tag;
  }

  std::vector<colour> get_pal_data(std::basic_istream<std::byte>& raw_data)
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

  std::int32_t write_pal_data(std::basic_ostream<std::byte>& raw_data, const std::vector<colour>& colours)
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

  bool is_phoenix_pal(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};

    raw_data.read(header.data(), sizeof(header));
    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == ppl_tag;
  }

  std::vector<palette> get_ppl_data(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};
    palette_info info{};

    raw_data.read(header.data(), sizeof(header));

    if (header != ppl_tag)
    {
      throw std::invalid_argument("File data is not PPL as expected.");
    }

    raw_data.read(reinterpret_cast<std::byte*>(&info), sizeof(info));

    std::vector<fixed_palette> temp(info.palette_count);

    raw_data.read(reinterpret_cast<std::byte*>(temp.data()), temp.size() * sizeof(fixed_palette));

    std::vector<palette> results;
    results.reserve(temp.size());

    std::transform(temp.begin(), temp.end(), std::back_inserter(results), [&] (const auto& temp_pal){
           palette new_pal{};
           new_pal.colours = std::vector(temp_pal.colours.begin(), temp_pal.colours.end());
           new_pal.index = temp_pal.index;
           new_pal.type = temp_pal.type;

           return new_pal;
    });

    return results;
  }

  bool is_earthsiege_pal(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};

    raw_data.read(header.data(), sizeof(header));
    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == dpl_tag;
  }

  std::vector<colour> get_earthsiege_data(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};
    raw_data.read(header.data(), sizeof(header));

    if (header != dpl_tag)
    {
      throw std::invalid_argument("File data is not PPL as expected.");
    }

    earthsiege_palette_header info{};

    raw_data.read(reinterpret_cast<std::byte*>(&info), sizeof(info));

    std::vector<colour> results(info.colour_count);

    raw_data.read(reinterpret_cast<std::byte*>(results.data()), results.size() * sizeof(colour));

    endian::little_int32_t unknown;
    raw_data.read(reinterpret_cast<std::byte*>(&unknown), sizeof(unknown));

    for (auto& colour : results)
    {
      if (colour.flags == std::byte(0x01))
      {
        colour.flags = std::byte(0xFF);
      }
      else
      {
        colour.flags = std::byte(0);
      }
    }

    return results;
  }
}