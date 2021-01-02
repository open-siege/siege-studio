#ifndef DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
#define DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include "archive.hpp"
#include "endian_arithmetic.hpp"

namespace darkstar::vol
{
  struct vol_file_archive : shared::archive::file_archive
  {
    bool stream_is_supported(std::basic_istream<std::byte>& stream) const override;
    std::vector<content_info> get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const override;
    void set_stream_position(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info) const override;
    void extract_file_contents(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info, std::basic_ostream<std::byte>& output) const override;
  };
}// namespace darkstar::vol


#endif//DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
