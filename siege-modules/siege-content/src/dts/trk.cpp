// TRK - Star Trek Invasion model files

#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::trk
{
  namespace endian = siege::platform;

  // actually the number of files in the file
  constexpr static auto header_tag = platform::to_tag<4>("TREK");
  constexpr static auto model_tag = platform::to_tag<4>("MODL");
  constexpr static auto lod_tag = platform::to_tag<4>("LODC");
  constexpr static auto collision_tag = platform::to_tag<4>("COLL");
  constexpr static auto tex_tag = platform::to_tag<4>("TEXS");
  constexpr static auto engine_tag = platform::to_tag<4>("ENGI");
  constexpr static auto tims_tag = platform::to_tag<4>("TIMS");
  constexpr static auto finish_tag = platform::to_tag<4>("FINI");

  bool is_trk(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag{};

    stream.read((char*)&tag, sizeof(tag));
    return tag == header_tag;
  }
}