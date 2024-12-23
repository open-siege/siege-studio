#include <array>
#include <siege/platform/resource.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::prj
{
  namespace endian = siege::platform;

   constexpr auto header_tag = platform::to_tag<4>({ 'P', 'R', 'O', 'J' });
   constexpr auto folder_index_tag = platform::to_tag<4>({ 'D', 'D', 'I', 'T' });
   constexpr auto folder_entry_tag = platform::to_tag<4>({ 'I', 'N', 'D', 'X' });
   constexpr auto file_name_data_tag = platform::to_tag<4>({ 'S', 'Y', 'M', 'B' });
   constexpr auto file_entry_data_tag = platform::to_tag<4>({ 'D', 'A', 'T', 'A' });

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

  struct prj_resource_reader final : siege::platform::resource_reader
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

  bool prj_resource_reader::is_supported(std::istream& stream)
  {
    return false;
  }

  bool prj_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<prj_resource_reader::content_info> prj_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<prj_resource_reader::content_info> results;

    return results;
  }

  void prj_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
  }

  void prj_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
  }
}// namespace siege::resource::prj