#include <set>
#include <map>
#include <cmath>
#include <siege/content/bmp/bitmap.hpp>
#include <siege/platform/tagged_data.hpp>
#include <siege/platform/stream.hpp>

namespace siege::content::bmp
{
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag bmp_alt1_tag = platform::to_tag<4>({ 'B', 'M', 'P', ' ' });

  constexpr file_tag bmp_alt2_tag = platform::to_tag<4>({ 'B', 'M', 'P', ':' });

  constexpr file_tag pbmp_tag = platform::to_tag<4>({ 'P', 'B', 'M', 'P' });

  constexpr file_tag pba_tag = platform::to_tag<4>({ 'P', 'B', 'M', 'A' });

  constexpr file_tag header_tag = platform::to_tag<4>({ 'h', 'e', 'a', 'd' });

  constexpr file_tag data_tag = platform::to_tag<4>({ 'd', 'a', 't', 'a' });

  constexpr file_tag detail_tag = platform::to_tag<4>({ 'D', 'E', 'T', 'L' });

  constexpr file_tag palette_tag = platform::to_tag<4>({ 'P', 'i', 'D', 'X' });

  constexpr file_tag dbm_tag = platform::to_tag<4>({ 0x0e, 0x00, 0x28, 0x00 });

  constexpr file_tag dci_tag = platform::to_tag<4>({ 0x0b, 0x00, 0x28, 0x00 });

  constexpr file_tag dba_tag = platform::to_tag<4>({ 0x01, 0x00, 0x28, 0x00 });

  constexpr std::array<std::byte, 2> special_reserved_tag = { std::byte{ 0xF7 }, std::byte{ 0xF5 } };// for palettes

  bool is_earthsiege_bmp(std::istream& raw_data)
  {
    platform::istream_pos_resetter resetter(raw_data);
    std::array<std::byte, 4> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    return header == dbm_tag;
  }

  bool is_earthsiege_curose(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == dci_tag;
  }

  bool is_earthsiege_bmp_array(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == dba_tag;
  }

  dbm_data read_earthsiege_bmp(std::istream& raw_data)
  {
    dbm_data data{};
    platform::read(raw_data, reinterpret_cast<char*>(&data.header), sizeof(data.header));

    if (data.header.tag != dbm_tag)
    {
      throw std::invalid_argument("File data is not DBM based at offset " + std::to_string(int(raw_data.tellg()) - sizeof(data.header)));
    }

    auto total_size = int(data.header.width) * int(data.header.height) * (int(data.header.bit_depth)/ 8);

    if (total_size == data.header.image_size)
    {
      data.pixels = std::vector<std::byte>(data.header.image_size);
      platform::read(raw_data, data.pixels.data(), data.pixels.size());
    }

    return data;
  }

  dbm_data read_earthsiege_cursor(std::istream& raw_data)
  {
    std::array<std::byte, 4> file_header{};

    platform::read(raw_data, file_header.data(), sizeof(file_header));

    if (file_header != dci_tag)
    {
      throw std::invalid_argument("File data is not DCI based at offset " + std::to_string(int(raw_data.tellg()) - sizeof(file_header)));
    }
    endian::little_uint32_t file_size{};
    platform::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));

    std::array<endian::little_uint32_t, 2> unknown{};
    platform::read(raw_data, reinterpret_cast<char*>(unknown.data()), sizeof(unknown));

    return read_earthsiege_bmp(raw_data);
  }

  std::vector<dbm_data> read_earthsiege_bmp_array(std::istream& raw_data)
  {
    std::vector<dbm_data> results;
    dba_header header;
    platform::read(raw_data, reinterpret_cast<char*>(&header), sizeof(header));

    results.reserve(header.count);

    for (auto i = 0u; i < header.count; ++i)
    {
      results.emplace_back(read_earthsiege_bmp(raw_data));
      skip_alignment_bytes(raw_data, results.back().header.file_size);
    }

    return results;
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

  bool is_phoenix_bmp(std::istream& raw_data)
  {
    platform::istream_pos_resetter resetter(raw_data);
    std::array<std::byte, 4> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    return header == pbmp_tag;
  }

  pbmp_data get_pbmp_data(std::istream& raw_data)
  {
    const auto start = std::size_t(raw_data.tellg());
    std::array<std::byte, 4> header{};
    endian::little_uint32_t file_size{};

    platform::read(raw_data, header.data(), sizeof(header));
    platform::read(raw_data, reinterpret_cast<char*>(&file_size), sizeof(file_size));

    if (header != pbmp_tag)
    {
      throw std::invalid_argument("File data is not PBMP based. Offset is " + std::to_string(raw_data.tellg()));
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

      platform::read(raw_data, chunk_header.data(), chunk_header.size());
      platform::read(raw_data, reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));

      if (chunk_header == header_tag)
      {
        platform::read(raw_data, reinterpret_cast<char*>(&bmp_header), sizeof(bmp_header));

        const auto num_pixels = bmp_header.width.value() * bmp_header.height * (bmp_header.bit_depth / 8);
        pixels = std::vector<std::byte>(num_pixels, std::byte{});
      }
      else if (chunk_header == data_tag)
      {
        auto bytes_read = read_pixel_data<std::int32_t>(raw_data, pixels, bmp_header.width, bmp_header.height, bmp_header.bit_depth);
        // PBMP files contain mip maps of the main image.
        // For now, we won't worry about them.
        raw_data.seekg(chunk_size - bytes_read, std::ios::cur);
      }
      else if (chunk_header == detail_tag)
      {
        platform::read(raw_data, reinterpret_cast<char*>(&detail_levels), sizeof(detail_levels));
      }
      else if (chunk_header == palette_tag)
      {
        platform::read(raw_data, reinterpret_cast<char*>(&palette_index), sizeof(palette_index));
      }
      else if (chunk_header == pbmp_tag)
      {
        raw_data.seekg(-int(sizeof(file_size)), std::ios::cur);
        raw_data.seekg(-int(sizeof(pbmp_tag)), std::ios::cur);
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

    return {
      bmp_header,
      detail_levels,
      palette_index,
      pixels
    };
  }

  void write_pbmp_data(std::ofstream& raw_data,
    std::int32_t width,
    std::int32_t height,
    const std::vector<pal::colour>& colours,
    const std::vector<std::byte>& pixels,
    std::optional<std::uint32_t> palette_id)
  {
    platform::write(raw_data, pbmp_tag.data(), sizeof(pbmp_tag));

    auto file_size_pos = raw_data.tellp();

    endian::little_int32_t file_size = 0;
    platform::write(raw_data, reinterpret_cast<const char*>(&file_size), sizeof(file_size));

    platform::write(raw_data, header_tag.data(), sizeof(header_tag));
    endian::little_int32_t header_size = sizeof(pbmp_header);
    platform::write(raw_data, reinterpret_cast<const char*>(&header_size), sizeof(header_size));
    pbmp_header header{};
    header.version = 3;
    header.width = width;
    header.height = height;
    header.bit_depth = 8;
    header.flags = 8;

    platform::write(raw_data, reinterpret_cast<const char*>(&header), sizeof(header));

    auto pal_bytes = platform::palette::write_pal_data(raw_data, colours);

    platform::write(raw_data, data_tag.data(), sizeof(data_tag));
    header_size = std::int32_t(pixels.size());
    platform::write(raw_data, reinterpret_cast<const char*>(&header_size), sizeof(header_size));
    platform::write(raw_data, pixels.data(), pixels.size());

    platform::write(raw_data, detail_tag.data(), sizeof(detail_tag));
    header_size = sizeof(std::int32_t);
    platform::write(raw_data, reinterpret_cast<const char*>(&header_size), sizeof(header_size));
    endian::little_int32_t num_details = 1;
    platform::write(raw_data, reinterpret_cast<const char*>(&num_details), sizeof(num_details));


    if (palette_id.has_value())
    {
      platform::write(raw_data, palette_tag.data(), sizeof(palette_tag));
      header_size = sizeof(std::int32_t);
      platform::write(raw_data, reinterpret_cast<const char*>(&header_size), sizeof(header_size));
      endian::little_uint32_t palette_index = palette_id.value();
      platform::write(raw_data, reinterpret_cast<const char*>(&palette_index), sizeof(palette_index));
    }

    raw_data.seekp(file_size_pos, std::ios::beg);

    file_size = std::int32_t(sizeof(std::int32_t) + sizeof(pbmp_header) + pal_bytes + sizeof(std::array<std::int32_t, 2>) + pixels.size());
    platform::write(raw_data, reinterpret_cast<const char*>(&file_size), sizeof(file_size));
  }

  bool is_phoenix_bmp_array(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    platform::read(raw_data, header.data(), sizeof(header));

    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == pba_tag;
  }

  std::vector<pbmp_data> get_pba_data(std::istream& raw_data)
  {
    std::vector<pbmp_data> results;

    std::array<std::byte, 4> header{};
    endian::little_uint32_t count{};

    platform::read(raw_data, header.data(), sizeof(header));
    platform::read(raw_data, reinterpret_cast<char*>(&count), sizeof(count));

    if (header != pba_tag)
    {
      throw std::invalid_argument("File data is not PBA based.");
    }

    platform::read(raw_data, header.data(), sizeof(header));

    platform::read(raw_data, reinterpret_cast<char*>(&count), sizeof(count));

    // Count appears twice in the files. Or it means something else. Haven't seen a file where it is something else.
    platform::read(raw_data, reinterpret_cast<char*>(&count), sizeof(count));
    platform::read(raw_data, reinterpret_cast<char*>(&count), sizeof(count));

    results.reserve(count);

    for (auto i = 0u; i < count; ++i)
    {
      results.emplace_back(get_pbmp_data(raw_data));
    }

    return results;
  }

  template<typename IndexType>
  std::vector<IndexType> remap_bitmap(const std::vector<IndexType>& pixels,
                                         const std::vector<pal::colour>& original_colours,
                                         const std::vector<pal::colour>& other_colours,
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
          auto distance = pal::colour_distance(colour, other);
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
                                         const std::vector<pal::colour>& original_colours,
                                         const std::vector<pal::colour>& other_colours,
                                         bool only_unique)
  {
    return remap_bitmap<std::byte>(pixels, original_colours, other_colours, only_unique);
  }


  std::vector<std::int32_t> remap_bitmap(const std::vector<std::int32_t>& pixels,
    const std::vector<pal::colour>& original_colours,
    const std::vector<pal::colour>& other_colours,
    bool only_unique)
  {
    return remap_bitmap<std::int32_t>(pixels, original_colours, other_colours, only_unique);
  }
}// namespace siege::content::bmp
