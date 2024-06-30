#include <vector>
#include <algorithm>
#include <istream>
#include <string>
#include <siege/platform/stream.hpp>
#include "exe_controller.hpp"

namespace siege::views
{
  bool exe_controller::is_exe(std::istream& stream)
  {
    auto position = stream.tellg();

    thread_local std::vector<char> data(1024, '\0');

    stream.read(data.data(), data.size());
    stream.seekg(position, std::ios::beg);

    if (data[0] == 'M' && data[1] == 'Z')
    {
      auto e_iter = std::find(data.rbegin(), data.rend(), 'E');
      auto p_iter = std::find(e_iter, data.rend(), 'P');


      return e_iter != data.rend() && p_iter != data.rend() && std::distance(e_iter, p_iter) == 1;
    }

    return false;
  }

  std::size_t exe_controller::load_executable(std::istream& image_stream, std::optional<std::filesystem::path> path) noexcept
  {
    if (!path)
    {
      path = platform::get_stream_path(image_stream);
    }

    if (path)
    {
      loaded_module.reset(::LoadLibraryExW(path->c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE));

      if (loaded_module)
      {
        if (extensions.empty())
        {
          std::filesystem::path app_path = std::filesystem::path(win32::module_ref::current_application().GetModuleFileName()).parent_path();
          extensions = platform::game_extension_module::load_modules(app_path);

          matching_extension = std::find_if(extensions.begin(), extensions.end(), [&](platform::game_extension_module& ext) {
            return ext.ExecutableIsSupported(*path);
          });
        }

        return 1;
      }
    }

    return 0;
  }

  std::map<std::wstring, std::set<std::wstring>> exe_controller::get_resource_names() const
  {
    std::map<std::wstring, std::set<std::wstring>> results;

    if (loaded_module)
    {
      struct enumerator
      {
        static BOOL CALLBACK next_type(HMODULE hModule, LPWSTR lpType, LONG_PTR lParam)
        {
          auto* temp = (std::map<std::wstring, std::set<std::wstring>>*)lParam;

          if (IS_INTRESOURCE(lpType))
          {
            temp->emplace(L"#" + std::to_wstring((int)lpType), std::set<std::wstring>{});
            return TRUE;
          }


          temp->emplace(lpType, std::set<std::wstring>{});
          return TRUE;
        }

        static BOOL CALLBACK next_name(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
        {
          auto* temp = (std::set<std::wstring>*)lParam;
          if (IS_INTRESOURCE(lpName))
          {
            temp->emplace(L"#" + std::to_wstring((int)lpName));
            return TRUE;
          }

          temp->emplace(lpName);
          return TRUE;
        }
      };

      if (::EnumResourceTypesW(loaded_module, enumerator::next_type, (LONG_PTR)&results))
      {
        for (auto& result : results)
        {
          ::EnumResourceNamesW(loaded_module, result.first.c_str(), enumerator::next_name, (LONG_PTR)&result.second);
        }
      }
    }


    return results;
  }
}// namespace siege::views