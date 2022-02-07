#ifndef DARKSTARDTSCONVERTER_DTS_IO_HPP
#define DARKSTARDTSCONVERTER_DTS_IO_HPP

#include <sstream>
#include <optional>
#include "darkstar_structures.hpp"

namespace studio::content::dts::darkstar
{
  bool is_darkstar_dts(std::basic_istream<std::byte>& stream);

  shape_variant read_shape(std::basic_istream<std::byte>& stream, std::optional<tag_header> file_header);

  shape_or_material_list read_shape(std::basic_istream<std::byte>& stream);

  void write_material_list(std::basic_ostream<std::byte>& stream, const material_list_variant&);

  void write_shape(std::basic_ostream<std::byte>& stream, const shape_variant&);
}// namespace darkstar::dts

#endif//DARKSTARDTSCONVERTER_DTS_IO_HPP
