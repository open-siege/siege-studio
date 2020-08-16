#ifndef DARKSTARDTSCONVERTER_BINARY_IO_HPP
#define DARKSTARDTSCONVERTER_BINARY_IO_HPP

#include <optional>
#include <array>
#include <vector>
#include <fstream>
#include <filesystem>
#include "dts_structures.hpp"
#include "endian_arithmetic.hpp"

namespace binary::io
{
  template<std::size_t size>
  std::array<std::byte, size> read(std::basic_ifstream<std::byte>& stream)
  {
    std::array<std::byte, size> dest{};

    stream.read(&dest[0], size);

    return dest;
  }

  template<typename destination_type>
  destination_type read(std::basic_ifstream<std::byte>& stream)
  {
    destination_type dest{};

    stream.read(reinterpret_cast<std::byte*>(&dest), sizeof(destination_type));

    return dest;
  }

  template<typename destination_type>
  std::vector<destination_type> read_vector(std::basic_ifstream<std::byte>& stream, std::size_t size)
  {
    if (size == 0)
    {
      return {};
    }

    std::vector<destination_type> dest(size);

    stream.read(reinterpret_cast<std::byte*>(&dest[0]), sizeof(destination_type) * size);

    return dest;
  }

  std::string read_string(std::basic_ifstream<std::byte>& stream, std::size_t size, std::size_t max_size = 16)
  {
    std::string dest(size, '\0');

    stream.read(reinterpret_cast<std::byte*>(&dest[0]), size);

    // There is always an embedded \0 in the
    // file if the string length is less than 16 bytes.
    if (size < max_size)
    {
      stream.seekg(1, std::ios_base::cur);
    }

    return dest;
  }


  template<std::size_t Size>
  void write(std::basic_ostream<std::byte>& stream, const std::array<std::byte, Size>& value)
  {
    stream.write(value.data(), Size);
  }

  template<typename ValueType>
  void write(std::basic_ostream<std::byte>& stream, const ValueType& value)
  {
    stream.write(reinterpret_cast<const std::byte*>(&value), sizeof(value));
  }

  template<typename ValueType>
  void write(std::basic_ostream<std::byte>& stream, const std::vector<ValueType>& values)
  {
    stream.write(reinterpret_cast<const std::byte*>(values.data()), values.size() * sizeof(ValueType));
  }

  template<typename ValueType>
  void write(std::basic_ostream<std::byte>& stream, const ValueType* value, std::size_t size)
  {
    stream.write(reinterpret_cast<const std::byte*>(value), size);
  }
}
#endif//DARKSTARDTSCONVERTER_BINARY_IO_HPP