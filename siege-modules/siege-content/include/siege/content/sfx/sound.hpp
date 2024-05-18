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

      std::size_t track_count() const;
      
      std::variant<std::monostate, std::filesystem::path, std::span<std::byte>> get_sound_data(std::size_t);
  private:
      std::vector<std::any> tracks;
  };
}

#endif// OPEN_SIEGE_IMAGE_HPP
