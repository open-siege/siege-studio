#ifndef EXT_SHARED_HPP
#define EXT_SHARED_HPP

#include <filesystem>
#include <span>
#include <utility>
#include <string_view>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/extension_module.hpp>

namespace siege
{
  template<std::size_t PairCount>
  HRESULT get_name_ranges(
    const std::array<std::pair<std::string_view, std::string_view>, PairCount>& ranges,
    std::size_t length,
    std::array<const char*, 2>* data,
    std::size_t* saved) noexcept
  {
    if ((length == 0 || data == nullptr) && saved)
    {
      *saved = ranges.size();
      return S_OK;
    }

    if (!data)
    {
      return E_POINTER;
    }

    auto i = 0u;

    length = std::clamp<std::size_t>(length, 0, ranges.size());

    for (; i < length; ++i)
    {

      data[i][0] = ranges[i].first.data();
      data[i][1] = ranges[i].second.data();
    }

    if (saved)
    {
      *saved = i;
    }

    return i == length ? S_OK : S_FALSE;
  }

  HRESULT executable_is_supported(const wchar_t* filename, const auto& verification_strings) noexcept
  {
    if (filename == nullptr)
    {
      return E_POINTER;
    }

    std::error_code last_error;

    if (!std::filesystem::exists(filename, last_error))
    {
      return E_INVALIDARG;
    }

    try
    {
      win32::file file(std::filesystem::path(filename), GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

      auto file_size = file.GetFileSizeEx();
      auto mapping = file.CreateFileMapping(std::nullopt, PAGE_READONLY, 0, 0, L"");

      if (mapping && file_size)
      {
        auto view = mapping->MapViewOfFile(FILE_MAP_READ, (std::size_t)file_size->QuadPart);
        std::string_view data((char*)view.get(), (std::size_t)file_size->QuadPart);

        if (data.empty())
        {
          return S_FALSE;
        }

        auto is_executable = data.find("MZ") == 0 && data.find("PE") != std::string_view::npos;

        bool has_all_verification_strings = is_executable && std::all_of(verification_strings.begin(), verification_strings.end(), [&](auto& item) {
          return data.find(item) != std::string_view::npos;
        });

        return has_all_verification_strings ? S_OK : S_FALSE;
      }

      return S_FALSE;
    }
    catch (...)
    {
      return S_FALSE;
    }
  }

  HRESULT executable_is_supported(const wchar_t* filename,
    const auto& verification_strings,
    const auto& function_name_ranges,
    const auto& variable_name_ranges) noexcept
  {
    if (filename == nullptr)
    {
      return E_POINTER;
    }

    std::error_code last_error;

    if (!std::filesystem::exists(filename, last_error))
    {
      return E_INVALIDARG;
    }

    try
    {
      win32::file file(std::filesystem::path(filename), GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

      auto file_size = file.GetFileSizeEx();
      auto mapping = file.CreateFileMapping(std::nullopt, PAGE_READONLY, 0, 0, L"");

      if (mapping && file_size)
      {
        auto view = mapping->MapViewOfFile(FILE_MAP_READ, (std::size_t)file_size->QuadPart);
        std::string_view data((char*)view.get(), (std::size_t)file_size->QuadPart);

        if (data.empty())
        {
          return S_FALSE;
        }

        auto is_executable = data.find("MZ") == 0 && data.find("PE") != std::string_view::npos;

        bool has_all_verification_strings = is_executable && std::all_of(verification_strings.begin(), verification_strings.end(), [&](auto& item) {
          return data.find(item.first) != std::string_view::npos;
        });

        bool has_all_functions = has_all_verification_strings && !function_name_ranges.empty() && std::all_of(function_name_ranges.begin(), function_name_ranges.end(), [&](auto& item) {
          auto first_index = data.find(item.first);

          if (first_index != std::string_view::npos)
          {
            return data.find(item.second, first_index) != std::string_view::npos;
          }
          return false;
        });

        bool has_all_variables = has_all_functions && !variable_name_ranges.empty() && std::all_of(variable_name_ranges.begin(), variable_name_ranges.end(), [&](auto& item) {
          auto first_index = data.find(item.first);

          if (first_index != std::string_view::npos)
          {
            return data.find(item.second, first_index) != std::string_view::npos;
          }
          return false;
        });

        return has_all_variables ? S_OK : S_FALSE;
      }

      return S_FALSE;
    }
    catch (...)
    {
      return S_FALSE;
    }
  }

}// namespace siege

#endif// !EXE_IS_SUPPORTED_HPP
