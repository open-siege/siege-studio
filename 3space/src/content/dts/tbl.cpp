#include <array>
#include "endian_arithmetic.hpp"
#include "shared.hpp"

namespace studio::content::tbl::three_space
{
  namespace endian = studio::endian;

  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag map_tag = shared::to_tag<4>({ 'M', 'A', 'P', ':' });
  constexpr file_tag app_tag = shared::to_tag<4>({ 'A', 'P', 'P', ':' });
  constexpr file_tag gid_tag = shared::to_tag<4>({ 'G', 'I', 'D', ':' });
  constexpr file_tag dat_tag = shared::to_tag<4>({ 'D', 'A', 'T', ':' });

  struct map_section
  {

  };

  struct app_section
  {
    endian::little_int16_t unknown;
    endian::little_int16_t shape_count;
    endian::little_int16_t data_size;
    std::vector<std::vector<std::byte>> shape_data;
  };

  struct gid_section
  {

  };

  struct dat_section
  {
    endian::little_int16_t unknown0;
    endian::little_int16_t object_count;
    endian::little_int16_t unknown1;
  };

}