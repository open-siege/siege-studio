#ifndef OPEN_SIEGE_SOUND_HPP
#define OPEN_SIEGE_SOUND_HPP

#include <fstream>
#include <array>
#include <vector>
#include <any>
#include <variant>
#include <span>
#include <siege/content/sfx/wave.hpp>

namespace siege::content::sfx
{
  bool is_ogg(std::istream& raw_data);
  bool is_flac(std::istream& raw_data);

  class platform_sound
  {
  public:
      explicit platform_sound(std::filesystem::path);
      explicit platform_sound(std::istream&);
      
      std::variant<std::filesystem::path, std::span<std::byte>> get_sound_data();
  private:
      std::any sound;
  };
}

#endif// OPEN_SIEGE_IMAGE_HPP
