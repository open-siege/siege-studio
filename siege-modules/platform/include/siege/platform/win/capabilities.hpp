#ifndef WIN32_CAPABILITIES_HPP
#define WIN32_CAPABILITIES_HPP
#include <optional>
#include <map>
#include <cstdint>
#include <siege/platform/win/com.hpp>
#include <versionhelpers.h>

namespace win32
{
  struct version
  {
    std::uint8_t major;
    std::uint8_t minor;
    std::uint8_t build;
    std::uint8_t revision;
  };

  std::optional<std::pair<version, std::wstring_view>> get_xinput_version();
  std::optional<version> get_gdi_plus_version();
  std::optional<version> get_wic_version();
  bool has_font(std::wstring_view text);
  bool has_segoe_emoji();
  std::optional<std::wstring_view> get_best_system_icon_font();
  std::optional<std::wstring_view> get_best_system_font();

  std::optional<std::wstring_view> get_segoe_symbol_name();
  
}// namespace win32

#endif