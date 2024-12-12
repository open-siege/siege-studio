//constexpr auto header_tag = platform::to_tag<4>({ 'P', 'R', 'O', 'J' });
//constexpr auto folder_index_tag = platform::to_tag<4>({ 'D', 'D', 'I', 'T' });
//constexpr auto folder_entry_tag = platform::to_tag<4>({ 'I', 'N', 'D', 'X' });
//constexpr auto file_entry_data = platform::to_tag<4>({ 'S', 'Y', 'M', 'B' });
//constexpr auto file_entry_data = platform::to_tag<4>({ 'D', 'A', 'T', 'A' });

// uses lzw compression

namespace siege::resource::mw4
{
    struct dir_entry
    {
        std::array<char, 4> tag;
        endian::little_uint32_t index_offset;
        endian::little_uint32_t index_size;
        endian::little_uint32_t symbol_offset;
        endian::little_uint32_t symbol_size;
        endian::little_uint32_t unknown;
    };

    struct prf_index
    {
        endian::little_uint32_t offset;
        endian::little_uint32_t size;
    };


}