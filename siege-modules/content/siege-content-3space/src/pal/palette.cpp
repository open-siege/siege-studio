#include <optional>
#include <siege/content/pal/palette.hpp>
#include <siege/platform/stream.hpp>

namespace siege::content::pal
{
  constexpr file_tag ppl_tag = platform::to_tag<4>({ 'P', 'L', '9', '8' });

  constexpr file_tag dpl_tag = platform::to_tag<4>({ 0x0f, 0x00, 0x28, 0x00 });

  constexpr file_tag dpl_tag2 = platform::to_tag<4>({ 0x02, 0x00, 0x28, 0x00 });

  constexpr file_tag old_pal_tag = platform::to_tag<4>({ 'P', 'A', 'L', ':' });
  constexpr file_tag vga_tag = platform::to_tag<4>({ 'V', 'G', 'A', ':' });
  constexpr file_tag cga_tag = platform::to_tag<4>({ 'C', 'G', 'A', ':' });
  constexpr file_tag ega_tag = platform::to_tag<4>({ 'E', 'G', 'A', ':' });

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

    platform::read(raw_data, header.data(), sizeof(header));
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
      platform::read(raw_data, temp.data(), sizeof(temp));
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

    platform::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));
    raw_data.seekg(sizeof(std::byte), std::ios::cur);

    endian::little_uint32_t block_size{};

    for (auto i = 0u; i < file_size; i += block_size + sizeof(std::array<std::byte, 4>) + sizeof(block_size))
    {
      std::array<std::byte, 4> block;
      platform::read(raw_data, block.data(), sizeof(block));

      if (block == vga_tag)
      {
        platform::read(raw_data, reinterpret_cast<char*>(&block_size), sizeof(block_size));

        auto item_count = block_size / sizeof(std::array<std::byte, 3>);
        std::vector<std::array<std::byte, 3>> colours(item_count);

        platform::read(raw_data, reinterpret_cast<char*>(colours.data()), item_count * sizeof(std::array<std::byte, 3>));

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
        platform::read(raw_data, reinterpret_cast<char*>(&block_size), sizeof(block_size));

        auto item_count = block_size / sizeof(std::array<std::byte, 16>);
        std::vector<std::array<std::byte, 16>> colours(item_count);

        platform::read(raw_data, reinterpret_cast<char*>(colours.data()), item_count * sizeof(std::array<std::byte, 16>));

        std::uint16_t unknown;
        platform::read(raw_data, reinterpret_cast<char*>(&unknown), sizeof(unknown));
      }
      else if (block == ega_tag)
      {
        platform::read(raw_data, reinterpret_cast<char*>(&block_size), sizeof(block_size));
        std::vector<std::byte> data(block_size);
        platform::read(raw_data, data.data(), block_size);
      }
      else
      {
        block_size = static_cast<std::uint32_t>(header.value().size());
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
      platform::read(raw_data, header.data(), sizeof(header));

      if (header == old_pal_tag)
      {
        get_old_pal_data(raw_data, results, header);
      }
    } while (header == old_pal_tag);

    return results;
  }

  bool is_phoenix_pal(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};

    platform::read(raw_data, header.data(), sizeof(header));
    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == ppl_tag;
  }

  std::vector<palette> get_ppl_data(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    palette_info info{};

    platform::read(raw_data, header.data(), sizeof(header));

    if (header != ppl_tag)
    {
      throw std::invalid_argument("File data is not PPL as expected.");
    }

    platform::read(raw_data, reinterpret_cast<char*>(&info), sizeof(info));

    std::vector<fixed_palette> temp(info.palette_count);

    platform::read(raw_data, reinterpret_cast<char*>(temp.data()), temp.size() * sizeof(fixed_palette));

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

    platform::read(raw_data, header.data(), sizeof(header));
    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == dpl_tag || header == dpl_tag2;
  }

  std::vector<colour> get_earthsiege_pal(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    if (!(header == dpl_tag || header == dpl_tag2))
    {
      throw std::invalid_argument("File data is not DPL as expected.");
    }

    earthsiege_palette_header info{};

    platform::read(raw_data, reinterpret_cast<char*>(&info), sizeof(info));

    std::vector<colour> results(info.colour_count);

    platform::read(raw_data, reinterpret_cast<char*>(results.data()), results.size() * sizeof(colour));

    endian::little_int32_t unknown;
    platform::read(raw_data, reinterpret_cast<char*>(&unknown), sizeof(unknown));

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
}// namespace siege::content::pal
