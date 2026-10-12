module;

#include <siege/platform/wave.hpp>

export module siege.content.sfx;

import std;


namespace siege::content::sfx
{
  constexpr siege::platform::wave::file_tag ogg_tag = platform::to_tag<4>({ 'O', 'g', 'g', 'S' });

  constexpr siege::platform::wave::file_tag flac_tag = platform::to_tag<4>({ 'f', 'L', 'a', 'C' });

  export bool is_ogg(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == ogg_tag;
  }

  export bool is_flac(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == flac_tag;
  }

  export bool is_sfx(std::istream& stream)
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

  export class platform_sound
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
