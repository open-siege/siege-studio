module;

#include <climits>

export module siege.content.structures;

import std;

import siege.platform.endian;
import siege.platform.shared;

namespace siege::content
{
  namespace endian = siege::platform;

  export struct texture_vertex
  {
    constexpr static auto keys = platform::make_keys({ "x", "y" });
    float x;
    float y;
  };

  export struct vector3f
  {
    constexpr static auto keys = platform::make_keys({ "x", "y", "z" });
    float x;
    float y;
    float z;
  };

  export struct vector4f
  {
    constexpr static auto keys = platform::make_keys({ "x", "y", "z", "w" });
    float x;
    float y;
    float z;
    float w;
  };

  export vector3f operator+(const vector3f& left, const vector3f& right)
  {
    return { left.x + right.x, left.y + right.y, left.z + right.z };
  }

  export vector3f operator-(const vector3f& left, const vector3f& right)
  {
    return { left.x - right.x, left.y - right.y, left.z - right.z };
  }

  export vector3f operator+=(vector3f& left, const vector3f& right)
  {
    left = left + right;
    return left;
  }

  export vector3f operator*(const vector3f& left, const vector3f& right)
  {
    return { left.x * right.x, left.y * right.y, left.z * right.z };
  }

  export vector3f operator*=(vector3f& left, const vector3f& right)
  {
    left = left * right;
    return left;
  }

  export struct vector3f_pair
  {
    constexpr static auto keys = platform::make_keys({ "min", "max" });
    vector3f min;
    vector3f max;
  };

  static_assert(sizeof(vector3f) == sizeof(std::array<float, 3>));

  export struct vector3s
  {
    constexpr static auto keys = platform::make_keys({ "x", "y", "z" });
    endian::little_int16_t x;
    endian::little_int16_t y;
    endian::little_int16_t z;
  };

  export struct quaternion4s
  {
    constexpr static auto keys = platform::make_keys({ "x", "y", "z", "w" });
    endian::little_int16_t x;
    endian::little_int16_t y;
    endian::little_int16_t z;
    endian::little_int16_t w;
  };

  export struct quaternion4f
  {
    constexpr static auto keys = platform::make_keys({ "x", "y", "z", "w" });
    float x;
    float y;
    float z;
    float w;
  };

  export quaternion4f to_float(const quaternion4f& other)
  {
    return other;
  }

  export quaternion4f to_float(const quaternion4s& other)
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

  export struct rgb_data
  {
    constexpr static auto keys = platform::make_keys({ "red", "green", "blue", "rgbFlags" });

    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t rgb_flags;
  };
}
