#ifndef DARKSTARDTSCONVERTER_3D_STRUCTURES_HPP
#define DARKSTARDTSCONVERTER_3D_STRUCTURES_HPP

#include <array>
#include "shared.hpp"
#include "endian_arithmetic.hpp"

namespace studio::content
{
  namespace endian = boost::endian;
  struct texture_vertex
  {
    KEYS_CONSTEXPR static auto keys = shared::make_keys({ "x", "y" });
    float x;
    float y;
  };

  struct vector3f
  {
    KEYS_CONSTEXPR static auto keys = shared::make_keys({ "x", "y", "z" });
    float x;
    float y;
    float z;
  };

  inline vector3f operator+(const vector3f& left, const vector3f& right)
  {
    return { left.x + right.x, left.y + right.y, left.z + right.z };
  }

  inline vector3f operator-(const vector3f& left, const vector3f& right)
  {
    return { left.x - right.x, left.y - right.y, left.z - right.z };
  }

  inline vector3f operator+=(vector3f& left, const vector3f& right)
  {
    left = left + right;
    return left;
  }

  inline vector3f operator*(const vector3f& left, const vector3f& right)
  {
    return { left.x * right.x, left.y * right.y, left.z * right.z };
  }

  inline vector3f operator*=(vector3f& left, const vector3f& right)
  {
    left = left * right;
    return left;
  }

  struct vector3f_pair
  {
    KEYS_CONSTEXPR static auto keys = shared::make_keys({ "min", "max" });
    vector3f min;
    vector3f max;
  };

  static_assert(sizeof(vector3f) == sizeof(std::array<float, 3>));

  struct quaternion4s
  {
    KEYS_CONSTEXPR static auto keys = shared::make_keys({ "x", "y", "z", "w" });
    endian::little_int16_t x;
    endian::little_int16_t y;
    endian::little_int16_t z;
    endian::little_int16_t w;
  };

  struct quaternion4f
  {
    KEYS_CONSTEXPR static auto keys = shared::make_keys({ "x", "y", "z", "w" });
    float x;
    float y;
    float z;
    float w;
  };

  inline quaternion4f to_float(const quaternion4f& other)
  {
    return other;
  }

  inline quaternion4f to_float(const quaternion4s& other)
  {
    constexpr std::int16_t max = SHRT_MAX;

    return {
      float(other.x) / max,
      float(other.y) / max,
      float(other.z) / max,
      float(other.w) / max
    };
  }

  static_assert(sizeof(quaternion4s) == sizeof(std::array<endian::little_int16_t, 4>));

  struct rgb_data
  {
    KEYS_CONSTEXPR static auto keys = shared::make_keys({ "red", "green", "blue", "rgbFlags" });

    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t rgb_flags;
  };
}


#endif//DARKSTARDTSCONVERTER_3D_STRUCTURES_HPP
