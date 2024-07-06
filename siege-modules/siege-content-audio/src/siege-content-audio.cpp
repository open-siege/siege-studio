#include <bit>
#include <filesystem>
#include <memory>
#include <system_error>
#include <algorithm>
#include <siege/platform/stream.hpp>
#include "views/sfx_controller.hpp"

using namespace std::literals;
using namespace siege::views;
using storage_info = siege::platform::storage_info;

extern "C" {
  std::errc get_supported_extensions(std::size_t count, const siege::fs_char** strings, std::size_t* fetched) noexcept
  {
    if (!strings)
    {
      if (fetched)
      {
        *fetched = 0;
      }

      return std::errc::invalid_argument;
    }

    count = std::clamp<std::size_t>(count, 0u, sfx_controller::formats.size());

    std::transform(sfx_controller::formats.begin(), sfx_controller::formats.begin() + count, strings, [](const auto value) {
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
      if (fetched)
      {
        *fetched = 0;
      }
      return std::errc::invalid_argument;
    }

    static auto categories = std::array<std::wstring_view, 1>{ { L"All Audio" } };

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

    if (category == L"All Audio"sv)
    {
      count = std::clamp<std::size_t>(count, 0u, sfx_controller::formats.size());

      std::transform(sfx_controller::formats.begin(), sfx_controller::formats.begin() + count, strings, [](const auto value) {
        return value.data();
      });

      if (fetched)
      {
        *fetched = count;
      }

      return std::errc(0);
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

    if (sfx_controller::is_sfx(*stream))
    {
      return std::errc(0);
    }

    return std::errc::not_supported;
  }
}