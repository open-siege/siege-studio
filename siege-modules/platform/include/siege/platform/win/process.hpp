#ifndef WIN_CORE_PROCESS_HPP
#define WIN_CORE_PROCESS_HPP

#include <SDKDDKVer.h>
#include <expected>
#include <string_view>
#include <string>
#include <optional>
#include <filesystem>
#include <expected>
#include <wtypes.h>
#include <WinDef.h>
#include <processthreadsapi.h>
#undef GetModuleFileName

namespace win32
{
  struct process_info
  {
    std::wstring_view application_name;
    std::wstring command_line;
    std::optional<SECURITY_ATTRIBUTES> process_attributes;
    std::optional<SECURITY_ATTRIBUTES> thread_attributes;
    bool inherit_handles;
    DWORD creation_flags;
    // TODO make this into a map of strings
    std::wstring environment;
    std::optional<std::filesystem::path> current_directory;
    STARTUPINFOW startup_info = { .cb = sizeof(STARTUPINFOW) };
  };

  inline std::expected<::PROCESS_INFORMATION, DWORD> CreateProcessW(process_info info)
  {
    ::PROCESS_INFORMATION process_data{};
    auto result = ::CreateProcessW(info.application_name.empty() ? nullptr : info.application_name.data(),
      info.command_line.empty() ? nullptr : info.command_line.data(),
      info.process_attributes ? &*info.process_attributes : nullptr,
      info.thread_attributes ? &*info.thread_attributes : nullptr,
      info.inherit_handles ? TRUE : FALSE,
      info.creation_flags,
      info.environment.empty() ? nullptr : info.environment.data(),
      info.current_directory ? info.current_directory->c_str() : nullptr,
        &info.startup_info,
        &process_data
    );

    if (!result)
    {
      return std::unexpected(::GetLastError());
    }

    return process_data;
  }
}// namespace win32

#endif// !WIN_CORE_MODULE_HPP
