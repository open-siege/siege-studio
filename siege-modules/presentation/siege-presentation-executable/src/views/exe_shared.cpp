#include <siege/platform/stream.hpp>
#include <siege/platform/win/module.hpp>
#include <siege/platform/win/registry.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/shared.hpp>
#include <locale>
#include <codecvt>
#include <winreg.h>
#include <detours.h>
#include <nlohmann/json.hpp>
#include "exe_shared.hpp"

using json = nlohmann::json;

namespace siege::views
{
  struct registry_settings
  {
    std::array<fs_char, 64> last_player_name;
    std::array<fs_char, 64> last_ip_address;
    std::array<fs_char, 64> last_zero_tier_network_id;
    std::map<siege::fs_string, siege::fs_string> last_zero_tier_ip_addresses;

    std::array<fs_char, 64> last_hosting_preference;
    std::uint32_t zero_tier_enabled;

    // has to be 384 for zero tier to work
    std::array<char, 384> last_zero_tier_node_id_and_private_key;
  };

  constexpr static std::size_t char_size = sizeof(siege::fs_char);

  auto convert_to_string = [](auto& item) -> std::wstring {
    if constexpr (std::is_same_v<std::decay_t<decltype(item)>, bool>)
    {
      return item ? L"Yes" : L"No";
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(item)>, int>)
    {
      return std::to_wstring(item);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(item)>, float>)
    {
      return std::to_wstring(item);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(item)>, std::wstring>)
    {
      return item;
    }

    return L"";
  };


  auto enable_setting = [](game_setting& setting) {
    switch (setting.type)
    {
    case extension_setting_type::env_setting:
      [[fallthrough]];
    case extension_setting_type::string_setting: {
      setting.value = L"0.0.0.0";
      break;
    }
    case extension_setting_type::int_setting: {
      setting.value = 1;
      break;
    }
    case extension_setting_type::float_setting: {
      setting.value = 1.0f;
      break;
    }
    case extension_setting_type::flag_setting: {
      setting.value = true;
      break;
    }
    case extension_setting_type::computed_setting:
      [[fallthrough]];
    case extension_setting_type::unknown: {
      break;
    }
    }
  };

  auto disable_setting = [](game_setting& setting) {
    switch (setting.type)
    {
    case extension_setting_type::env_setting:
      [[fallthrough]];
    case extension_setting_type::string_setting: {
      setting.value = L"";
      break;
    }
    case extension_setting_type::int_setting: {
      setting.value = 0;
      break;
    }
    case extension_setting_type::float_setting: {
      setting.value = 0.0f;
      break;
    }
    case extension_setting_type::flag_setting: {
      setting.value = false;
      break;
    }
    case extension_setting_type::computed_setting:
      [[fallthrough]];
    case extension_setting_type::unknown: {
      break;
    }
    }
  };

  auto copy_to_array = [](auto& raw_value, auto& array) {
    auto value = std::visit(convert_to_string, raw_value);
    auto max_size = value.size() > array.size() ? array.size() : value.size();
    std::copy_n(value.data(), max_size, array.data());
  };

  void game_setting::update_value(int new_value, std::wstring_view new_display_value)
  {
    display_value = new_display_value;
    switch (type)
    {
    case extension_setting_type::env_setting: {
      value = std::to_wstring(new_value);
      break;
    }
    case extension_setting_type::string_setting: {
      value = std::to_wstring(new_value);
      break;
    }
    case extension_setting_type::int_setting: {
      value = new_value;
      break;
    }
    case extension_setting_type::float_setting: {
      value = (float)new_value;
      break;
    }
    case extension_setting_type::flag_setting: {
      value = new_value ? true : false;
      break;
    }
    case extension_setting_type::computed_setting: {
      value = new_value;
      break;
    }
    case extension_setting_type::unknown: {
      break;
    }
    }

    if (persist)
    {
      persist();
    }
  }

  void game_setting::update_value(std::wstring_view new_value, std::wstring_view new_display_value)
  {
    display_value = new_display_value;

    switch (type)
    {
    case extension_setting_type::env_setting: {
      value = std::wstring(new_value);
      break;
    }
    case extension_setting_type::string_setting: {
      value = std::wstring(new_value);
      break;
    }
    case extension_setting_type::int_setting: {
      value = std::stoi(std::wstring(new_value));
      break;
    }
    case extension_setting_type::float_setting: {
      value = std::stof(std::wstring(new_value));
      break;
    }
    case extension_setting_type::flag_setting: {
      value = new_value == L"Enabled";
      break;
    }
    case extension_setting_type::computed_setting: {
      value = std::wstring(new_value);
      break;
    }
    case extension_setting_type::unknown: {
      break;
    }
    }
    if (persist)
    {
      persist();
    }
  }

  std::wstring game_setting::get_computed_display_value()
  {
    if (!display_value.empty())
    {
      return display_value;
    }

    return std::visit(convert_to_string, value);
  }

  struct exe_state
  {
    std::filesystem::path loaded_path;
    win32::module loaded_module;
    std::optional<siege::platform::game_extension_module> matching_extension;
    registry_settings registry_data{};
    std::vector<game_setting> launch_settings = {};

    std::optional<extension_setting_type> dedicated_setting_type{};
    std::optional<extension_setting_type> listen_setting_type{};

    std::unique_ptr<siege::platform::game_command_line_args> final_args = std::make_unique<siege::platform::game_command_line_args>();
  };

  exe_state& get(std::any& cache)
  {
    if (cache.type() != typeid(std::shared_ptr<exe_state>))
    {
      auto temp = std::make_shared<exe_state>();
      cache = temp;
    }

    return *std::any_cast<std::shared_ptr<exe_state>>(cache);
  }

  const exe_state& get(const std::any& cache)
  {
    return *std::any_cast<const std::shared_ptr<exe_state>>(cache);
  }

  siege::platform::game_command_line_args& get_final_args(std::any& state)
  {
    auto& self = get(state);

    return *self.final_args;
  }

  std::span<const siege::fs_string_view> get_executable_formats() noexcept
  {
    constexpr static auto exe_formats = std::array<siege::fs_string_view, 2>{ { FSL ".exe", FSL ".com" } };
    return exe_formats;
  }

  std::span<const siege::fs_string_view> get_library_formats() noexcept
  {
    constexpr static auto lib_formats = std::array<siege::fs_string_view, 6>{ {
      FSL ".dll",
      FSL ".ocx",
      FSL ".olb",
      FSL ".lib",
      FSL ".asi",
      FSL ".ovl",
    } };

    return lib_formats;
  }

  bool has_extension_module(const std::any& state) { return get(state).matching_extension.has_value(); }
  siege::platform::game_extension_module& get_extension(std::any& state) { return *get(state).matching_extension; }
  const siege::platform::game_extension_module& get_extension(const std::any& state) { return *get(state).matching_extension; }

  std::filesystem::path get_exe_path(const std::any& state) { return get(state).loaded_path; }

  const registry_settings& load_game_settings(std::any& state);

  std::span<game_setting> init_launch_settings(std::any& state)
  {
    constexpr auto hosting_pref_name = L"MULTIPLAYER_HOSTING_PREFERENCE";
    constexpr static auto pref_options = std::array<std::wstring_view, 4>{ { L"Use Game UI", L"Client/Connect to Server", L"Listen/Host & Connect", L"Dedicated Server" } };
    constexpr static auto pref_options_keys = std::array<std::wstring_view, 4>{ { L"nothing", L"connect", L"listen", L"dedicated" } };

    siege::platform::game_command_line_caps empty_caps{};

    auto& settings = load_game_settings(state);

    auto& self = get(state);

    self.launch_settings.clear();
    auto& caps = self.matching_extension && self.matching_extension->caps ? *self.matching_extension->caps : empty_caps;

    bool has_ip = (caps.ip_connect_setting == nullptr || !std::wstring_view(caps.ip_connect_setting).empty());
    bool has_listen = (caps.listen_setting == nullptr || !std::wstring_view(caps.listen_setting).empty());
    bool has_dedicated = (caps.dedicated_setting == nullptr || !std::wstring_view(caps.dedicated_setting).empty());


    std::vector<std::wstring_view> real_options;
    real_options.reserve(4);
    for (auto i = 0; i < pref_options.size(); ++i)
    {
      if (i == 1 && !has_ip)
      {
        continue;
      }

      if (i == 2 && !has_listen)
      {
        continue;
      }
      if (i == 3 && !has_dedicated)
      {
        continue;
      }
      real_options.emplace_back(pref_options[i]);
    }

    auto has_networking = has_ip || has_listen || has_dedicated;

    if (has_networking)
    {
      auto index = 0;

      auto key = std::find(pref_options_keys.begin(), pref_options_keys.end(), std::wstring_view(settings.last_hosting_preference.data()));

      if (key != pref_options_keys.end())
      {
        index = std::distance(pref_options_keys.begin(), key);
      }

      self.launch_settings.emplace_back(game_setting{
        .setting_name = hosting_pref_name,
        .type = extension_setting_type::env_setting,
        .value = std::wstring{ pref_options[index] },
        .display_name = L"Hosting",
        .group_id = 1,
        .get_predefined_string = [real_options = std::move(real_options), results = std::vector<siege::platform::predefined_string>{}](auto name) mutable -> std::span<siege::platform::predefined_string> {
          if (name == hosting_pref_name)
          {
            if (!results.empty())
            {
              return results;
            }
            for (auto& string : real_options)
            {
              results.emplace_back(siege::platform::predefined_string{
                .label = string.data(),
                .value = string.data() });
            }
            return results;
          }

          return std::span<siege::platform::predefined_string>{};
        },
        .persist = [&self, &state]() {
          siege::platform::game_command_line_caps default_caps{};
          auto& caps = has_extension_module(state) && get_extension(state).caps ? *get_extension(state).caps : default_caps;

          std::wstring_view ip_setting = caps.ip_connect_setting ? caps.ip_connect_setting : L"";

          if (auto setting_iter = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [&](game_setting& setting) {
                return setting.setting_name == hosting_pref_name;
              });
            setting_iter != self.launch_settings.end())
          {
            auto value = std::visit(convert_to_string, setting_iter->value);

            auto key = std::find(pref_options.begin(), pref_options.end(), value);

            if (key != pref_options.end())
            {
              auto index = std::distance(pref_options.begin(), key);
              decltype(setting_iter->value) temp = std::wstring(pref_options_keys[index]);
              copy_to_array(temp, self.registry_data.last_hosting_preference);
            }

            if (caps.ip_connect_setting)
            {
              auto connect_setting = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [&](game_setting& setting) {
                return setting.type == extension_setting_type::string_setting && setting.setting_name == caps.ip_connect_setting;
              });

              auto should_connect = value == pref_options[1];// connect to server

              if (connect_setting != self.launch_settings.end())
              {
                connect_setting->enabled = should_connect;
                if (connect_setting->persist)
                {
                  connect_setting->persist();
                }
              }
            }

            if (self.listen_setting_type && caps.listen_setting)
            {
              auto listen_setting = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [&](game_setting& setting) {
                return setting.setting_name == caps.listen_setting;
              });

              bool enabled = value == pref_options[2];

              if (listen_setting != self.launch_settings.end())
              {
                if (enabled)
                {
                  enable_setting(*listen_setting);
                }
                enabled ? enable_setting(*listen_setting) : disable_setting(*listen_setting);
                if (listen_setting->persist)
                {
                  listen_setting->persist();
                }
              }
            }

            if (self.dedicated_setting_type && caps.dedicated_setting)// dedicated
            {
              auto dedicated_setting = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [&](game_setting& setting) {
                return setting.setting_name == caps.dedicated_setting;
              });

              auto enabled = value == pref_options[3];

              if (dedicated_setting != self.launch_settings.end())
              {
                enabled ? enable_setting(*dedicated_setting) : disable_setting(*dedicated_setting);

                if (dedicated_setting->persist)
                {
                  dedicated_setting->persist();
                }
              }
            }
          } } });
    }

    if (!has_extension_module(state))
    {
      self.launch_settings.emplace_back(game_setting{
        .setting_name = L"CMD_ARGS",
        .type = extension_setting_type::string_setting,
        .value = std::wstring{},
        .display_name = L"Command Line Arguments",
        .group_id = 2 });
    }

    if (can_support_zero_tier(state) && has_zero_tier_extension(state))
    {
      self.launch_settings.emplace_back(game_setting{
        .setting_name = L"ZERO_TIER_ENABLED",
        .type = extension_setting_type::env_setting,
        .value = (int)settings.zero_tier_enabled,
        .display_name = L"Enable Zero Tier",
        .group_id = 1,
        .get_predefined_int = [results = std::vector<siege::platform::predefined_int>{}](auto name) mutable -> std::span<siege::platform::predefined_int> {
          if (name == L"ZERO_TIER_ENABLED")
          {
            if (!results.empty())
            {
              return results;
            }

            results.emplace_back(siege::platform::predefined_int{
              .label = L"Yes",
              .value = 1 });

            results.emplace_back(siege::platform::predefined_int{
              .label = L"No",
              .value = 0 });
          }
          return std::span<siege::platform::predefined_int>{};
        },
        .persist = [&self]() {
          if (auto setting_iter = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [&](game_setting& setting) {
                return setting.setting_name == L"ZERO_TIER_ENABLED";
              });
            setting_iter != self.launch_settings.end())
          {
            self.registry_data.zero_tier_enabled = true;
          } } });

      auto network_id = std::wstring{ settings.last_zero_tier_network_id.data() };

      constexpr static std::wstring_view zt_network_id = L"ZERO_TIER_NETWORK_ID";

      self.launch_settings.emplace_back(game_setting{
        .setting_name = L"ZERO_TIER_NETWORK_ID",
        .type = extension_setting_type::env_setting,
        .value = network_id,
        .display_name = L"Zero Tier Network ID",
        .group_id = 1,
        .persist = [&self]() {
          if (auto setting_iter = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [&](game_setting& setting) {
                return setting.setting_name == zt_network_id;
              });
            setting_iter != self.launch_settings.end())
          {
            copy_to_array(setting_iter->value, self.registry_data.last_zero_tier_network_id);
          }
        } });


      self.launch_settings.emplace_back(game_setting{
        .setting_name = L"ZERO_TIER_LAST_NETWORK_IP_ADDRESS",
        .type = extension_setting_type::computed_setting,
        .value = settings.last_zero_tier_ip_addresses.contains(network_id) ? settings.last_zero_tier_ip_addresses.at(network_id) : std::wstring(),
        .display_name = L"Zero Tier Last Network IP Address",
        .group_id = 1,
      });

      std::string_view zt_node_id = settings.last_zero_tier_node_id_and_private_key.data();

      if (zt_node_id.contains(':'))
      {
        zt_node_id = zt_node_id.substr(0, zt_node_id.find(':'));

        self.launch_settings.emplace_back(game_setting{
          .setting_name = L"ZERO_TIER_PEER_ID",
          .type = extension_setting_type::computed_setting,
          .value = std::wstring(zt_node_id.begin(), zt_node_id.end()),
          .display_name = L"Zero Tier Node ID",
          .group_id = 1 });
      }

      if (!has_ip)
      {
        self.launch_settings.emplace_back(game_setting{
          .setting_name = zt_fallback_ip,
          .type = extension_setting_type::env_setting,
          .value = settings.last_ip_address.data(),
          .display_name = L"Fallback Broadcast IP",
          .group_id = 1,
          .persist = [&self]() {
            if (auto setting_iter = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [&](game_setting& setting) {
                  siege::platform::game_command_line_caps empty_caps{};
                  auto& caps = self.matching_extension && self.matching_extension->caps ? *self.matching_extension->caps : empty_caps;

                  std::wstring_view ip_setting = caps.ip_connect_setting ? caps.ip_connect_setting : L"";

                  if (ip_setting.empty())
                  {
                    return setting.setting_name == zt_fallback_ip;
                  }

                  return !ip_setting.empty() && setting.setting_name == ip_setting;
                });
              setting_iter != self.launch_settings.end())
            {
              copy_to_array(setting_iter->value, self.registry_data.last_ip_address);
            }
          } });
      }
    }

    std::wstring_view dedicated_setting = caps.dedicated_setting ? caps.dedicated_setting : L"";
    std::wstring_view listen_setting = caps.listen_setting ? caps.listen_setting : L"";

    for (auto& setting : caps.string_settings)
    {
      if (!setting)
      {
        break;
      }

      std::wstring_view player_name_setting = caps.player_name_setting ? caps.player_name_setting : L"";

      if (!player_name_setting.empty() && setting == player_name_setting)
      {
        self.launch_settings.emplace_back(game_setting{
          .setting_name = std::wstring(player_name_setting),
          .type = extension_setting_type::string_setting,
          .value = settings.last_player_name.data(),
          .display_name = L"Player Name",
          .group_id = has_ip ? 1 : 2,
          .persist = [&state, &self]() {
            siege::platform::game_command_line_caps default_caps{};
            auto& caps = has_extension_module(state) && get_extension(state).caps ? *get_extension(state).caps : default_caps;

            std::wstring_view player_name_setting = caps.player_name_setting ? caps.player_name_setting : L"";

            if (auto setting_iter = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [&](game_setting& setting) {
                  return !player_name_setting.empty() && setting.setting_name == player_name_setting;
                });
              setting_iter != self.launch_settings.end())
            {
              copy_to_array(setting_iter->value, self.registry_data.last_player_name);
            }
          } });

        continue;
      }

      std::wstring_view ip_connect_setting = caps.ip_connect_setting ? caps.ip_connect_setting : L"";

      if (!ip_connect_setting.empty() && setting == ip_connect_setting)
      {
        self.launch_settings.emplace_back(game_setting{
          .setting_name = std::wstring(ip_connect_setting),
          .type = extension_setting_type::string_setting,
          .value = settings.last_ip_address.data(),
          .display_name = L"Server IP Address",
          .group_id = 1 });

        if (!listen_setting.empty() && setting == listen_setting)
        {
          self.listen_setting_type = extension_setting_type::string_setting;
        }
        continue;
      }

      // has to be checked after in case one of the predefined settings are the same as these two.
      if (!dedicated_setting.empty() && setting == dedicated_setting)
      {
        self.dedicated_setting_type = extension_setting_type::string_setting;
        continue;
      }

      if (!listen_setting.empty() && setting == listen_setting)
      {
        self.listen_setting_type = extension_setting_type::string_setting;
        continue;
      }

      auto proc = get_extension(state).get_predefined_string_command_line_settings_proc;

      self.launch_settings.emplace_back(game_setting{
        .setting_name = setting,
        .type = extension_setting_type::string_setting,
        .value = L"",
        .display_name = setting,
        .group_id = 2 });

      if (proc)
      {
        self.launch_settings.back().get_predefined_string = [proc](std::wstring_view name) {
          auto result = proc(name.data());

          auto size = 0;

          auto* first = result;

          do
          {
            if (!first)
            {
              break;
            }
            if (!first->label)
            {
              break;
            }
            size++;
            first++;
          } while (first->label);

          return std::span(result, size);
        };
      }
    }


    auto is_visible = [&](auto& setting, extension_setting_type type) {
      if (!dedicated_setting.empty() && setting == dedicated_setting)
      {
        self.dedicated_setting_type = type;
        return false;
      }

      if (!listen_setting.empty() && setting == listen_setting)
      {
        self.listen_setting_type = type;
        return false;
      }

      return true;
    };

    for (auto& setting : caps.int_settings)
    {
      if (!setting)
      {
        break;
      }

      self.launch_settings.emplace_back(game_setting{
        .setting_name = setting,
        .type = extension_setting_type::int_setting,
        .value = L"",
        .display_name = setting,
        .visible = is_visible(setting, extension_setting_type::int_setting),
        .group_id = 2,
      });

      auto proc = get_extension(state).get_predefined_int_command_line_settings_proc;

      if (proc)
      {
        self.launch_settings.back().get_predefined_int = [proc](std::wstring_view name) {
          auto result = proc(name.data());

          auto size = 0;

          auto* first = result;

          do
          {
            if (!first)
            {
              break;
            }
            if (!first->label)
            {
              break;
            }
            size++;
            first++;
          } while (first->label);

          return std::span(result, size);
        };
      }
    }

    for (auto& setting : caps.float_settings)
    {
      if (!setting)
      {
        break;
      }

      self.launch_settings.emplace_back(game_setting{
        .setting_name = setting,
        .type = extension_setting_type::float_setting,
        .value = L"",
        .display_name = setting,
        .visible = is_visible(setting, extension_setting_type::float_setting),
        .group_id = 2 });
    }

    for (auto& setting : caps.flags)
    {
      if (!setting)
      {
        break;
      }

      self.launch_settings.emplace_back(game_setting{
        .setting_name = setting,
        .type = extension_setting_type::flag_setting,
        .value = L"",
        .display_name = setting,
        .visible = is_visible(setting, extension_setting_type::float_setting),
        .group_id = 2 });
    }


    static std::set<std::wstring> converted_strings;


    for (auto& setting : self.launch_settings)
    {
      setting.persist = [persist = std::move(setting.persist), &setting, &self]() {
        if (persist)
        {
          persist();
        }

        try
        {
          switch (setting.type)
          {
          case extension_setting_type::env_setting: {

            auto env_iter = std::find_if(self.final_args->environment_settings.begin(), self.final_args->environment_settings.end(), [&setting](auto& item) {
              return item.name && item.name == setting.setting_name;
            });

            if (env_iter == self.final_args->environment_settings.end())
            {
              env_iter = std::find_if(self.final_args->environment_settings.begin(), self.final_args->environment_settings.end(), [&setting](auto& item) {
                return !item.name;
              });
            }

            if (env_iter == self.final_args->environment_settings.end())
            {
              return;
            }

            if (!setting.enabled)
            {
              env_iter->name = nullptr;
              env_iter->value = nullptr;
              return;
            }

            env_iter->name = setting.setting_name.c_str();
            auto iter = converted_strings.emplace(std::visit(convert_to_string, setting.value));
            env_iter->value = iter.first->c_str();
            break;
          }
          case extension_setting_type::string_setting: {
            auto str_iter = std::find_if(self.final_args->string_settings.begin(), self.final_args->string_settings.end(), [&setting](auto& item) {
              return item.name && item.name == setting.setting_name;
            });

            if (str_iter == self.final_args->string_settings.end())
            {
              str_iter = std::find_if(self.final_args->string_settings.begin(), self.final_args->string_settings.end(), [&setting](auto& item) {
                return !item.name;
              });
            }

            if (str_iter == self.final_args->string_settings.end())
            {
              return;
            }

            if (!setting.enabled)
            {
              str_iter->name = nullptr;
              str_iter->value = nullptr;
              return;
            }

            str_iter->name = setting.setting_name.c_str();
            auto iter = converted_strings.emplace(std::visit(convert_to_string, setting.value));
            str_iter->value = iter.first->c_str();

            break;
          }
          case extension_setting_type::int_setting: {
            if (auto* value = std::get_if<int>(&setting.value); value)
            {
              auto int_iter = std::find_if(self.final_args->int_settings.begin(), self.final_args->int_settings.end(), [&setting](auto& item) {
                return item.name && item.name == setting.setting_name;
              });

              if (int_iter == self.final_args->int_settings.end())
              {
                int_iter = std::find_if(self.final_args->int_settings.begin(), self.final_args->int_settings.end(), [&setting](auto& item) {
                  return !item.name;
                });
              }

              if (int_iter == self.final_args->int_settings.end())
              {
                return;
              }


              if (!setting.enabled)
              {
                int_iter->name = nullptr;
                int_iter->value = 0;
                return;
              }

              int_iter->name = setting.setting_name.c_str();
              int_iter->value = *value;
            }
            break;
          }
          case extension_setting_type::float_setting: {
            if (auto* value = std::get_if<float>(&setting.value); value)
            {
              auto float_iter = std::find_if(self.final_args->float_settings.begin(), self.final_args->float_settings.end(), [&setting](auto& item) {
                return item.name && item.name == setting.setting_name;
              });

              if (float_iter == self.final_args->float_settings.end())
              {
                float_iter = std::find_if(self.final_args->float_settings.begin(), self.final_args->float_settings.end(), [&setting](auto& item) {
                  return !item.name;
                });
              }

              if (float_iter == self.final_args->float_settings.end())
              {
                return;
              }

              if (!setting.enabled)
              {
                float_iter->name = nullptr;
                float_iter->value = NAN;
                return;
              }

              float_iter->name = setting.setting_name.c_str();
              float_iter->value = *value;
            }
            break;
          }
          case extension_setting_type::flag_setting: {
            if (auto* value = std::get_if<bool>(&setting.value); value)
            {
              if (*value && setting.enabled)
              {
                auto flag_iter = std::find_if(self.final_args->flags.begin(), self.final_args->flags.end(), [&setting](auto& item) {
                  return item == setting.setting_name;
                });

                if (flag_iter != self.final_args->flags.end())
                {
                  return;
                }

                flag_iter = std::find_if(self.final_args->flags.begin(), self.final_args->flags.end(), [&setting](auto& item) {
                  return !item;
                });

                if (flag_iter == self.final_args->flags.end())
                {
                  return;
                }
                *flag_iter = setting.setting_name.c_str();
              }
              else
              {
                auto flag_iter = std::find_if(self.final_args->flags.begin(), self.final_args->flags.end(), [&setting](auto& item) {
                  return item && item == setting.setting_name;
                });

                if (flag_iter == self.final_args->flags.end())
                {
                  return;
                }
                *flag_iter = nullptr;
              }
            }
            break;
          }
          case extension_setting_type::computed_setting:
            [[fallthrough]];
          case extension_setting_type::unknown: {
            break;
          }
          }
        }
        catch (...)
        {
        }
      };
    }

    for (auto& setting : self.launch_settings)
    {
      if (setting.persist)
      {
        setting.persist();
      }
    }

    return self.launch_settings;
  }

  std::optional<std::reference_wrapper<game_setting>> get_game_setting(std::any& state, std::size_t index)
  {
    try
    {
      return std::ref(get(state).launch_settings.at(index));
    }
    catch (const std::out_of_range&)
    {
    }
    return std::nullopt;
  }

  struct networking_support
  {
    bool wsock_32;
    bool ws2_32;
    bool dplayx;
  };

  networking_support get_supported_networking_libraries(std::span<char> data);

  bool is_exe_or_lib(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    thread_local std::string data(1024, '\0');

    stream.read(data.data(), data.size());

    if (auto filename = siege::platform::get_stream_path(stream); data[0] == 'M' && data[1] == 'Z' && data.find("PE") != std::string::npos && filename)
    {
      auto module = ::LoadLibraryExW(filename->c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);

      if (!module)
      {
        return false;
      }

      std::shared_ptr<void> deferred = { nullptr,
        [=](...) { ::FreeLibrary(module); } };

      bool has_resource_data = false;

      struct handler
      {
        static BOOL __stdcall enum_types(HMODULE module, LPWSTR type, LONG_PTR lParam)
        {
          bool* has_data = (bool*)lParam;

          *has_data = true;
          return FALSE;
        }
      };

      ::EnumResourceTypesW(module, handler::enum_types, (LONG_PTR)&has_resource_data);


      if (!has_resource_data)
      {
        deferred.reset();

        win32::file file_to_read(*filename, GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

        auto mapping = file_to_read.CreateFileMapping(std::nullopt, PAGE_READONLY, {}, L"");

        auto size = file_to_read.GetFileSizeEx();

        if (!size)
        {
          return false;
        }

        if (size->QuadPart <= 1024)
        {
          return false;
        }

        size->QuadPart -= 1024;

        if (size->QuadPart >= 64 * 1024 * 1024)
        {
          return false;
        }

        auto view = mapping->MapViewOfFile(FILE_MAP_READ, LARGE_INTEGER{ .QuadPart = 1024 }, (std::size_t)size->QuadPart);

        auto results = get_supported_networking_libraries(view);

        return results.wsock_32 && !results.ws2_32 && !results.dplayx;
      }


      return has_resource_data;
    }

    return false;
  }

  std::size_t load_executable(std::any& state, std::istream& image_stream, std::optional<std::filesystem::path> path) noexcept
  {
    auto& self = get(state);
    if (!path)
    {
      path = platform::get_stream_path(image_stream);
    }

    if (!path)
    {
      return 0;
    }

    self.loaded_module.reset(::LoadLibraryExW(path->c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE));

    if (self.loaded_module)
    {
      std::filesystem::path app_path = std::filesystem::path(win32::module_ref::current_module().GetModuleFileName()).parent_path();
      auto extensions = platform::game_extension_module::load_modules(app_path, [path](const auto& ext) {
        return ext.executable_is_supported(*path) == true;
      });

      if (!extensions.empty())
      {
        self.matching_extension.emplace(std::move(extensions.front()));
      }

      self.loaded_path = std::move(*path);
      return 1;
    }

    return 0;
  }

  std::vector<std::string> get_strings(const std::filesystem::path& loaded_path, const win32::module& loaded_module)
  {
    if (!loaded_module)
    {
      return {};
    }
    std::vector<std::string> results;

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

  std::set<std::string_view> get_function_names(const std::any& state)
  {
    auto& self = get(state);
    auto& cache = get_string_cache();
    auto existing = cache.find(self.loaded_path);

    if (existing == cache.end())
    {
      existing = cache.emplace(self.loaded_path, get_strings(self.loaded_path, self.loaded_module)).first;
    }

    std::set<std::string_view> results;

    if (self.matching_extension)
    {
      return get_function_names_for_range(self.matching_extension->get_function_name_ranges(),
        std::vector<std::string_view>(existing->second.begin(), existing->second.end()));
    }

    return results;
  }

  std::set<std::string_view> get_variable_names(const std::any& state)
  {
    auto& self = get(state);
    auto& cache = get_string_cache();
    auto existing = cache.find(self.loaded_path);

    if (existing == cache.end())
    {
      existing = cache.emplace(self.loaded_path, get_strings(self.loaded_path, self.loaded_module)).first;
    }

    std::set<std::string_view> results;

    if (self.matching_extension)
    {
      return get_function_names_for_range(self.matching_extension->get_variable_name_ranges(),
        std::vector<std::string_view>(existing->second.begin(), existing->second.end()));
    }

    return results;
  }

  std::map<std::wstring, std::set<std::wstring>> get_resource_names(const std::any& state)
  {
    auto& self = get(state);
    std::map<std::wstring, std::set<std::wstring>> results;

    if (!self.loaded_module)
    {
      return results;
    }

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

    if (::EnumResourceTypesW(self.loaded_module, enumerator::next_type, (LONG_PTR)&results))
    {
      for (auto& result : results)
      {
        ::EnumResourceNamesW(self.loaded_module, result.first.c_str(), enumerator::next_name, (LONG_PTR)&result.second);
      }
    }

    return results;
  }

  std::optional<menu_info> get_resource_menu_items(const std::any& state, std::wstring type, std::wstring name)
  {
    auto& self = get(state);
    auto resource = ::FindResourceW(self.loaded_module, name.c_str(), type.c_str());

    if (!resource)
    {
      return std::nullopt;
    }

    std::wstring raw_result;

    raw_result.resize(::SizeofResource(self.loaded_module, resource) / 2);

    if (raw_result.size() == 0)
    {
      return std::nullopt;
    }

    auto loaded_resource = ::LoadResource(self.loaded_module, resource);

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

  std::vector<std::wstring> get_resource_strings(const std::any& state, std::wstring type, std::wstring name)
  {
    auto& self = get(state);
    auto resource = ::FindResourceW(self.loaded_module, name.c_str(), type.c_str());

    if (!resource)
    {
      return {};
    }

    std::wstring raw_result;

    raw_result.resize(::SizeofResource(self.loaded_module, resource) / 2);

    if (raw_result.size() == 0)
    {
      return {};
    }

    auto loaded_resource = ::LoadResource(self.loaded_module, resource);

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

  std::vector<std::byte> get_resource_data(const std::any& state, std::wstring type, std::wstring name, bool raw)
  {
    auto& self = get(state);
    std::optional<NEWHEADER> main_header;
    std::optional<RESDIR> icon_header;
    std::vector<std::byte> icon_data;

    constexpr static auto icon_type = L"#3";
    constexpr static auto icon_group_type = L"#14";

    if (type == icon_type && !raw)
    {
      auto groups = get_resource_names(state);

      auto icon_groups = groups.find(icon_group_type);

      if (icon_groups != groups.end())
      {
        for (auto& group : icon_groups->second)
        {
          if (main_header || icon_header)
          {
            break;
          }
          auto data = get_resource_data(state, icon_group_type, group, true);

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
      auto data = get_resource_data(state, icon_group_type, name, true);

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

            icon_data = get_resource_data(state, icon_type, L"#" + std::to_wstring(entry.IconCursorId), true);

            entry.IconCursorId = sizeof(header) + sizeof(entry);
            main_header = header;
            icon_header = entry;
            break;
          }
        }
      }
    }

    auto resource = ::FindResourceW(self.loaded_module, name.c_str(), type.c_str());

    if (!resource)
    {
      return {};
    }

    std::vector<std::byte> raw_result;

    if (main_header && icon_header)
    {
      if (icon_data.empty())
      {
        raw_result.resize(icon_header->IconCursorId + ::SizeofResource(self.loaded_module, resource));
      }
      else
      {
        raw_result.resize(icon_header->IconCursorId + icon_data.size());
      }
    }
    else
    {
      raw_result.resize(::SizeofResource(self.loaded_module, resource));
    }

    if (raw_result.size() == 0)
    {
      return {};
    }

    auto loaded_resource = ::LoadResource(self.loaded_module, resource);

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
        std::memcpy(raw_result.data() + icon_header->IconCursorId, raw_resource, ::SizeofResource(self.loaded_module, resource));
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

  std::optional<std::wstring> get_extension_for_name(const std::any& state, std::wstring type, std::wstring name)
  {
    using namespace std::literals;

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

  bool store_registry_data(std::any& state);

  bool set_registry_data(std::any& state, const registry_settings& settings)
  {
    auto& self = get(state);
    auto node_id_and_private_key = self.registry_data.last_zero_tier_node_id_and_private_key;
    self.registry_data = settings;
    self.registry_data.last_zero_tier_node_id_and_private_key = node_id_and_private_key;

    store_registry_data(state);
    return false;
  }

  std::string wstring_to_utf8(const std::wstring& wstr)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
  }

  std::wstring utf8_to_wstring(const std::string& str)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
  }

  bool store_registry_data(std::any& state)
  {
    auto& self = get(state);
    HKEY main_key = nullptr;
    HKEY user_key = nullptr;

    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_WRITE;

    // TODO resolve the app name dynamically
    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, L"Software\\The Siege Hub\\Siege Studio", 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      std::vector<BYTE> raw_bytes;

      auto& settings = self.registry_data;
      raw_bytes.resize(settings.last_ip_address.size() * char_size);
      std::memcpy(raw_bytes.data(), settings.last_ip_address.data(), raw_bytes.size());

      bool result = false;
      result = ::RegSetValueExW(main_key, L"LastIPAddress", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

      raw_bytes.resize(settings.last_player_name.size() * char_size);
      std::memcpy(raw_bytes.data(), settings.last_player_name.data(), raw_bytes.size());
      result = result && ::RegSetValueExW(main_key, L"LastPlayerName", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

      raw_bytes.resize(settings.last_zero_tier_network_id.size() * char_size);
      std::memcpy(raw_bytes.data(), settings.last_zero_tier_network_id.data(), raw_bytes.size());
      result = result && ::RegSetValueExW(main_key, L"LastZeroTierNetworkId", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

      auto map = json::object();
      for (auto& item : settings.last_zero_tier_ip_addresses)
      {
        map.emplace(wstring_to_utf8(item.first), wstring_to_utf8(item.second));
      }

      auto data = map.dump();
      raw_bytes.resize(data.size());
      std::memcpy(raw_bytes.data(), data.data(), raw_bytes.size());
      result = result && ::RegSetValueExA(main_key, "LastZeroTierIpAddressesForNetwork", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

      std::string_view key_str = settings.last_zero_tier_node_id_and_private_key.data();
      result = result && ::RegSetValueExA(main_key, "LastZeroTierNodeIdAndPrivateKey", 0, REG_SZ, (BYTE*)key_str.data(), key_str.size()) == ERROR_SUCCESS;

      raw_bytes.resize(settings.last_hosting_preference.size() * char_size);
      std::memcpy(raw_bytes.data(), settings.last_hosting_preference.data(), raw_bytes.size());
      result = result && ::RegSetValueExW(main_key, L"LastHostingPreference", 0, REG_SZ, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

      raw_bytes.resize(sizeof(settings.zero_tier_enabled));
      std::memcpy(raw_bytes.data(), &settings.zero_tier_enabled, raw_bytes.size());
      result = result && ::RegSetValueExW(main_key, L"ZeroTierEnabled", 0, REG_DWORD, raw_bytes.data(), raw_bytes.size()) == ERROR_SUCCESS;

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


  const registry_settings& load_game_settings(std::any& state)
  {
    auto& self = get(state);
    HKEY user_key = nullptr;
    HKEY main_key = nullptr;
    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_WRITE;

    DWORD size = 0;
    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, L"Software\\The Siege Hub\\Siege Studio", 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      auto& game_settings = self.registry_data;
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

      try
      {
        std::string buffer;

        buffer.resize(win32::reg_query_value_ex(main_key, "LastZeroTierIpAddressesForNetwork").value_or({}).first);

        if (auto data = win32::reg_query_value_ex(main_key, "LastZeroTierIpAddressesForNetwork", buffer); data)
        {
          auto map = json::parse(buffer).template get<std::map<std::string, std::string>>();

          for (auto& item : map)
          {
            game_settings.last_zero_tier_ip_addresses.emplace(utf8_to_wstring(item.first), utf8_to_wstring(item.second));
          }
        }
      }
      catch (...)
      {
      }

      size = sizeof(game_settings.zero_tier_enabled);
      type = REG_DWORD;
      ::RegGetValueW(main_key, nullptr, L"ZeroTierEnabled", RRF_RT_DWORD, &type, &game_settings.zero_tier_enabled, &size);
      ::RegCloseKey(main_key);
    }

    if (user_key)
    {
      ::RegCloseKey(user_key);
    }

    if (!self.registry_data.last_ip_address[0])
    {
      std::memcpy(self.registry_data.last_ip_address.data(), L"0.0.0.0", 8 * char_size);
    }

    if (!self.registry_data.last_player_name[0])
    {
      size = self.registry_data.last_player_name.size();
      ::GetUserNameW(self.registry_data.last_player_name.data(), &size);
    }

    auto has_node_id = !std::all_of(self.registry_data.last_zero_tier_node_id_and_private_key.begin(), self.registry_data.last_zero_tier_node_id_and_private_key.end(), [](auto item) { return item == 0; });

    if (has_node_id)
    {
      ::SetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", self.registry_data.last_zero_tier_node_id_and_private_key.data());
    }
    else if (has_extension_module(state))
    {
      std::string extension_path = get_extension(state).GetModuleFileName<char>();
      auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
      self.registry_data.last_zero_tier_node_id_and_private_key = generate_zero_tier_node_id(zt_path);
      ::SetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", self.registry_data.last_zero_tier_node_id_and_private_key.data());
    }
    else
    {
      std::string extension_path = win32::module_ref::current_module().GetModuleFileName<char>();
      auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
      self.registry_data.last_zero_tier_node_id_and_private_key = generate_zero_tier_node_id(zt_path);
      ::SetEnvironmentVariableA("ZERO_TIER_PEER_ID_AND_KEY", self.registry_data.last_zero_tier_node_id_and_private_key.data());
    }

    return self.registry_data;
  }

  void set_ip_for_current_network(std::any& state, std::string ip_address)
  {
    auto& self = get(state);
    std::wstring_view network_id = self.registry_data.last_zero_tier_network_id.data();

    if (!network_id.empty() && !ip_address.empty())
    {
      std::wstring temp;
      temp.reserve(ip_address.size());
      std::transform(ip_address.begin(), ip_address.end(), std::back_inserter(temp), [](auto value) { return (wchar_t)value; });

      self.registry_data.last_zero_tier_ip_addresses[std::wstring(network_id)] = temp;
    }

    store_registry_data(state);

    // TODO the split state doesn't make sense.
    // Better to unify the state and separate things from the UI a bit better
    auto setting_iter = std::find_if(self.launch_settings.begin(), self.launch_settings.end(), [](auto& item) {
      return item.setting_name == L"ZERO_TIER_LAST_NETWORK_IP_ADDRESS";
    });

    if (setting_iter != self.launch_settings.end())
    {
      std::wstring temp;
      temp.resize(ip_address.size());

      std::transform(ip_address.begin(), ip_address.end(), temp.begin(), [](char value) {
        return (wchar_t)value;
      });

      setting_iter->update_value(temp);
    }
  }

  networking_support get_supported_networking_libraries(std::span<char> data)
  {
    networking_support result{};
    auto supports_name = [data = std::string_view(data)](std::string name) {
      if (data.find(name + ".DLL") != std::string_view::npos)
      {
        return true;
      }

      if (data.find(name + ".dll") != std::string_view::npos)
      {
        return true;
      }

      name[0] = (char)std::toupper(name[0]);
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

      return false;
    };

    result.wsock_32 = supports_name("wsock32");
    result.ws2_32 = supports_name("ws2_32");
    result.dplayx = supports_name("dplayx");

    return result;
  }

  bool can_support_zero_tier(const std::any& state)
  {
    auto& self = get(state);
    if (self.matching_extension)
    {
      return get_extension(state).caps->ip_connect_setting || get_extension(state).caps->dedicated_setting || get_extension(state).caps->listen_setting;
    }

    if (self.loaded_module)
    {

      win32::file file(self.loaded_path, GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

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
        auto result = get_supported_networking_libraries(view);

        return result.wsock_32 && !result.ws2_32 && !result.dplayx;
      }
    }

    return false;
  }

  bool has_zero_tier_extension(const std::any& state)
  {
    auto& self = get(state);
    if (self.matching_extension)
    {
      std::string extension_path = get_extension(state).GetModuleFileName<char>();
      auto wsock_path = std::filesystem::path(extension_path).parent_path() / "wsock32-on-zero-tier.dll";
      auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
      std::error_code last_errorc;
      return std::filesystem::exists(wsock_path, last_errorc) && std::filesystem::exists(zt_path, last_errorc);
    }


    std::string extension_path = win32::module_ref::current_module().GetModuleFileName<char>();
    auto wsock_path = std::filesystem::path(extension_path).parent_path() / "wsock32-on-zero-tier.dll";
    auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
    std::error_code last_errorc;
    return std::filesystem::exists(wsock_path, last_errorc) && std::filesystem::exists(zt_path, last_errorc);
  }

  std::optional<std::filesystem::path> get_zero_tier_extension_folder_path(const std::any& state)
  {
    auto& self = get(state);
    if (self.matching_extension)
    {
      std::string extension_path = get_extension(state).GetModuleFileName<char>();
      auto wsock_path = std::filesystem::path(extension_path).parent_path() / "wsock32-on-zero-tier.dll";
      auto zt_path = std::filesystem::path(extension_path).parent_path() / "zt-shared.dll";
      std::error_code last_errorc;
      if (std::filesystem::exists(wsock_path, last_errorc) && std::filesystem::exists(zt_path, last_errorc))
      {
        return std::filesystem::path(extension_path).parent_path();
      }
    }

    std::string extension_path = win32::module_ref::current_module().GetModuleFileName<char>();
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

  HRESULT launch_game_with_extension(std::any& state, const siege::platform::game_command_line_args* game_args, PROCESS_INFORMATION* process_info) noexcept
  {
    auto& self = get(state);

    std::error_code last_errorc;

    if (!game_args)
    {
      return E_POINTER;
    }

    if (!process_info)
    {
      return E_POINTER;
    }

    store_registry_data(state);

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

      if (has_zero_tier_extension(state) && std::any_of(game_args->environment_settings.begin(), game_args->environment_settings.end(), zt_is_enabled) && std::any_of(game_args->environment_settings.begin(), game_args->environment_settings.end(), [](auto& item) {
            return item.name != nullptr && std::wstring_view(item.name) == L"ZERO_TIER_NETWORK_ID" && item.value != nullptr && item.value[0] != '\0';
          }))
      {
        namespace fs = std::filesystem;

        auto ext_path = fs::path(win32::module_ref::current_module().GetModuleFileName()).parent_path() / "runtime-extensions";

        fs::remove_all(ext_path, last_errorc);
        fs::create_directories(ext_path, last_errorc);

        auto wsock_path = *get_zero_tier_extension_folder_path(state) / "wsock32-on-zero-tier.dll";
        auto zt_path = *get_zero_tier_extension_folder_path(state) / "zt-shared.dll";

        fs::copy_file(wsock_path, ext_path / "wsock32.dll", fs::copy_options::overwrite_existing, last_errorc);
        fs::copy_file(zt_path, ext_path / "zt-shared.dll", fs::copy_options::overwrite_existing, last_errorc);

        ::SetDllDirectoryW(ext_path.c_str());

        if (has_extension_module(state))
        {
          if (auto& caps = get_extension(state).caps; caps && caps->ip_connect_setting)
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
              ::SetEnvironmentVariableW(L"ZERO_TIER_FALLBACK_BROADCAST_IP_V4", self.registry_data.last_ip_address.data());
            }
          }
        }
        else
        {
          ::SetEnvironmentVariableW(L"ZERO_TIER_FALLBACK_BROADCAST_IP_V4", self.registry_data.last_ip_address.data());
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

    if (has_extension_module(state))
    {
      std::string extension_path = get_extension(state).GetModuleFileName<char>();

      auto* apply_prelaunch_settings_func = get_extension(state).GetProcAddress<std::add_pointer_t<apply_prelaunch_settings>>("apply_prelaunch_settings");

      if (apply_prelaunch_settings_func)
      {
        if (apply_prelaunch_settings_func(self.loaded_path.c_str(), game_args) != S_OK)
        {
          return E_ABORT;
        }
      }

      auto* format_command_line_func = get_extension(state).GetProcAddress<std::add_pointer_t<format_command_line>>("format_command_line");

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

      auto* input_backends = get_extension(state).GetProcAddress<std::add_pointer_t<wchar_t*>>("controller_input_backends");

      if (input_backends && input_backends[0] && std::wstring_view(input_backends[0]) == get_extension(state).GetModuleFileName<wchar_t>())
      {
        dll_paths.emplace_back(extension_path.c_str());
      }

      auto* detour_ordinal = get_extension(state).GetProcAddress(1);

      if (detour_ordinal)
      {
        dll_paths.emplace_back(extension_path.c_str());
      }

      std::wstring args;
      args.reserve(argc + 3 * sizeof(std::wstring) + 3);


      auto exe_path = self.loaded_path;

      if (auto& caps = get_extension(state).caps; game_args && caps && caps->preferred_exe_setting && caps->preferred_exe_setting[0] != '\0')
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

      if (dll_paths.empty() && ::CreateProcessW(exe_path.c_str(), args.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, self.loaded_path.parent_path().c_str(), &startup_info, process_info))
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
                 self.loaded_path.parent_path().c_str(),
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
      args.append(self.loaded_path.wstring());
      args.append(1, L'"');

      auto cmd_args = std::find_if(game_args->string_settings.begin(), game_args->string_settings.end(), [=](auto& item) {
        return item.name && item.value && item.value[0] != '\0' && item.name == std::wstring_view(L"CMD_ARGS");
      });

      if (cmd_args != game_args->string_settings.end())
      {
        args.append(cmd_args->value);
      }

      auto deferred = configure_environment();

      if (::CreateProcessW(self.loaded_path.c_str(), args.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, self.loaded_path.parent_path().c_str(), &startup_info, process_info))
      {
        return S_OK;
      }

      auto last_error = ::GetLastError();
      return HRESULT_FROM_WIN32(last_error);
    }
  }
}// namespace siege::views