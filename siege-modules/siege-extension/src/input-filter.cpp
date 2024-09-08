#include <siege/platform/win/core/file.hpp>
#include "input-filter.hpp"

namespace siege
{
  active_input_state& get_active_input_state()
  {
    static std::pair<win32::file_mapping, win32::file_view> mapping = [] {
      auto existing = ::OpenFileMappingW(FILE_MAP_WRITE, FALSE, L"SiegeAppActiveInputState");

      if (existing == nullptr)
      {
        auto mapping = win32::file_mapping(::CreateFileMappingW(
          INVALID_HANDLE_VALUE,// use paging file
          nullptr,// default security attributes
          PAGE_READWRITE,// read/write access
          0,// size: high 32-bits
          sizeof(active_input_state),// size: low 32-bits
          L"SiegeAppActiveInputState"));

        auto result = mapping.MapViewOfFile(FILE_MAP_WRITE, sizeof(sizeof(active_input_state)));
        new (result.get()) active_input_state{};
        return std::make_pair(std::move(mapping), std::move(result));
      }

      auto mapping = win32::file_mapping(existing);
      auto result = mapping.MapViewOfFile(FILE_MAP_WRITE, sizeof(sizeof(active_input_state)));
      return std::make_pair(std::move(mapping), std::move(result));
    }();

    return *(active_input_state*)mapping.second.get();
  }
}
