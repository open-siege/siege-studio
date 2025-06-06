#include <optional>
#include <siege/platform/palette.hpp>
#include <siege/platform/stream.hpp>

namespace siege::platform::palette
{
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag riff_tag = platform::to_tag<4>({ 'R', 'I', 'F', 'F' });

  constexpr file_tag pal_tag = platform::to_tag<4>({ 'P', 'A', 'L', ' ' });

  constexpr file_tag data_tag = platform::to_tag<4>({ 'd', 'a', 't', 'a' });

  struct palette_header
  {
    endian::big_int16_t version;
    endian::little_int16_t colour_count;
  };

  auto expand(std::byte colour)
  {
    auto temp = int(colour) * 4;
    return temp > 255 ? std::byte(255) : std::byte(temp);
  }

  bool is_microsoft_pal(std::istream& raw_data)
  {
    const auto start = raw_data.tellg();
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};
    std::array<std::byte, 4> sub_header{};

    platform::read(raw_data, header.data(), sizeof(header));
    platform::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));
    platform::read(raw_data, sub_header.data(), sizeof(sub_header));

    raw_data.seekg(start, std::ios::beg);

    return header == riff_tag && sub_header == pal_tag;
  }

  std::vector<colour> get_pal_data(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};
    std::array<std::byte, 4> sub_header{};

    platform::read(raw_data, header.data(), sizeof(header));
    platform::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));

    const auto start = std::size_t(raw_data.tellg());

    platform::read(raw_data, sub_header.data(), sizeof(sub_header));

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

      platform::read(raw_data, chunk_header.data(), chunk_header.size());
      platform::read(raw_data, reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));

      if (chunk_header == data_tag)
      {
        palette_header pal_header{};
        platform::read(raw_data, reinterpret_cast<char*>(&pal_header), sizeof(pal_header));

        colours.reserve(pal_header.colour_count);

        for (auto i = 0; i < pal_header.colour_count; ++i)
        {
          colour colour{};
          platform::read(raw_data, reinterpret_cast<char*>(&colour), sizeof(colour));
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
}// namespace siege::content::pal
