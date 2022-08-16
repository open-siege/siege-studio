#ifndef OPEN_SIEGE_STREAM_HPP
#define OPEN_SIEGE_STREAM_HPP

#include <istream>
#include <ostream>

namespace studio
{
  inline std::istream& read(std::istream& stream, char* data, std::size_t size)
  {
    return stream.read(data, size);
  }

  inline std::istream& read(std::istream& stream, std::byte* data, std::size_t size)
  {
    return stream.read(reinterpret_cast<char*>(data), size);
  }

  inline std::ostream& write(std::ostream& stream, const std::byte* data, std::size_t size)
  {
    return stream.write(reinterpret_cast<const char*>(data), size);
  }

  inline std::ostream& write(std::ostream& stream, const char* data, std::size_t size)
  {
    return stream.write(data, size);
  }
}

#endif// OPEN_SIEGE_STREAM_HPP
