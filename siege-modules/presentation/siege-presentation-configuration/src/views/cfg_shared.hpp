#ifndef CFG_SHARED_HPP
#define CFG_SHARED_HPP

#include <siege/platform/shared.hpp>
#include <any>
#include <span>
#include <map>
#include <string>

namespace siege::views
{
  std::span<const siege::fs_string_view> get_cfg_formats() noexcept;
  bool is_cfg(std::istream& image_stream) noexcept;

  std::size_t load_config(std::any& state, std::istream& image_stream) noexcept;

  std::map<std::wstring, std::wstring> get_key_values(std::any& state);
}// namespace siege::views

#endif// CFG_SHARED_HPP
