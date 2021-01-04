#ifndef DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
#define DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP

#include "archive.hpp"
#include "endian_arithmetic.hpp"

namespace trophy_bass::vol
{
  struct rbx_file_archive : shared::archive::file_archive
  {
    static bool is_supported(std::basic_istream<std::byte>& stream);

    bool stream_is_supported(std::basic_istream<std::byte>& stream) const override;

    std::vector<content_info> get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const override;

    void set_stream_position(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info) const override;

    void extract_file_contents(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info, std::basic_ostream<std::byte>& output) const override;
  };

  struct tbv_file_archive : shared::archive::file_archive
  {
    static bool is_supported(std::basic_istream<std::byte>& stream);

    bool stream_is_supported(std::basic_istream<std::byte>& stream) const override;

    std::vector<content_info> get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const override;

    void set_stream_position(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info) const override;

    void extract_file_contents(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info, std::basic_ostream<std::byte>& output) const override;
  };
}// namespace trophy_bass::vol

#endif//DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
