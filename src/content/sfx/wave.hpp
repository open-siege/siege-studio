#ifndef DARKSTARDTSCONVERTER_WAVE_HPP
#define DARKSTARDTSCONVERTER_WAVE_HPP

#include "shared.hpp"

namespace studio::content::sfx
{
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag riff_tag = shared::to_tag<4>({ 'R', 'I', 'F', 'F' });

  constexpr file_tag wave_tag = shared::to_tag<4>({ 'W', 'A', 'V', 'E' });

  constexpr file_tag fmt_tag = shared::to_tag<4>({ 'f', 'm', 't', '0' });

  constexpr file_tag data_tag = shared::to_tag<4>({ 'd', 'a', 't', 'a' });

  constexpr file_tag ogg_tag = shared::to_tag<4>({ 'O', 'g', 'g', 'S' });

  constexpr file_tag voc_tag = shared::to_tag<4>({ 'C', 'r', 'e', 'a' });

  struct format_header
  {
    endian::little_int16_t type;
    endian::little_int16_t num_channels;
    endian::little_int32_t sample_rate;
    endian::little_int32_t byte_rate;
    endian::little_int16_t block_alignment;
    endian::little_int16_t bits_per_sample;
  };

  bool is_sfx_file(std::basic_istream<std::byte>& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(tag.data(), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag != riff_tag && tag != ogg_tag && tag != voc_tag;
  }

  inline std::int32_t write_wav_data(std::basic_ostream<std::byte>& raw_data, const std::vector<std::byte>& samples)
  {
    format_header format_data{};
    format_data.type = 1;
    format_data.num_channels = 1;
    format_data.sample_rate = 11025;
    format_data.bits_per_sample = 8;
    format_data.byte_rate = format_data.sample_rate * format_data.num_channels * format_data.bits_per_sample / 8;
    format_data.block_alignment = format_data.num_channels * format_data.bits_per_sample / 8;

    endian::little_int32_t data_size = static_cast<int32_t>(samples.size() * format_data.num_channels * format_data.bits_per_sample / 8);
    endian::little_int32_t file_size = static_cast<int32_t>(sizeof(std::array<std::int32_t, 5>) + sizeof(format_header) + data_size);
    endian::little_int32_t format_size = static_cast<int32_t>(sizeof(format_header));

    raw_data.write(riff_tag.data(), sizeof(riff_tag));
    raw_data.write(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));
    raw_data.write(wave_tag.data(), sizeof(wave_tag));
    raw_data.write(fmt_tag.data(), sizeof(fmt_tag));

    raw_data.write(reinterpret_cast<std::byte*>(&format_size), sizeof(format_size));
    raw_data.write(reinterpret_cast<std::byte*>(&format_data), sizeof(format_data));

    raw_data.write(data_tag.data(), sizeof(data_tag));
    raw_data.write(reinterpret_cast<std::byte*>(&data_size), sizeof(data_size));
    raw_data.write(samples.data(), samples.size());

    return sizeof(riff_tag) + sizeof(file_size) + file_size;
  }
}

#endif//DARKSTARDTSCONVERTER_WAVE_HPP
