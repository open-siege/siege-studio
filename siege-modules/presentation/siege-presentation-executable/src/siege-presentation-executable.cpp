#include <bit>
#include <filesystem>
#include <memory>
#include <system_error>
#include <algorithm>

#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/hresult.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/shared.hpp>
#include "views/exe_view.hpp"

using namespace siege::views;
using storage_info = siege::platform::storage_info;

static ATOM exe_view_atom;

namespace siege::views
{
  ATOM register_exe_view(win32::window_module_ref module);
}

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
    auto exe_formats = get_executable_formats();
    auto lib_formats = get_library_formats();

    std::copy(exe_formats.begin(), exe_formats.end(), std::back_inserter(extensions));
    std::copy(lib_formats.begin(), lib_formats.end(), std::back_inserter(extensions));
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
    auto exe_formats = get_executable_formats();

    count = std::clamp<std::size_t>(count, 0u, exe_formats.size());

    std::transform(exe_formats.begin(), exe_formats.begin() + count, strings, [](const auto value) {
      return value.data();
    });
  }
  else if (category_str == u"All Libraries")
  {
    auto lib_formats = get_library_formats();

    count = std::clamp<std::size_t>(count, 0u, lib_formats.size());

    std::transform(lib_formats.begin(), lib_formats.begin() + count, strings, [](const auto value) {
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

  if (is_exe_or_lib(*stream))
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

  auto stream = siege::platform::create_istream(*data);

  static auto this_module = win32::window_module_ref::current_module();

  if (!is_exe_or_lib(*stream))
  {
    return S_FALSE;
  }

  static std::wstring storage;

  if (!storage.empty())
  {
    *class_name = storage.data();
    return S_OK;
  }

  auto window = this_module.CreateWindowExW(CREATESTRUCTW{
    .hwndParent = HWND_MESSAGE,
    .lpszClass = MAKEINTATOM(exe_view_atom) });

  if (!window)
  {
    return S_FALSE;
  }

  storage.resize(255);
  storage.resize(::GetClassNameW(*window, storage.data(), (int)storage.size()));
  ::DestroyWindow(*window);
  window->release();
  *class_name = storage.data();
  return S_OK;
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

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      exe_view_atom = register_exe_view(win32::window_module_ref(hinstDLL));
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      ::UnregisterClassW(MAKEINTATOM(exe_view_atom), hinstDLL);
    }
  }

  return TRUE;
}
}