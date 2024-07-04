#include <siege/platform/win/desktop/window_impl.hpp>

#include <bit>
#include <filesystem>
#include <cassert>
#include <atomic>
#include <system_error>
#include "views/vol_view.hpp"
#include <siege/platform/win/core/com/collection.hpp>
#include <siege/platform/win/core/com/stream_buf.hpp>
#include <siege/platform/resource_storage.hpp>
#include <siege/resource/resource_maker.hpp>

using namespace siege::views;

extern "C" {
extern const std::uint32_t default_file_icon = SIID_ZIPFILE;

std::errc __stdcall get_supported_extensions(std::size_t count, wchar_t const** strings, std::size_t* fetched) noexcept
{
  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  count = std::clamp<std::size_t>(count, 0u, vol_controller::formats.size());

  std::transform(vol_controller::formats.begin(), vol_controller::formats.begin() + count, strings, [](const std::wstring_view value) {
    return value.data();
  });

  if (fetched)
  {
    *fetched = count;
  }

  return std::errc(0);
}

std::errc __stdcall get_supported_format_categories(std::size_t count, const wchar_t** strings, std::size_t* fetched) noexcept
{
  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  static auto categories = std::array<std::wstring_view, 1>{ { L"All Archives" } };

  count = std::clamp<std::size_t>(count, 0u, categories.size());

  std::transform(categories.begin(), categories.begin() + count, strings, [](const std::wstring_view value) {
    return value.data();
  });

  if (fetched)
  {
    *fetched = count;
  }

  return std::errc(0);
}

std::errc __stdcall get_supported_extensions_for_category(const wchar_t* category, std::size_t count, const wchar_t** strings, std::size_t* fetched) noexcept
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

  if (category_str == L"All Archives")
  {
    count = std::clamp<std::size_t>(count, 0u, vol_controller::formats.size());

    std::transform(vol_controller::formats.begin(), vol_controller::formats.begin() + count, strings, [](const std::wstring_view value) {
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

std::errc __stdcall is_stream_supported(_In_ IStream* data) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  win32::com::StreamBufRef buffer(*data);
  std::istream stream(&buffer);

  if (siege::views::vol_controller::is_vol(stream))
  {
    return std::errc(0);
  }

  return std::errc::not_supported;
}

_Success_(return == S_OK || return == S_FALSE)
  std::errc __stdcall get_window_class_for_stream(_In_ IStream* data, _Outptr_ wchar_t** class_name) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  if (!class_name)
  {
    return std::errc::invalid_argument;
  }

  static std::wstring empty;
  *class_name = empty.data();

  win32::com::StreamBufRef buffer(*data);
  std::istream stream(&buffer);

  try
  {
    static auto this_module = win32::window_module_ref::current_module();

    if (siege::views::vol_controller::is_vol(stream))
    {
      static auto window_type_name = win32::type_name<siege::views::vol_view>();

      if (this_module.GetClassInfoExW(window_type_name))
      {
        *class_name = window_type_name.data();
        return std::errc(0);
      }
    }

    return std::errc::not_supported;
  }
  catch (...)
  {
    return std::errc::not_supported;
  }
}

std::errc __stdcall StreamIsStorage(_In_ IStream* data) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  win32::com::StreamBufRef buffer(*data);
  std::istream stream(&buffer);

  if (siege::resource::is_resource_reader(stream))
  {
    return std::errc(0);
  }

  return std::errc::not_supported;
}

std::errc __stdcall CreateStorageFromStream(IStream* data, ::IStorage** storage) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  if (!storage)
  {
    return std::errc::invalid_argument;
  }

  win32::com::StreamBufRef buffer(*data);
  std::istream temp_stream(&buffer);

  if (siege::resource::is_resource_reader(temp_stream))
  {
    auto final_stream = siege::platform::shallow_clone(*data);
    auto reader = siege::resource::make_resource_reader(*final_stream);

    auto temp = std::make_unique<siege::platform::OwningStorageReader<>>(
      std::move(final_stream),
      std::move(reader));

    *storage = temp.release();
    return std::errc(0);
  }

  *storage = nullptr;

  return std::errc::not_supported;
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,// handle to DLL module
  DWORD fdwReason,// reason for calling function
  LPVOID lpvReserved)// reserved
{

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    if (lpvReserved != nullptr)
    {
      return TRUE;// do not do cleanup if process termination scenario
    }

    static win32::hwnd_t info_instance = nullptr;

    static std::wstring module_file_name(255, '\0');
    GetModuleFileNameW(hinstDLL, module_file_name.data(), module_file_name.size());

    std::filesystem::path module_path(module_file_name.data());

    win32::window_module_ref this_module(hinstDLL);
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      this_module.RegisterClassExW(win32::window_meta_class<siege::views::vol_view>());
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      this_module.UnregisterClassW<siege::views::vol_view>();
    }
  }

  return TRUE;
}
}