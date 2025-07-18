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
#include <detours.h>

namespace siege::views
{
  constexpr static std::size_t char_size = sizeof(siege::fs_char);

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
          std::filesystem::path app_path = std::filesystem::path(win32::module_ref::current_module().GetModuleFileName()).parent_path();
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

  // TODO use endian-specific members
  struct NEWHEADER
  {
    WORD Reserved;
    WORD ResType;
    WORD ResCount;
  };

  struct CURSORDIR
  {
    WORD Width;
    WORD Height;
  };

  struct ICONRESDIR
  {
    BYTE Width;
    BYTE Height;
    BYTE ColorCount;
    BYTE reserved;
  };

  struct RESDIR
  {
    union
    {
      ICONRESDIR Icon;
      CURSORDIR Cursor;
    };
    WORD Planes;
    WORD BitCount;
    DWORD BytesInRes;
    WORD IconCursorId;
  };

  std::vector<std::byte> exe_controller::get_resource_data(std::wstring type, std::wstring name, bool raw) const
  {
    std::optional<NEWHEADER> main_header;
    std::optional<RESDIR> icon_header;
    std::vector<std::byte> icon_data;

    constexpr static auto icon_type = L"#3";
    constexpr static auto icon_group_type = L"#14";

    if (type == icon_type && !raw)
    {
      auto groups = this->get_resource_names();

      auto icon_groups = groups.find(icon_group_type);

      if (icon_groups != groups.end())
      {
        for (auto& group : icon_groups->second)
        {
          if (main_header || icon_header)
          {
            break;
          }
          auto data = get_resource_data(icon_group_type, group, true);

          if (data.size() >= sizeof(NEWHEADER))
          {
            NEWHEADER header{};
            std::memcpy(&header, data.data(), sizeof(header));

            constexpr static auto real_size = sizeof(RESDIR) - sizeof(WORD);

            if (data.size() - sizeof(NEWHEADER) >= header.ResCount * real_size)
            {
              auto start = data.data() + sizeof(NEWHEADER);
              for (auto i = 0; i < header.ResCount; ++i)
              {
                RESDIR entry{};
                std::memcpy(&entry, start + (i * real_size), real_size);

                if (L"#" + std::to_wstring(entry.IconCursorId) == name)
                {
                  entry.IconCursorId = sizeof(header) + sizeof(entry);
                  main_header = header;
                  icon_header = entry;
                  break;
                }
              }
            }
          }
        }
      }
    }

    if (type == icon_group_type && !raw)
    {
      auto data = get_resource_data(icon_group_type, name, true);

      if (data.size() >= sizeof(NEWHEADER))
      {
        NEWHEADER header{};
        std::memcpy(&header, data.data(), sizeof(header));

        constexpr static auto real_size = sizeof(RESDIR) - sizeof(WORD);

        if (data.size() - sizeof(NEWHEADER) >= header.ResCount * real_size)
        {
          auto start = data.data() + sizeof(NEWHEADER);
          for (auto i = 0; i < header.ResCount; ++i)
          {
            RESDIR entry{};
            std::memcpy(&entry, start + (i * real_size), real_size);

            icon_data = get_resource_data(icon_type, L"#" + std::to_wstring(entry.IconCursorId), true);

            entry.IconCursorId = sizeof(header) + sizeof(entry);
            main_header = header;
            icon_header = entry;
            break;
          }
        }
      }
    }

    auto resource = ::FindResourceW(loaded_module, name.c_str(), type.c_str());

    if (!resource)
    {
      return {};
    }

    std::vector<std::byte> raw_result;

    if (main_header && icon_header)
    {
      if (icon_data.empty())
      {
        raw_result.resize(icon_header->IconCursorId + ::SizeofResource(loaded_module, resource));
      }
      else
      {
        raw_result.resize(icon_header->IconCursorId + icon_data.size());
      }
    }
    else
    {
      raw_result.resize(::SizeofResource(loaded_module, resource));
    }

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

    if (main_header && icon_header)
    {
      std::memcpy(raw_result.data(), &*main_header, sizeof(*main_header));
      std::memcpy(raw_result.data() + sizeof(*main_header), &*icon_header, sizeof(*icon_header));
      if (icon_data.empty())
      {
        std::memcpy(raw_result.data() + icon_header->IconCursorId, raw_resource, ::SizeofResource(loaded_module, resource));
      }
      else
      {
        std::memcpy(raw_result.data() + icon_header->IconCursorId, icon_data.data(), icon_data.size());
      }
    }
    else
    {
      std::memcpy(raw_result.data(), raw_resource, raw_result.size());
    }

    return raw_result;
  }

  std::optional<std::wstring> exe_controller::get_extension_for_name(std::wstring type, std::wstring name) const
  {
    static std::map<std::wstring_view, std::wstring_view> group_extensions = {
      { L"#1"sv, L".cur"sv },
      { L"#2"sv, L".bmp"sv },
      { L"#3"sv, L".ico"sv },
      { L"#10"sv, L".bin"sv },
      { L"#12"sv, L".cur"sv },
      { L"#14"sv, L".ico"sv },
      { L"#22"sv, L".ico"sv },
      { L"#23"sv, L".html"sv },
      { L"#24"sv, L".manifest"sv },
    };

    auto iter = group_extensions.find(type);

    if (iter != group_extensions.end())
    {
      return std::wstring(iter->second);
    }

    return std::nullopt;
  }

  std::array<char, 384> generate_zero_tier_node_id(std::filesystem::path zt_path)
  {
    std::error_code last_errorc;
    if (std::filesystem::exists(zt_path, last_errorc) && std::filesystem::exists(zt_path, last_errorc))
    {
      try
      {
        auto module = win32::module(zt_path);

        using id_new = int __cdecl(char* key, std::uint32_t* key_buf_len);

        auto* new_func = module.GetProcAddress<std::add_pointer_t<id_new>>("zts_id_new");

        if (new_func)
        {
          std::array<char, 384> node_id_and_private_key{};
          std::uint32_t size = node_id_and_private_key.size();
          new_func(node_id_and_private_key.data(), &size);
          return node_id_and_private_key;
        }
      }
      catch (...)
      {
      }
    }
    return {};
  }

  bool exe_controller::set_game_settings(const siege::platform::persistent_game_settings& settings)
  {
    auto node_id_and_private_key = game_settings.last_zero_tier_node_id_and_private_key;
    game_settings = settings;
    game_settings.last_zero_tier_node_id_and_private_key = node_id_and_private_key;

    HKEY main_key = nullptr;
    HKEY user_key = nullptr;

    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_WRITE;

    // TODO resolve the app name dynamically
    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, L"Software\\The Siege Hub\\Siege Studio", 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      std::vector<BYTE> raw_bytes;

      raw_bytes.resize(settings.last_ip_address.size() * char_size);
      std::memcpy(raw_bytes.data(), settings.last_ip_address.data(), raw_bytes.size());

      bool result = false;
      result = ::RegSetValueExW(main_key, L"LastIPAddress", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

      raw_bytes.resize(settings.last_zero_tier_network_id.size() * char_size);
      std::memcpy(raw_bytes.data(), settings.last_zero_tier_network_id.data(), raw_bytes.size());
      result = result && ::RegSetValueExW(main_key, L"LastZeroTierNetworkId", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

      std::string_view key_str = node_id_and_private_key.data();
      result = result && ::RegSetValueExA(main_key, "LastZeroTierNodeIdAndPrivateKey", 0, REG_SZ, (BYTE*)key_str.data(), key_str.size()) == ERROR_SUCCESS;

      raw_bytes.resize(settings.last_hosting_preference.size() * char_size);
      std::memcpy(raw_bytes.data(), settings.last_hosting_preference.data(), raw_bytes.size());
      result = result && ::RegSetValueExW(main_key, L"LastHostingPreference", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

      ::RegCloseKey(main_key);
      ::RegCloseKey(user_key);

      return result;
    }

    if (user_key)
    {
      ::RegCloseKey(user_key);
    }

    return false;
  }

  const siege::platform::persistent_game_settings& exe_controller::get_game_settings()
  {
    HKEY user_key = nullptr;
    HKEY main_key = nullptr;
    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_WRITE;

    DWORD size = 0;
    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, L"Software\\The Siege Hub\\Siege Studio", 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      auto type = REG_SZ;
      size = (DWORD)game_settings.last_ip_address.size() * char_size;
      ::RegGetValueW(main_key, nullptr, L"LastIPAddress", RRF_RT_REG_SZ, &type, game_settings.last_ip_address.data(), &size);
      size = game_settings.last_player_name.size() * char_size;
      ::RegGetValueW(main_key, nullptr, L"LastPlayerName", RRF_RT_REG_SZ, &type, game_settings.last_player_name.data(), &size);

      size = game_settings.last_zero_tier_network_id.size() * char_size;
      ::RegGetValueW(main_key, nullptr, L"LastZeroTierNetworkId", RRF_RT_REG_SZ, &type, game_settings.last_zero_tier_network_id.data(), &size);

      size = game_settings.last_zero_tier_node_id_and_private_key.size();
      ::RegGetValueA(main_key, nullptr, "LastZeroTierNodeIdAndPrivateKey", RRF_RT_REG_SZ, &type, game_settings.last_zero_tier_node_id_and_private_key.data(), &size);

      size = game_settings.last_hosting_preference.size() * char_size;
      ::RegGetValueW(main_key, nullptr, L"LastHostingPreference", RRF_RT_REG_SZ, &type, game_settings.last_hosting_preference.data(), &size);

      ::RegCloseKey(main_key);
    }

    if (user_key)
    {
      ::RegCloseKey(user_key);
    }

    if (!game_settings.last_ip_address[0])
    {
      std::memcpy(game_settings.last_ip_address.data(), L"0.0.0.0", 8 * char_size);
    }

    if (!game_settings.last_player_name[0])
    {
      size = game_settings.last_player_name.size();
      ::GetUserNameW(game_settings.last_player_name.data(), &size);
    }

    auto has_node_id = !std::all_of(game_settings.last_zero_tier_node_id_and_private_key.begin(), game_settings.last_zero_tier_node_id_and_private_key.end(), [](auto item) { return item == 0; });

    if (has_node_id)
    {
      ::SetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", game_settings.last_zero_tier_node_id_and_private_key.data());
    }
    else if (has_extension_module())
    {
      std::string extension_path = get_extension().GetModuleFileName<char>();
      auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
      game_settings.last_zero_tier_node_id_and_private_key = generate_zero_tier_node_id(zt_path);
      ::SetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", game_settings.last_zero_tier_node_id_and_private_key.data());
    }

    return game_settings;
  }

  bool exe_controller::can_support_zero_tier() const
  {
    if (has_extension_module())
    {
      return get_extension().caps->ip_connect_setting || get_extension().caps->dedicated_setting || get_extension().caps->listen_setting;
    }

    if (loaded_module)
    {
      static std::vector<std::string> names{
        "wsock32",
        "sdl2_net",
        "sdl_net",
      };

      win32::file file(loaded_path, GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

      auto file_size = file.GetFileSizeEx();

      constexpr static std::size_t max_file_size = 128 * 1024 * 1024;

      std::size_t clamped_file_size = 0;

      if (file_size)
      {
        clamped_file_size = std::clamp<std::size_t>((std::size_t)file_size->QuadPart, 0, max_file_size);
      }

      auto mapping = file.CreateFileMapping(std::nullopt, PAGE_READONLY, LARGE_INTEGER{ .QuadPart = clamped_file_size }, L"");

      if (mapping && file_size)
      {
        auto view = mapping->MapViewOfFile(FILE_MAP_READ, clamped_file_size);
        std::string_view data((char*)view.get(), clamped_file_size);

        for (auto& name : names)
        {
          if (data.find(name + ".DLL") != std::string_view::npos)
          {
            return true;
          }

          if (data.find(name + ".dll") != std::string_view::npos)
          {
            return true;
          }

          if (data.find(siege::platform::to_upper(name) + ".dll") != std::string_view::npos)
          {
            return true;
          }

          if (data.find(siege::platform::to_upper(name) + ".DLL") != std::string_view::npos)
          {
            return true;
          }

          if (data.find(name) != std::string_view::npos)
          {
            return true;
          }

          if (data.find(siege::platform::to_upper(name)) != std::string_view::npos)
          {
            return true;
          }
        }
      }
    }

    return false;
  }

  bool exe_controller::has_zero_tier_extension() const
  {
    if (has_extension_module())
    {
      std::string extension_path = get_extension().GetModuleFileName<char>();
      auto wsock_path = std::filesystem::path(extension_path).parent_path() / "wsock32-on-zero-tier.dll";
      auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
      std::error_code last_errorc;
      return std::filesystem::exists(wsock_path, last_errorc) && std::filesystem::exists(zt_path, last_errorc);
    }


    std::string extension_path = win32::module_ref::current_application().GetModuleFileName<char>();
    auto wsock_path = std::filesystem::path(extension_path).parent_path() / "wsock32-on-zero-tier.dll";
    auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
    std::error_code last_errorc;
    return std::filesystem::exists(wsock_path, last_errorc) && std::filesystem::exists(zt_path, last_errorc);
  }

  std::optional<std::filesystem::path> exe_controller::get_zero_tier_extension_folder_path() const
  {
    if (has_extension_module())
    {
      std::string extension_path = get_extension().GetModuleFileName<char>();
      auto wsock_path = std::filesystem::path(extension_path).parent_path() / "wsock32-on-zero-tier.dll";
      auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
      std::error_code last_errorc;
      if (std::filesystem::exists(wsock_path, last_errorc) && std::filesystem::exists(zt_path, last_errorc))
      {
        return std::filesystem::path(extension_path).parent_path();
      }
    }

    std::string extension_path = win32::module_ref::current_application().GetModuleFileName<char>();
    auto wsock_path = std::filesystem::path(extension_path).parent_path() / "wsock32-on-zero-tier.dll";
    auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
    std::error_code last_errorc;
    if (std::filesystem::exists(wsock_path, last_errorc) && std::filesystem::exists(zt_path, last_errorc))
    {
      return std::filesystem::path(extension_path).parent_path();
    }

    return std::nullopt;
  }

  using apply_prelaunch_settings = HRESULT(const wchar_t* exe_path_str, const siege::platform::game_command_line_args*);
  using format_command_line = const wchar_t**(const siege::platform::game_command_line_args*, std::uint32_t* new_size);
  bool allow_input_filtering = false;// TODO There are still some issues with id Tech 3 games that should be fixed.

  HRESULT exe_controller::launch_game_with_extension(const siege::platform::game_command_line_args* game_args, PROCESS_INFORMATION* process_info) noexcept
  {
    std::error_code last_errorc;

    if (!game_args)
    {
      return E_POINTER;
    }

    if (!process_info)
    {
      return E_POINTER;
    }

    auto configure_environment = [&]() {
      auto get_env = [](auto key) {
        auto size = ::GetEnvironmentVariableW(key, nullptr, 0);

        if (size == 0)
        {
          return std::wstring{};
        }

        std::wstring temp(size + 1, L'\0');

        temp.resize(::GetEnvironmentVariableW(key, temp.data(), temp.size()));
        return temp;
      };

      std::wstring current_path = get_env(L"Path");

      std::array<std::filesystem::path, 3> search_paths{ {
        get_env(L"SystemDrive") + L"//",
        get_env(L"ProgramFiles"),
        get_env(L"ProgramFiles(X86)"),
      } };

      for (auto& search_path : search_paths)
      {
        if (search_path.empty())
        {
          continue;
        }

        auto steam_path = (search_path / L"Steam").wstring();
        if (std::filesystem::exists(steam_path, last_errorc) && !current_path.contains(steam_path))
        {
          current_path = steam_path + L";" + current_path;
        }
      }

      auto zt_is_enabled = [](auto& item) {
        return item.name != nullptr && std::wstring_view(item.name) == L"ZERO_TIER_ENABLED" && item.value != nullptr && item.value[0] == '1';
      };

      if (has_zero_tier_extension() && std::any_of(game_args->environment_settings.begin(), game_args->environment_settings.end(), zt_is_enabled) && std::any_of(game_args->environment_settings.begin(), game_args->environment_settings.end(), [](auto& item) {
            return item.name != nullptr && std::wstring_view(item.name) == L"ZERO_TIER_NETWORK_ID" && item.value != nullptr && item.value[0] != '\0';
          }))
      {
        namespace fs = std::filesystem;

        auto ext_path = fs::path(win32::module_ref::current_application().GetModuleFileName()).parent_path() / "runtime-extensions";

        fs::remove_all(ext_path, last_errorc);
        fs::create_directories(ext_path, last_errorc);

        auto wsock_path = *get_zero_tier_extension_folder_path() / "wsock32-on-zero-tier.dll";
        auto zt_path = *get_zero_tier_extension_folder_path() / "zt-shared.dll";

        fs::copy_file(wsock_path, ext_path / "wsock32.dll", fs::copy_options::overwrite_existing, last_errorc);
        fs::copy_file(zt_path, ext_path / "zt-shared.dll", fs::copy_options::overwrite_existing, last_errorc);

        ::SetDllDirectoryW(ext_path.c_str());

        if (has_extension_module())
        {
          if (auto& caps = get_extension().caps; caps && caps->ip_connect_setting)
          {
            auto connect_str = std::wstring_view(caps->ip_connect_setting);

            auto setting = std::find_if(game_args->string_settings.begin(), game_args->string_settings.end(), [&](auto& item) {
              return item.name != nullptr && item.name == connect_str && item.value != nullptr && item.value[0] != '\0' && std::wstring_view(item.value) != L"0.0.0.0";
            });

            if (setting != game_args->string_settings.end())
            {
              ::SetEnvironmentVariableW(L"ZERO_TIER_FALLBACK_BROADCAST_IP_V4", setting->value);
            }
            else
            {
              ::SetEnvironmentVariableW(L"ZERO_TIER_FALLBACK_BROADCAST_IP_V4", nullptr);
            }
          }
        }
      }


      for (auto i = 0; i < game_args->environment_settings.size(); ++i)
      {
        if (!game_args->environment_settings[i].name)
        {
          break;
        }
        ::SetEnvironmentVariableW(game_args->environment_settings[i].name, game_args->environment_settings[i].value);
      }

      if (!std::any_of(game_args->environment_settings.begin(), game_args->environment_settings.end(), zt_is_enabled))
      {
        ::SetEnvironmentVariableW(L"ZERO_TIER_ENABLED", nullptr);
        ::SetEnvironmentVariableW(L"ZERO_TIER_NETWORK_ID", nullptr);
        ::SetEnvironmentVariableW(L"ZERO_TIER_FALLBACK_BROADCAST_IP_V4", nullptr);
      }

      ::SetEnvironmentVariableW(L"Path", current_path.c_str());

      return std::shared_ptr<void>(nullptr, [](...) { ::SetDllDirectoryW(nullptr); });
    };

    if (has_extension_module())
    {
      std::string extension_path = get_extension().GetModuleFileName<char>();

      auto* apply_prelaunch_settings_func = get_extension().GetProcAddress<std::add_pointer_t<apply_prelaunch_settings>>("apply_prelaunch_settings");

      if (apply_prelaunch_settings_func)
      {
        if (apply_prelaunch_settings_func(loaded_path.c_str(), game_args) != S_OK)
        {
          return E_ABORT;
        }
      }

      auto* format_command_line_func = get_extension().GetProcAddress<std::add_pointer_t<format_command_line>>("format_command_line");

      const wchar_t** argv = nullptr;
      std::uint32_t argc = 0;


      if (format_command_line_func)
      {
        argv = format_command_line_func(game_args, &argc);
      }

      STARTUPINFOW startup_info{ .cb = sizeof(STARTUPINFOW) };

      auto hook_path = (std::filesystem::path(extension_path).parent_path() / "siege-extension-input-filter-raw-input.dll").string();

      std::vector<const char*> dll_paths;

      if (allow_input_filtering)
      {
        dll_paths.emplace_back(hook_path.c_str());
      }

      auto* input_backends = get_extension().GetProcAddress<std::add_pointer_t<wchar_t*>>("controller_input_backends");

      if (input_backends && input_backends[0] && std::wstring_view(input_backends[0]) == get_extension().GetModuleFileName<wchar_t>())
      {
        dll_paths.emplace_back(extension_path.c_str());
      }

      auto* detour_ordinal = get_extension().GetProcAddress(1);

      if (detour_ordinal)
      {
        dll_paths.emplace_back(extension_path.c_str());
      }

      std::wstring args;
      args.reserve(argc + 3 * sizeof(std::wstring) + 3);


      auto exe_path = loaded_path;

      if (auto& caps = get_extension().caps; game_args && caps && caps->preferred_exe_setting && caps->preferred_exe_setting[0] != '\0')
      {
        auto preferred_game_exe = std::find_if(game_args->string_settings.begin(), game_args->string_settings.end(), [=](auto& item) {
          return item.name && item.value && item.value[0] != '\0' && item.name == std::wstring_view(caps->preferred_exe_setting);
        });

        if (preferred_game_exe != game_args->string_settings.end())
        {
          auto temp_path = exe_path;
          temp_path.replace_filename(preferred_game_exe->value);

          if (std::filesystem::exists(temp_path, last_errorc))
          {
            exe_path = temp_path;
          }
        }
      }

      args.append(1, L'"');
      args.append(exe_path.wstring());
      args.append(1, L'"');

      if (argv && argc > 0)
      {
        args.append(1, L' ');
        for (auto i = 0u; i < argc; ++i)
        {
          args.append(argv[i]);

          if (i < (argc - 1))
          {
            args.append(1, L' ');
          }
        }
      }

      auto deferred = configure_environment();

      if (dll_paths.empty() && ::CreateProcessW(exe_path.c_str(), args.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, loaded_path.parent_path().c_str(), &startup_info, process_info))
      {
        return S_OK;
      }
      else if (::DetourCreateProcessWithDllsW(exe_path.c_str(),
                 args.data(),
                 nullptr,
                 nullptr,
                 FALSE,
                 DETACHED_PROCESS,
                 nullptr,
                 loaded_path.parent_path().c_str(),
                 &startup_info,
                 process_info,
                 dll_paths.size(),
                 dll_paths.data(),
                 nullptr))
      {
        return S_OK;
      }

      auto last_error = ::GetLastError();

      return HRESULT_FROM_WIN32(last_error);
    }
    else
    {
      STARTUPINFOW startup_info{ .cb = sizeof(STARTUPINFOW) };


      std::wstring args;
      args.append(1, L'"');
      args.append(loaded_path.wstring());
      args.append(1, L'"');

      auto deferred = configure_environment();

      if (::CreateProcessW(loaded_path.c_str(), args.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, loaded_path.parent_path().c_str(), &startup_info, process_info))
      {
        return S_OK;
      }

      auto last_error = ::GetLastError();
      return HRESULT_FROM_WIN32(last_error);
    }
  }
}// namespace siege::views