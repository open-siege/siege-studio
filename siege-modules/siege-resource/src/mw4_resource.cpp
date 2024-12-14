//constexpr auto header_tag = platform::to_tag<4>({ '#', 'V', 'B', 'D' });
//constexpr auto item_tag = platform::to_tag<4>({ 0x80, 0x10, 0x54, 0xc0 });

// uses lzw compression

#include <memory>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::mw4
{
    namespace endian = siege::platform;

    struct mw4_file_entry
    {
        std::byte compression_type;
        endian::little_uint32_t uncompressed_size;
        endian::little_uint32_t compressed_size;
        endian::little_uint16_t id;
        char string_size;
        std::unique_ptr<char> path;
    };
}