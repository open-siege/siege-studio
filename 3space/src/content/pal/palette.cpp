#include <optional>
#include "content/pal/palette.hpp"
#include "stream.hpp"

namespace studio::content::pal
{
  constexpr file_tag riff_tag = shared::to_tag<4>({ 'R', 'I', 'F', 'F' });

  constexpr file_tag pal_tag = shared::to_tag<4>({ 'P', 'A', 'L', ' ' });

  constexpr file_tag data_tag = shared::to_tag<4>({ 'd', 'a', 't', 'a' });

  constexpr file_tag ppl_tag = shared::to_tag<4>({ 'P', 'L', '9', '8' });

  constexpr file_tag dpl_tag = shared::to_tag<4>({ 0x0f, 0x00, 0x28, 0x00 });

  constexpr file_tag dpl_tag2 = shared::to_tag<4>({ 0x02, 0x00, 0x28, 0x00 });

  constexpr file_tag old_pal_tag = shared::to_tag<4>({ 'P', 'A', 'L', ':' });
  constexpr file_tag vga_tag = shared::to_tag<4>({ 'V', 'G', 'A', ':' });
  constexpr file_tag cga_tag = shared::to_tag<4>({ 'C', 'G', 'A', ':' });
  constexpr file_tag ega_tag = shared::to_tag<4>({ 'E', 'G', 'A', ':' });


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

  auto expand(std::byte colour)
  {
    auto temp = int(colour) * 4;
    return temp > 255 ? std::byte(255) : std::byte(temp);
  }

  bool is_old_pal(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};

    studio::read(raw_data, header.data(), sizeof(header));
    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == old_pal_tag;
  }

  void get_old_pal_data(std::istream& raw_data,
    std::vector<old_palette>& results,
    std::optional<std::array<std::byte, 4>> header = std::nullopt)
  {
    endian::little_uint24_t file_size{};

    if (!header.has_value())
    {
      std::array<std::byte, 4> temp;
      studio::read(raw_data, temp.data(), sizeof(temp));
      header.emplace(temp);
    }

    if (results.empty() && header != old_pal_tag)
    {
      throw std::invalid_argument("PAL file not valid.");
    }
    else if (header != old_pal_tag)
    {
      return;
    }

    studio::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));
    raw_data.seekg(sizeof(std::byte), std::ios::cur);

    endian::little_uint32_t block_size{};

    for (auto i = 0u; i < file_size; i += block_size + sizeof(std::array<std::byte, 4>) + sizeof(block_size))
    {
      std::array<std::byte, 4> block;
      studio::read(raw_data, block.data(), sizeof(block));

      if (block == vga_tag)
      {
        studio::read(raw_data, reinterpret_cast<char*>(&block_size), sizeof(block_size));

        auto item_count = block_size / sizeof(std::array<std::byte, 3>);
        std::vector<std::array<std::byte, 3>> colours(item_count);

        studio::read(raw_data, reinterpret_cast<char*>(colours.data()), item_count * sizeof(std::array<std::byte, 3>));

        auto& palette = results.emplace_back(old_palette{});
        palette.type = palette_type::vga;
        palette.colours.reserve(colours.size());

        for (auto& value : colours)
        {
          palette.colours.emplace_back(colour{ expand(value[0]), expand(value[1]), expand(value[2]), std::byte(0xFF) });
        }
      }
      else if (block == cga_tag)
      {
        studio::read(raw_data, reinterpret_cast<char*>(&block_size), sizeof(block_size));

        auto item_count = block_size / sizeof(std::array<std::byte, 16>);
        std::vector<std::array<std::byte, 16>> colours(item_count);

        studio::read(raw_data, reinterpret_cast<char*>(colours.data()), item_count * sizeof(std::array<std::byte, 16>));

        std::uint16_t unknown;
        studio::read(raw_data, reinterpret_cast<char*>(&unknown), sizeof(unknown));
      }
      else if (block == ega_tag)
      {
        studio::read(raw_data, reinterpret_cast<char*>(&block_size), sizeof(block_size));
        std::vector<std::byte> data(block_size);
        studio::read(raw_data, data.data(), block_size);
      }
      else
      {
        block_size = header.value().size();
      }
    }
  }

  std::vector<old_palette> get_old_pal_data(std::istream& raw_data)
  {
    std::vector<old_palette> results;
    get_old_pal_data(raw_data, results);

    std::array<std::byte, 4> header;
    do
    {
      header = std::array<std::byte, 4>{ std::byte(0x00) };
      studio::read(raw_data, header.data(), sizeof(header));

      if (header == old_pal_tag)
      {
        get_old_pal_data(raw_data, results, header);
      }
    } while (header == old_pal_tag);

    return results;
  }

  bool is_microsoft_pal(std::istream& raw_data)
  {
    const auto start = raw_data.tellg();
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};
    std::array<std::byte, 4> sub_header{};

    studio::read(raw_data, header.data(), sizeof(header));
    studio::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));
    studio::read(raw_data, sub_header.data(), sizeof(sub_header));

    raw_data.seekg(start, std::ios::beg);

    return header == riff_tag && sub_header == pal_tag;
  }

  std::vector<colour> get_pal_data(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};
    std::array<std::byte, 4> sub_header{};

    studio::read(raw_data, header.data(), sizeof(header));
    studio::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));

    const auto start = std::size_t(raw_data.tellg());

    studio::read(raw_data, sub_header.data(), sizeof(sub_header));

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

      studio::read(raw_data, chunk_header.data(), chunk_header.size());
      studio::read(raw_data, reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));

      if (chunk_header == data_tag)
      {
        palette_header pal_header{};
        studio::read(raw_data, reinterpret_cast<char*>(&pal_header), sizeof(pal_header));

        colours.reserve(pal_header.colour_count);

        for (auto i = 0; i < pal_header.colour_count; ++i)
        {
          colour colour{};
          studio::read(raw_data, reinterpret_cast<char*>(&colour), sizeof(colour));
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

  std::int32_t write_pal_data(std::ostream& raw_data, const std::vector<colour>& colours)
  {
    raw_data.write(reinterpret_cast<const char*>(riff_tag.data()), sizeof(riff_tag));

    endian::little_int32_t file_size = static_cast<int32_t>(sizeof(std::array<std::int32_t, 3>) + sizeof(palette_header) + sizeof(colour) * colours.size());
    raw_data.write(reinterpret_cast<const char*>(&file_size), sizeof(file_size));
    raw_data.write(reinterpret_cast<const char*>(pal_tag.data()), sizeof(pal_tag));

    raw_data.write(reinterpret_cast<const char*>(data_tag.data()), sizeof(data_tag));

    endian::little_int32_t data_size = static_cast<int32_t>(sizeof(palette_header) + sizeof(colour) * colours.size());
    raw_data.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));

    palette_header header{};
    header.version = 3;
    header.colour_count = static_cast<std::int16_t>(colours.size());

    raw_data.write(reinterpret_cast<const char*>(&header), sizeof(header));
    raw_data.write(reinterpret_cast<const char*>(colours.data()), sizeof(colour) * colours.size());

    return sizeof(riff_tag) + sizeof(file_size) + file_size;
  }

  bool is_phoenix_pal(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};

    studio::read(raw_data, header.data(), sizeof(header));
    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == ppl_tag;
  }

  std::vector<palette> get_ppl_data(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    palette_info info{};

    studio::read(raw_data, header.data(), sizeof(header));

    if (header != ppl_tag)
    {
      throw std::invalid_argument("File data is not PPL as expected.");
    }

    studio::read(raw_data, reinterpret_cast<char*>(&info), sizeof(info));

    std::vector<fixed_palette> temp(info.palette_count);

    studio::read(raw_data, reinterpret_cast<char*>(temp.data()), temp.size() * sizeof(fixed_palette));

    std::vector<palette> results;
    results.reserve(temp.size());

    std::transform(temp.begin(), temp.end(), std::back_inserter(results), [&](const auto& temp_pal) {
      palette new_pal{};
      new_pal.colours = std::vector(temp_pal.colours.begin(), temp_pal.colours.end());
      new_pal.index = temp_pal.index;
      new_pal.type = temp_pal.type;

      return new_pal;
    });

    return results;
  }

  bool is_earthsiege_pal(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};

    studio::read(raw_data, header.data(), sizeof(header));
    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == dpl_tag || header == dpl_tag2;
  }

  std::vector<colour> get_earthsiege_pal(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    studio::read(raw_data, header.data(), sizeof(header));

    if (!(header == dpl_tag || header == dpl_tag2))
    {
      throw std::invalid_argument("File data is not DPL as expected.");
    }

    earthsiege_palette_header info{};

    studio::read(raw_data, reinterpret_cast<char*>(&info), sizeof(info));

    std::vector<colour> results(info.colour_count);

    studio::read(raw_data, reinterpret_cast<char*>(results.data()), results.size() * sizeof(colour));

    endian::little_int32_t unknown;
    studio::read(raw_data, reinterpret_cast<char*>(&unknown), sizeof(unknown));

    for (auto& colour : results)
    {
      colour.red = expand(colour.red);
      colour.green = expand(colour.green);
      colour.blue = expand(colour.blue);

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
}// namespace studio::content::pal
