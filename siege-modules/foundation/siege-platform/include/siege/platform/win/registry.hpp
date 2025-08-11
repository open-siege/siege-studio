#ifndef WIN_REGISTRY_MODULE_HPP
#define WIN_REGISTRY_MODULE_HPP

#include <siege/platform/win/auto_handle.hpp>
#include <expected>
#include <span>
#include <string_view>
#include <wtypes.h>
#include <WinDef.h>
#include <WinBase.h>

namespace win32
{
  enum reg_value_type : DWORD
  {

  };

  std::expected<std::pair<std::size_t, reg_value_type>, LSTATUS> reg_query_value_ex(HKEY key, std::string_view name);
  std::expected<std::pair<std::size_t, reg_value_type>, LSTATUS> reg_query_value_ex(HKEY key, std::wstring_view name);
  std::expected<std::span<wchar_t>, LSTATUS> reg_query_value_ex(HKEY key, std::wstring_view name, std::span<wchar_t> data);
  std::expected<std::span<char>, LSTATUS> reg_query_value_ex(HKEY key, std::string_view name, std::span<char> data);

}// namespace win32

#endif