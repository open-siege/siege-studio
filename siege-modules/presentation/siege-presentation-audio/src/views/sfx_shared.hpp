#ifndef SFX_SHARED_HPP
#define SFX_SHARED_HPP

#include <string>
#include <string_view>
#include <array>
#include <any>
#include <siege/platform/shared.hpp>

namespace siege::views
{
  std::span<const siege::fs_string_view> get_sfx_formats() noexcept;
  bool is_sfx(std::istream& image_stream) noexcept;

  std::size_t load_sound(std::any& state, std::istream& image_stream) noexcept;

  std::optional<std::filesystem::path> get_sound_path(std::any& state, std::size_t index);
  std::optional<std::span<std::byte>> get_sound_data(std::any& state, std::size_t index);
}// namespace siege::views

#endif// SFX_SHARED_HPP
