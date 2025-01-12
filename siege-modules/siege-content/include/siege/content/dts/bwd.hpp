#ifndef SIEGE_BWD_HPP
#define SIEGE_BWD_HPP

#include <array>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/content/dts/wtb.hpp>


namespace siege::content::bwd
{
  namespace endian = siege::platform;

  struct vec_3s
  {
    endian::little_int16_t x;
    endian::little_int16_t y;
    endian::little_int16_t z;
  };

  struct dtb_object
  {
    endian::little_int16_t id;
    vec_3s origin;
    std::array<endian::little_uint16_t, 11> unknown;
    vec_3s translation;
    vec_3s scale;
    std::array<endian::little_uint16_t, 3> unknown2;
    endian::little_int16_t prf_file_index;
    endian::little_int16_t padding;
  };

  struct dtb_object_with_shape : dtb_object
  {
    std::optional<wtb::wtb_shape> shape;
  };


  struct file_info
  {
    endian::little_int16_t prj_file_index;
    std::array<char, 10> string_tag;
  };

  struct bwd_model
  {
    std::array<char, 4> version;
    std::vector<dtb_object_with_shape> objects;
    std::map<int, std::vector<dtb_object_with_shape>> lod_objects;
    file_info animation_file;
    file_info vpt_file;
    file_info cockpit_file;
    file_info hud_file;
    file_info mgd_file;
    file_info pit_file;
    file_info sound_file;
  };

  bool is_bwd(std::istream& stream);
  bwd_model load_bwd(std::istream& stream);
}// namespace siege::content::bwd

#endif// !SIEGE_WTB_HPP
