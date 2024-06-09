#ifndef EXE_IS_SUPPORTED_HPP
#define EXE_IS_SUPPORTED_HPP

#include <filesystem>
#include <span>
#include <utility>
#include <string_view>
#include <siege/platform/win/core/file.hpp>


namespace siege
{
  HRESULT ExecutableIsSupported(const wchar_t* filename, 
        const auto& verification_strings,
        const auto& function_name_ranges,
        const auto& variable_name_ranges) noexcept
  {
    if (filename == nullptr)
    {
      return E_POINTER;
    }

    if (!std::filesystem::exists(filename))
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

        bool has_all_functions = has_all_verification_strings && std::all_of(function_name_ranges.begin(), function_name_ranges.end(), [&](auto& item) {
          auto first_index = data.find(item.first);

          if (first_index != std::string_view::npos)
          {
            return data.find(item.second, first_index) != std::string_view::npos;
          }
          return false;
        });

        bool has_all_variables = has_all_functions && std::all_of(variable_name_ranges.begin(), variable_name_ranges.end(), [&](auto& item) {
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

}

#endif// !EXE_IS_SUPPORTED_HPP
