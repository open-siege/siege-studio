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

        loaded_path = std::move(*path);
        return 1;
      }
    }

    return 0;
  }

  std::set<std::wstring> exe_controller::get_strings() const
  {
    std::set<std::wstring> results;

    if (loaded_module)
    {
      auto file = win32::file(loaded_path, GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, 0);
      auto file_size = file.GetFileSizeEx();
      auto mapping = file.CreateFileMapping(std::nullopt, PAGE_READONLY, 0, 0, L"");

      if (mapping && file_size)
      {
        auto view = mapping->MapViewOfFile(FILE_MAP_READ, (std::size_t)file_size->QuadPart);

        if (view)
        {
          std::string_view data((char*)view.get(), (std::size_t)file_size->QuadPart);

          auto is_whitespace = [](char raw) {
            auto value = static_cast<unsigned char>(raw);
            return std::isspace(value);
          };

          auto is_ascii = [](char raw) {
            auto value = static_cast<unsigned char>(raw);
            return std::isalpha(value) || std::isdigit(value) || std::isspace(value) || std::ispunct(value);
          };

          auto first = data.begin();
          auto second = data.begin() + 1;

          do
          {
            first = std::find_if(first, data.end(), is_ascii);

            if (first == data.end())
            {
              break;
            }

            second = first + 1;

            if (second == data.end())
            {
              break;
            }

            second = std::find_if(second, data.end(), [](char raw) {
              return raw == '\0';
            });

            if (second != data.end() && std::distance(first, second) > 1)
            {
              if (std::all_of(first, second, is_ascii) && !std::all_of(first, second, is_whitespace) && !(is_whitespace(*first) && first + 1 == second - 1))
              {
                std::wstring final;

                if (is_whitespace(*first))
                {
                  first += 1;
                }

                if (is_whitespace(*(second - 1)))
                {
                  second -= 1;
                }

                if (std::distance(first, second) > 4)
                {
                  final.reserve(std::distance(first, second));
                  std::transform(first, second, std::back_inserter(final), [](char raw) { return (wchar_t)raw; });

                  auto first_char = std::find_if(final.begin(), final.end(), [](wchar_t raw) { return !std::isspace(int(raw)); });
                  auto left_begin = std::find_if(final.begin(), first_char, [](wchar_t raw) { return std::isspace(int(raw)); });

                  if (left_begin != first_char)
                  {
                    final.erase(left_begin, first_char);
                  }

                  auto last_char = std::find_if(final.rbegin(), final.rend(), [](wchar_t raw) { return !std::isspace(int(raw)); });

                  auto right_begin = std::find_if(final.rbegin(), last_char, [](wchar_t raw) { return std::isspace(int(raw)); });

                  if (right_begin != last_char)
                  {
                    final.erase(last_char.base(), right_begin.base());
                  }

                  results.emplace(std::move(final));
                }
              }
            }

            if (second == data.end())
            {
              break;
            }

            first = second + 1;

            if (first == data.end())
            {
              break;
            }

          } while (first != data.end());
        }
      }
    }

    return results;
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