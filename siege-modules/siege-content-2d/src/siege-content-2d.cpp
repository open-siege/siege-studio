#include <bit>
#include <filesystem>
#include <memory>
#include <system_error>
#include <siege/platform/win/core/hresult.hpp>
#include <siege/platform/stream.hpp>

#include "views/bmp_controller.hpp"
#include "views/pal_controller.hpp"

using namespace siege::views;
using storage_info = siege::platform::storage_info;

extern "C" {
std::errc get_supported_extensions(std::size_t count, const siege::fs_char** strings, std::size_t* fetched) noexcept
{
  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  static std::vector<siege::fs_string_view> supported_extensions = [] {
    std::vector<siege::fs_string_view> extensions;
    extensions.reserve(32);

    std::copy(bmp_controller::formats.begin(), bmp_controller::formats.end(), std::back_inserter(extensions));
    std::copy(pal_controller::formats.begin(), pal_controller::formats.end(), std::back_inserter(extensions));

    return extensions;
  }();

  count = std::clamp<std::size_t>(count, 0u, supported_extensions.size());

  std::transform(supported_extensions.begin(), supported_extensions.begin() + count, strings, [](const auto value) {
    return value.data();
  });

  if (fetched)
  {
    *fetched = count;
  }
  return std::errc(0);
}

std::errc get_supported_format_categories(std::size_t count, const wchar_t** strings, std::size_t* fetched) noexcept
{
  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  static auto categories = std::array<std::wstring_view, 2>{ { L"All Images",
    L"All Palettes" } };

  count = std::clamp<std::size_t>(count, 0u, categories.size());

  std::transform(categories.begin(), categories.begin() + count, strings, [](const auto value) {
    return value.data();
  });

  if (fetched)
  {
    *fetched = count;
  }

  return std::errc(0);
}

std::errc get_supported_extensions_for_category(const wchar_t* category, std::size_t count, const siege::fs_char** strings, std::size_t* fetched) noexcept
{
  if (!category)
  {
    return std::errc::invalid_argument;
  }

  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  std::wstring_view category_str = category;

  if (category_str == L"All Images")
  {
    count = std::clamp<std::size_t>(count, 0u, bmp_controller::formats.size());

    std::transform(bmp_controller::formats.begin(), bmp_controller::formats.begin() + count, strings, [](const auto value) {
      return value.data();
    });
  }
  else if (category_str == L"All Palettes")
  {
    count = std::clamp<std::size_t>(count, 0u, pal_controller::formats.size());

    std::transform(pal_controller::formats.begin(), pal_controller::formats.begin() + count, strings, [](const auto value) {
      return value.data();
    });
  }
  else
  {
    count = 0;
  }

  if (fetched)
  {
    *fetched = count;
  }

  return count == 0 ? std::errc::not_supported : std::errc(0);
}

std::errc is_stream_supported(storage_info* data) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  auto stream = siege::platform::create_istream(*data);

  if (pal_controller::is_pal(*stream))
  {
    return std::errc(0);
  }

  if (bmp_controller::is_bmp(*stream))
  {
    return std::errc(0);
  }

  return std::errc::not_supported;
}
}
