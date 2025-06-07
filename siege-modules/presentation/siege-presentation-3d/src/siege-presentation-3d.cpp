#include <bit>
#include <filesystem>
#include <memory>
#include <system_error>
#include <algorithm>
#include <array>
#include <siege/platform/stream.hpp>
#include "views/dts_controller.hpp"
#include "views/dml_controller.hpp"

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
    extensions.reserve(8);

    std::copy(dts_controller::formats.begin(), dts_controller::formats.end(), std::back_inserter(extensions));
    std::copy(dml_controller::formats.begin(), dml_controller::formats.end(), std::back_inserter(extensions));

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

std::errc get_supported_format_categories(std::size_t count, const char16_t** strings, std::size_t* fetched) noexcept
{
  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  static auto categories = std::array<std::u16string_view, 1>{ { u"All 3D Models" } };

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

std::errc get_supported_extensions_for_category(const char16_t* category, std::size_t count, const siege::fs_char** strings, std::size_t* fetched) noexcept
{
  if (fetched)
  {
    *fetched = 0;
  }

  if (!category)
  {
    return std::errc::invalid_argument;
  }

  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  std::u16string_view category_str = category;

  if (category_str == u"All 3D Models")
  {
    count = std::clamp<std::size_t>(count, 0u, dts_controller::formats.size());

    std::transform(dts_controller::formats.begin(), dts_controller::formats.begin() + count, strings, [](const auto value) {
      return value.data();
    });

    if (fetched)
    {
      *fetched = count;
    }

    return std::errc(0);
  }
  if (fetched)
  {
    *fetched = 0;
  }

  return std::errc::not_supported;
}

std::errc is_stream_supported(storage_info* data) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  auto stream = siege::platform::create_istream(*data);

  if (dts_controller::is_shape(*stream))
  {
    return std::errc(0);
  }

  if (dml_controller::is_material(*stream))
  {
    return std::errc(0);
  }

  return std::errc::not_supported;
}

}