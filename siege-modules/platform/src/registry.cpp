#include <siege/platform/win/registry.hpp>

namespace win32
{
  std::expected<std::pair<std::size_t, reg_value_type>, LSTATUS> reg_query_value_ex(HKEY key, std::string_view name)
  {
    DWORD type = 0;
    DWORD size = 0;
    auto result = ::RegQueryValueExA(key, name.data(), nullptr, &type, nullptr, &size);

    if (result != ERROR_SUCCESS)
    {
      return std::unexpected(result);
    }

    return std::make_pair(size, reg_value_type{ type });
  }

  std::expected<std::span<char>, LSTATUS> reg_query_value_ex(HKEY key, std::string_view name, std::span<char> data)
  {
    DWORD size = (DWORD)data.size();
    auto result = ::RegQueryValueExA(key, name.data(), nullptr, nullptr, (BYTE*)data.data(), &size);

    if (result != ERROR_SUCCESS)
    {
      return std::unexpected(result);
    }

    return data;
  }

  std::expected<std::pair<std::size_t, reg_value_type>, LSTATUS> reg_query_value_ex(HKEY key, std::wstring_view name)
  {
    DWORD type = 0;
    DWORD size = 0;
    auto result = ::RegQueryValueExW(key, name.data(), nullptr, &type, nullptr, &size);

    if (result != ERROR_SUCCESS)
    {
      return std::unexpected(result);
    }

    return std::make_pair(size / sizeof(wchar_t), reg_value_type{ type });
  }
  std::expected<std::span<wchar_t>, LSTATUS> reg_query_value_ex(HKEY key, std::wstring_view name, std::span<wchar_t> data)
  {
    DWORD size = (DWORD)data.size_bytes();
    auto result = ::RegQueryValueExW(key, name.data(), nullptr, nullptr, (BYTE*)data.data(), &size);

    if (result != ERROR_SUCCESS)
    {
      return std::unexpected(result);
    }

    return data;
  }
}// namespace win32