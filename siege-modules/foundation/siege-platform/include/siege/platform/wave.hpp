#ifndef SIEGE_PLATFORM_WAVE_HPP
#define SIEGE_PLATFORM_WAVE_HPP

#include <ostream>
#include <siege/platform/shared.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::platform::wave
{
  namespace endian = siege::platform;
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag riff_tag = platform::to_tag<4>({ 'R', 'I', 'F', 'F' });

  constexpr file_tag wave_tag = platform::to_tag<4>({ 'W', 'A', 'V', 'E' });

  constexpr file_tag fmt_tag = platform::to_tag<4>({ 'f', 'm', 't', ' ' });

  constexpr file_tag data_tag = platform::to_tag<4>({ 'd', 'a', 't', 'a' });

  constexpr file_tag voc_tag = platform::to_tag<4>({ 'C', 'r', 'e', 'a' });

  constexpr file_tag empty_tag = platform::to_tag<4>({ '\0', '\0', '\0', '\0' });

  struct format_header
  {
    endian::little_int16_t type;
    endian::little_int16_t num_channels;
    endian::little_int32_t sample_rate;
    endian::little_int32_t byte_rate;
    endian::little_int16_t block_alignment;
    endian::little_int16_t bits_per_sample;
  };

  inline bool is_wav(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    std::array<std::byte, 4> temp{};
    stream.seekg(sizeof(temp), std::ios::cur);
    stream.read(reinterpret_cast<char*>(temp.data()), sizeof(temp));

    stream.seekg(-int(sizeof(tag)) * 3, std::ios::cur);

    return tag == riff_tag && temp == wave_tag;
  }

  struct header_settings
  {
    std::int16_t num_channels = 1;
    std::int32_t sample_rate = 11025;
    std::int16_t bits_per_sample = 8;
  };

  struct raw_wave
  {
    format_header header;
    std::vector<std::byte> samples;
  };

  inline std::int32_t write_wav_header(std::ostream& raw_data, format_header format_data, std::int32_t data_size)
  {
    endian::little_int32_t data_size_le = data_size;
    endian::little_int32_t file_size = static_cast<int32_t>(sizeof(std::array<std::int32_t, 5>) + sizeof(format_header) + data_size);
    endian::little_int32_t format_size = static_cast<int32_t>(sizeof(format_header));

    raw_data.write(reinterpret_cast<const char*>(riff_tag.data()), sizeof(riff_tag));
    raw_data.write(reinterpret_cast<const char*>(&file_size), sizeof(file_size));
    raw_data.write(reinterpret_cast<const char*>(wave_tag.data()), sizeof(wave_tag));
    raw_data.write(reinterpret_cast<const char*>(fmt_tag.data()), sizeof(fmt_tag));

    raw_data.write(reinterpret_cast<const char*>(&format_size), sizeof(format_size));
    raw_data.write(reinterpret_cast<const char*>(&format_data), sizeof(format_data));

    raw_data.write(reinterpret_cast<const char*>(data_tag.data()), sizeof(data_tag));
    raw_data.write(reinterpret_cast<const char*>(&data_size_le), sizeof(data_size_le));
    return sizeof(riff_tag) + sizeof(file_size) + file_size;
  }

  inline std::int32_t write_wav_header(std::ostream& raw_data, header_settings settings, std::size_t sample_size)
  {
    format_header format_data{};
    format_data.type = 1;
    format_data.num_channels = settings.num_channels;
    format_data.sample_rate = settings.sample_rate;
    format_data.bits_per_sample = settings.bits_per_sample;
    format_data.byte_rate = format_data.sample_rate * format_data.num_channels * format_data.bits_per_sample / 8;
    format_data.block_alignment = std::int16_t(format_data.num_channels) * format_data.bits_per_sample / 8;

    std::int32_t data_size = static_cast<int32_t>(sample_size * format_data.num_channels * format_data.bits_per_sample / 8);

    return write_wav_header(raw_data, format_data, data_size);
  }

  inline std::int32_t write_wav_data(std::ostream& raw_data, header_settings settings, const std::vector<std::byte>& samples)
  {
    auto result = write_wav_header(raw_data, settings, samples.size());
    raw_data.write(reinterpret_cast<const char*>(samples.data()), samples.size());
    return result;
  }
}// namespace siege::platform::wave

#endif// SIEGE_PLATFORM_WAVE_HPP
