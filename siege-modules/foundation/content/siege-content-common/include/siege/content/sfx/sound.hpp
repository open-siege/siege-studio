#ifndef OPEN_SIEGE_SOUND_HPP
#define OPEN_SIEGE_SOUND_HPP

#include <fstream>
#include <array>
#include <vector>
#include <any>
#include <variant>
#include <span>
#include <siege/platform/wave.hpp>

namespace siege::content::sfx
{
  bool is_ogg(std::istream& raw_data);
  bool is_flac(std::istream& raw_data);

  inline bool is_sfx(std::istream& stream)
  {
    if (is_ogg(stream) || is_flac(stream))
    {
      return false;
    }

    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag != platform::wave::riff_tag && tag != platform::wave::voc_tag && tag != platform::wave::empty_tag;
  }

  class platform_sound
  {
  public:
    explicit platform_sound(std::filesystem::path);
    explicit platform_sound(std::istream&);
    
    std::size_t track_count() const;

    std::variant<std::monostate, std::filesystem::path, std::span<std::byte>> get_sound_data(std::size_t);

  private:
    std::vector<std::any> tracks;
  };
}// namespace siege::content::sfx

#endif// OPEN_SIEGE_IMAGE_HPP
