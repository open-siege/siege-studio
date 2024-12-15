#include <siege/platform/tagged_data.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/bitmap.hpp>
#include <bitset>

namespace siege::content::tim
{
  namespace endian = siege::platform;
  using file_tag = std::array<std::byte, 8>;

  constexpr file_tag four_bit_image = platform::to_tag<8>({ 0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 });
  constexpr file_tag eight_bit_image = platform::to_tag<8>({ 0x10, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00 });
  constexpr file_tag sixteen_bit_image = platform::to_tag<8>({ 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 });

  struct palette_header
  {
    endian::little_uint32_t size;
    endian::little_uint16_t offset_x;
    endian::little_uint16_t offset_y;
    endian::little_uint16_t color_count;
    endian::little_uint16_t palette_count;
  };

  struct image_header
  {
    endian::little_uint32_t size;
    endian::little_uint16_t offset_x;
    endian::little_uint16_t offset_y;
    endian::little_uint16_t width;
    endian::little_uint16_t height;
  };

  struct tim_data
  {
    enum tim_type
    {
      sixteen_bit,
      eight_bit,
      four_bit
    } type{};
    std::optional<palette_header> palette_header;

    struct tim_palette
    {
      std::vector<endian::little_uint16_t> colours;
    };
    std::vector<tim_palette> palettes;

    image_header header;
    std::vector<endian::little_uint16_t> pixels;
  };

  bool is_tim(std::istream& raw_data)
  {
    std::array<std::byte, 8> header{};
    platform::istream_pos_resetter resetter(raw_data);
    platform::read(raw_data, header.data(), sizeof(header));

    return header == four_bit_image || header == eight_bit_image || header == sixteen_bit_image;
  }

  tim_data get_tim_data(std::istream& raw_data)
  {
    platform::istream_pos_resetter resetter(raw_data);
    std::array<std::byte, 8> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    tim_data result{};

    if (header == four_bit_image || header == eight_bit_image)
    {
      palette_header temp;
      raw_data.read((char*)&temp, sizeof(temp));

      if (temp.palette_count > 4096 || (temp.palette_count == 0 || temp.palette_count >= 10))
      {
        return result;
      }

      result.palette_header = std::move(temp);

      for (auto i = 0; i < result.palette_header->palette_count; ++i)
      {
        auto& palette = result.palettes.emplace_back();
        palette.colours.resize(result.palette_header->color_count);
        raw_data.read((char*)palette.colours.data(), palette.colours.size() * sizeof(std::uint16_t));
      }
    }

    raw_data.read((char*)&result.header, sizeof(result.header));

    auto stride = result.header.width;

    if (header == four_bit_image)
    {
      stride = stride * 2;
      result.type = result.four_bit;
    }
    else if (header == eight_bit_image)
    {
      stride = stride * 2;
      result.type = result.eight_bit;
    }

    result.pixels.resize((int)stride * result.header.height);

    raw_data.read((char*)result.pixels.data(), result.pixels.size() * sizeof(std::uint16_t));

    return result;
  }

  siege::platform::palette::colour to_rgba(std::uint16_t raw_colour)
  {
    siege::platform::palette::colour colour;
    std::bitset<16> bits(raw_colour);
    std::bitset<5> b;
    std::bitset<5> g;
    std::bitset<5> r;
    colour.flags = bits[0] ? std::byte(0xff) : std::byte{};

    b[0] = bits[1];
    b[1] = bits[2];
    b[2] = bits[3];
    b[3] = bits[4];
    b[4] = bits[5];
    g[0] = bits[6];
    g[1] = bits[7];
    g[2] = bits[8];
    g[3] = bits[9];
    g[4] = bits[10];
    r[0] = bits[11];
    r[1] = bits[12];
    r[2] = bits[13];
    r[3] = bits[14];
    r[4] = bits[15];

    colour.red = std::byte(r.to_ulong());
    colour.green = std::byte(g.to_ulong());
    colour.blue = std::byte(b.to_ulong());

    return colour;
  }


  platform::bitmap::windows_bmp_data get_tim_data_as_bitmap(std::istream& raw_data)
  {
    tim_data temp = get_tim_data(raw_data);
    platform::bitmap::windows_bmp_data result{};

    if (temp.type == temp.eight_bit)
    {
      result.info.width = temp.header.width * 2;
      result.info.height = temp.header.height;
      result.info.bit_depth = 8;
      if (temp.palettes.size() > 0)
      {
        result.colours.reserve(temp.palettes[0].colours.size());

        for (auto& colour : temp.palettes[0].colours)
        {
          result.colours.emplace_back(to_rgba(colour));
        }

        result.indexes.reserve(temp.pixels.size() * 2);

        for (auto& pixel : temp.pixels)
        {
          result.indexes.emplace_back((std::int32_t)pixel.data[0]);
          result.indexes.emplace_back((std::int32_t)pixel.data[1]);
        }
      }
    }
    else if (temp.type == temp.four_bit)
    {
      result.info.bit_depth = 8;
      result.info.width = temp.header.width * 4;
      result.info.height = temp.header.height;
      if (temp.palettes.size() > 0)
      {
        result.colours.reserve(temp.palettes[0].colours.size());

        for (auto& colour : temp.palettes[0].colours)
        {
          result.colours.emplace_back(to_rgba(colour));
        }

        result.indexes.reserve(temp.pixels.size() * 2);

        for (auto& pixel : temp.pixels)
        {
          result.indexes.emplace_back((std::uint32_t)pixel.data[0] & 0x0f);
          result.indexes.emplace_back((std::uint32_t)pixel.data[0] >> 4);
          result.indexes.emplace_back((std::uint32_t)pixel.data[1] & 0x0f);
          result.indexes.emplace_back((std::uint32_t)pixel.data[1] >> 4);
        }
      }
    }
    else
    {
      result.colours.reserve(temp.pixels.size());

      for (auto value : temp.pixels)
      {
        result.colours.emplace_back(to_rgba(value));
      }
    }

    return result;
  }


}// namespace siege::content::tim