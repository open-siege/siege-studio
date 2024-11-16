#include <bit>
#include <filesystem>
#include <memory>
#include <system_error>
#include <algorithm>
#include <siege/platform/win/desktop/window_impl.hpp>
#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/core/hresult.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/shared.hpp>
#include "views/exe_views.hpp"

using namespace siege::views;
using storage_info = siege::platform::storage_info;

extern "C" {
extern const std::uint32_t default_file_icon = SIID_APPLICATION;

std::errc get_supported_extensions(std::size_t count, const siege::fs_char** strings, std::size_t* fetched) noexcept
{
  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  static std::vector<std::wstring_view> supported_extensions = [] {
    std::vector<std::wstring_view> extensions;
    extensions.reserve(16);

    std::copy(exe_controller::exe_formats.begin(), exe_controller::exe_formats.end(), std::back_inserter(extensions));
    std::copy(exe_controller::lib_formats.begin(), exe_controller::lib_formats.end(), std::back_inserter(extensions));
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

  static auto categories = std::array<std::u16string_view, 2>{ {
    u"All Executables",
    u"All Libraries",
  } };


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

std::errc get_supported_extensions_for_category(const char16_t* category, std::size_t count, const wchar_t** strings, std::size_t* fetched) noexcept
{
  if (!category)
  {
    return std::errc::invalid_argument;
  }

  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  std::u16string_view category_str = category;

  if (category_str == u"All Executables")
  {
    count = std::clamp<std::size_t>(count, 0u, exe_controller::exe_formats.size());

    std::transform(exe_controller::exe_formats.begin(), exe_controller::exe_formats.begin() + count, strings, [](const auto value) {
      return value.data();
    });
  }
  else if (category_str == u"All Libraries")
  {
    count = std::clamp<std::size_t>(count, 0u, exe_controller::lib_formats.size());

    std::transform(exe_controller::lib_formats.begin(), exe_controller::lib_formats.begin() + count, strings, [](const auto value) {
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

std::errc is_stream_supported(_In_ storage_info* data) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  auto stream = siege::platform::create_istream(*data);

  if (exe_controller::is_exe(*stream))
  {
    return std::errc(0);
  }

  return std::errc::not_supported;
}

HRESULT get_window_class_for_stream(storage_info* data, wchar_t** class_name) noexcept
{
  if (!data)
  {
    return E_INVALIDARG;
  }

  if (!class_name)
  {
    return E_INVALIDARG;
  }

  static std::wstring empty;
  *class_name = empty.data();

  auto stream = siege::platform::create_istream(*data);

  try
  {
    static auto this_module = win32::window_module_ref::current_module();

    if (exe_controller::is_exe(*stream))
    {
      static auto window_type_name = win32::type_name<exe_view>();

      if (this_module.GetClassInfoExW(window_type_name))
      {
        *class_name = window_type_name.data();
        return S_OK;
      }
    }

    return S_FALSE;
  }
  catch (...)
  {
    return S_FALSE;
  }
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    if (lpvReserved != nullptr)
    {
      return TRUE;// do not do cleanup if process termination scenario
    }

    win32::window_module_ref this_module(hinstDLL);

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      this_module.RegisterClassExW(win32::window_meta_class<exe_view>());
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      this_module.UnregisterClassW<exe_view>();
    }
  }

  return TRUE;
}
}