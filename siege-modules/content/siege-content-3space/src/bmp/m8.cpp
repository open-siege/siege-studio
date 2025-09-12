#include <istream>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/tagged_data.hpp>

namespace siege::content::m8
{
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag header = platform::to_tag<4>({ 0x02, 0x00, 0x00, 0x00 });

  bool is_m8(std::istream& stream)
  {
    auto path = siege::platform::get_stream_path(stream);

    if (!path)
    {
      return false;
    }

    if (!(path->extension() == ".m8" || path->extension() == ".M8"))
    {
      return false;
    }

    std::array<std::byte, 4> header{};
    platform::istream_pos_resetter resetter(stream);
    platform::read(stream, header.data(), sizeof(header));

    return header == header;
  }
}// namespace siege::content::m8