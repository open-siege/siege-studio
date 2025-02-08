#include <algorithm>
#include <istream>
#include <string>
#include <map>
#include <siege/platform/stream.hpp>
#include <siege/platform/win/module.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/shared.hpp>
#include "views/exe_views.hpp"
#include <winreg.h>

namespace siege::views
{
  bool exe_controller::set_game_settings(const siege::platform::persistent_game_settings& settings)
  {
    OutputDebugStringW(L"set_game_settings\n");
    game_settings = settings;

    HKEY main_key = nullptr;
    HKEY user_key = nullptr;

    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_WRITE;

    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && 
        ::RegCreateKeyExW(user_key, L"Software\\The Siege Hub\\Siege Studio", 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      OutputDebugStringW(L"Failed successfully\n");
      std::vector<BYTE> raw_bytes;
      
      raw_bytes.resize(settings.last_ip_address.size() * sizeof(wchar_t));
      std::memcpy(raw_bytes.data(), settings.last_ip_address.data(), raw_bytes.size());
      
      bool result = false;
      result = ::RegSetValueExW(main_key, L"LastIPAddress", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;
      
      raw_bytes.resize(settings.last_player_name.size() * sizeof(wchar_t));
      std::memcpy(raw_bytes.data(), settings.last_player_name.data(), raw_bytes.size());
      result = result && ::RegSetValueExW(main_key, L"LastPlayerName", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;
     
      
      ::RegCloseKey(main_key);
      ::RegCloseKey(user_key);

      return result;
    }

    if (user_key)
    {
      OutputDebugStringW(L"User key being closed\n");
      ::RegCloseKey(user_key);
    }

    OutputDebugStringW(L"Could not save registry\n");
    return false;
  }

  const siege::platform::persistent_game_settings& exe_controller::get_game_settings()
  {
    HKEY user_key = nullptr;
    HKEY main_key = nullptr;
    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_WRITE;

    DWORD size = 0;
    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && 
        ::RegCreateKeyExW(user_key, L"Software\\The Siege Hub\\Siege Studio", 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      auto type = REG_SZ;
      size = (DWORD)game_settings.last_ip_address.size() * 2;
      ::RegGetValueW(main_key, nullptr, L"LastIPAddress", RRF_RT_REG_SZ, &type, game_settings.last_ip_address.data(), &size);
      size = game_settings.last_player_name.size() * 2;
      ::RegGetValueW(main_key, nullptr, L"LastPlayerName", RRF_RT_REG_SZ, &type, game_settings.last_player_name.data(), &size);
      ::RegCloseKey(main_key);
    }

    if (user_key)
    {
      ::RegCloseKey(user_key);
    }

    if (!game_settings.last_ip_address[0])
    {
      std::memcpy(game_settings.last_ip_address.data(), L"127.0.0.1", 10 * sizeof(wchar_t));
    }

    if (!game_settings.last_player_name[0])
    {
      size = game_settings.last_player_name.size();
      ::GetUserNameW(game_settings.last_player_name.data(), &size);
    }

    return game_settings;
  }


  bool exe_controller::is_exe(std::istream& stream)
  {
    auto position = stream.tellg();

    thread_local std::string data(1024, '\0');

    stream.read(data.data(), data.size());
    stream.seekg(position, std::ios::beg);

    if (data[0] == 'M' && data[1] == 'Z')
    {
      return data.find("PE") != std::string::npos;
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
            return ext.executable_is_supported(*path) == true;
          });
        }

        loaded_path = std::move(*path);
        return 1;
      }
    }

    return 0;
  }

  std::vector<std::string> get_strings(const std::filesystem::path& loaded_path, const win32::module& loaded_module)
  {
    std::vector<std::string> results;
    if (loaded_module)
    {
      auto file = win32::file(loaded_path, GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, 0);
      auto file_size = file.GetFileSizeEx();
      auto mapping = file.CreateFileMapping(std::nullopt, PAGE_READONLY, LARGE_INTEGER{}, L"");

      if (mapping && file_size)
      {
        results.reserve((std::size_t)file_size->QuadPart / 32);
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
                std::string final;

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
                  std::copy(first, second, std::back_inserter(final));

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

                  results.emplace_back(std::move(final));
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

  std::map<std::filesystem::path, std::vector<std::string>>& get_string_cache()
  {
    static std::map<std::filesystem::path, std::vector<std::string>> cache{};

    return cache;
  }

  std::set<std::string_view> get_function_names_for_range(auto functions, std::vector<std::string_view> strings)
  {
    std::set<std::string_view> results;
    for (auto& range : functions)
    {
      auto& [first, last] = range;

      std::set<std::size_t> first_indexes;
      std::map<std::size_t, std::size_t> range_sizes;

      for (auto first_iter = std::find(strings.begin(), strings.end(), first);
           first_iter != strings.end();
           first_iter = std::find(++first_iter, strings.end(), first))
      {
        first_indexes.emplace(std::distance(strings.begin(), first_iter));
      }

      for (auto index : first_indexes)
      {
        auto iter = strings.begin();
        std::advance(iter, index);
        auto last_iter = std::find(iter, strings.end(), last);

        if (last_iter == strings.end())
        {
          break;
        }

        range_sizes[std::distance(iter, last_iter)] = index;
      }

      if (!range_sizes.empty())
      {
        auto first_iter = strings.begin();
        std::advance(first_iter, range_sizes.begin()->second);
        auto last_iter = strings.begin();
        std::advance(last_iter, range_sizes.begin()->second);
        std::advance(last_iter, range_sizes.begin()->first + 1);
        std::copy_if(first_iter, last_iter, std::inserter(results, results.end()), [](std::string_view view) {
          if (view.empty())
          {
            return false;
          }

          if (view.contains("%"))
          {
            return false;
          }

          auto first_count = std::count(view.begin(), view.end(), view[0]);
          if (first_count == view.size())
          {
            return false;
          }

          auto space_count = std::count(view.begin(), view.end(), ' ');

          if (first_count == view.size() - space_count)
          {
            return false;
          }

          if (view.size() > 4 && std::ispunct((int)view[0]))
          {
            auto first_count = std::count(view.begin(), view.begin() + 2, view[0]);
            auto second_count = std::count(view.rbegin(), view.rbegin() + 2, view[0]);
            return first_count != second_count;
          }

          return true;
        });
      }
    }

    return results;
  }

  std::set<std::string_view> exe_controller::get_function_names() const
  {
    auto& cache = get_string_cache();
    auto existing = cache.find(loaded_path);

    if (existing == cache.end())
    {
      existing = cache.emplace(loaded_path, get_strings(loaded_path, loaded_module)).first;
    }

    std::set<std::string_view> results;

    if (matching_extension != extensions.end())
    {
      return get_function_names_for_range(matching_extension->get_function_name_ranges(),
        std::vector<std::string_view>(existing->second.begin(), existing->second.end()));
    }

    return results;
  }

  std::set<std::string_view> exe_controller::get_variable_names() const
  {
    auto& cache = get_string_cache();
    auto existing = cache.find(loaded_path);

    if (existing == cache.end())
    {
      existing = cache.emplace(loaded_path, get_strings(loaded_path, loaded_module)).first;
    }

    std::set<std::string_view> results;

    if (matching_extension != extensions.end())
    {
      return get_function_names_for_range(matching_extension->get_variable_name_ranges(),
        std::vector<std::string_view>(existing->second.begin(), existing->second.end()));
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

  std::optional<menu_info> exe_controller::get_resource_menu_items(std::wstring type, std::wstring name) const
  {
    auto resource = ::FindResourceW(loaded_module, name.c_str(), type.c_str());

    if (!resource)
    {
      return std::nullopt;
    }

    std::wstring raw_result;

    raw_result.resize(::SizeofResource(loaded_module, resource) / 2);

    if (raw_result.size() == 0)
    {
      return std::nullopt;
    }

    auto loaded_resource = ::LoadResource(loaded_module, resource);

    if (!loaded_resource)
    {
      return std::nullopt;
    }

    auto raw_resource = ::LockResource(loaded_resource);

    std::memcpy(raw_result.data(), raw_resource, raw_result.size() * 2);

    std::vector<std::wstring> results;

    menu_info result{};

    while (!raw_result.empty())
    {
      auto code = raw_result[0];
      raw_result.erase(0, 1);

      if (code == 0)
      {
        continue;
      }

      if (code < 1000 && code & MF_POPUP)
      {
        std::wstring text = raw_result.substr(0, raw_result.find(L'\0'));

        raw_result.erase(0, text.size() + 1);

        auto& menu_item = result.menu_items.emplace_back(std::move(text));

        while (!raw_result.empty())
        {
          auto state = raw_result[0];
          auto sub_id = raw_result[1];

          if (state < 1000 && state & MF_POPUP)
          {
            break;
          }

          raw_result.erase(0, 2);

          std::wstring sub_text = raw_result.substr(0, raw_result.find(L'\0'));
          raw_result.erase(0, sub_text.size() + 1);

          auto& sub_item = menu_item.sub_items.emplace_back(std::move(sub_text));

          if (state == 0 && sub_id == 0)
          {
            sub_item.fType = MF_SEPARATOR;
          }
          else
          {
            sub_item.wID = sub_id;
          }
        }
      }
    }

    return result;
  }

  std::vector<std::wstring> exe_controller::get_resource_strings(std::wstring type, std::wstring name) const
  {
    auto resource = ::FindResourceW(loaded_module, name.c_str(), type.c_str());

    if (!resource)
    {
      return {};
    }

    std::wstring raw_result;

    raw_result.resize(::SizeofResource(loaded_module, resource) / 2);

    if (raw_result.size() == 0)
    {
      return {};
    }

    auto loaded_resource = ::LoadResource(loaded_module, resource);

    if (!loaded_resource)
    {
      return {};
    }

    auto raw_resource = ::LockResource(loaded_resource);

    std::memcpy(raw_result.data(), raw_resource, raw_result.size() * 2);

    std::vector<std::wstring> results;
    results.reserve(16);

    while (!raw_result.empty())
    {
      auto size = raw_result[0];
      raw_result.erase(0, 1);

      if (size == 0)
      {
        continue;
      }
      results.emplace_back(raw_result.substr(0, size));

      raw_result.erase(0, size);
    }

    return results;
  }
}// namespace siege::views