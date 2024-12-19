//constexpr auto header_tag = platform::to_tag<4>({ '#', 'V', 'B', 'D' });
//constexpr auto item_tag = platform::to_tag<4>({ 0x80, 0x10, 0x54, 0xc0 });

// uses lzw compression

#include <memory>
#include <siege/platform/resource.hpp>
#include <siege/platform/stream.hpp>
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

    struct mw4_resource_reader final : siege::platform::resource_reader
    {
      static bool is_supported(std::istream& stream);

      bool stream_is_supported(std::istream& stream) const override;
      std::vector<content_info> get_content_listing(std::istream& stream, const platform::listing_query& query) const override;
      void set_stream_position(std::istream& stream, const siege::platform::file_info& info) const override;
      void extract_file_contents(std::istream& stream,
        const siege::platform::file_info& info,
        std::ostream& output,
        std::optional<std::reference_wrapper<platform::batch_storage>> = std::nullopt) const override;
    };

    bool mw4_resource_reader::is_supported(std::istream& stream)
    {
      return false;
    }

    bool mw4_resource_reader::stream_is_supported(std::istream& stream) const
    {
      return is_supported(stream);
    }

    std::vector<mw4_resource_reader::content_info> mw4_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
    {
      platform::istream_pos_resetter resetter(stream);
      std::vector<mw4_resource_reader::content_info> results;

      return results;
    }

    void mw4_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
    {
      
    }

    void mw4_resource_reader::extract_file_contents(std::istream& stream,
      const siege::platform::file_info& info,
      std::ostream& output,
      std::optional<std::reference_wrapper<platform::batch_storage>>) const
    {

    }
}