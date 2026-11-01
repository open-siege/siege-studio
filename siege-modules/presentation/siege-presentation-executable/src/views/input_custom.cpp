module;
#include <siege/platform/windows.hpp>

module exe.shared;

import std;
import win32.dialog;
import win32.capabilities;

namespace siege::views
{
  using namespace siege::platform;
  using vkey = siege::platform::vkey;
  std::wstring reg_name_for_controller_vkey(vkey key);
  std::wstring_view input_type_to_string(input_type value);
  std::wstring_view context_to_string(hardware_context value);
  std::function<std::optional<hardware_index>(vkey, controller_info::button_preference)> hardware_index_from_cache(HKEY cache);

  struct input_mapping
  {
    hardware_index index;
    vkey vkey;
  };

  std::function<std::optional<hardware_index>(vkey, controller_info::button_preference)> store_custom_controller_mapping(const controller_info& info, std::span<input_mapping> mappings)
  {
    std::wstring key = L"Software\\The Siege Hub\\Siege Studio\\ControllerCache";
    HKEY user_key = nullptr;
    HKEY main_key = nullptr;
    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_SET_VALUE;

    key = key + L"\\" + std::to_wstring(info.vendor_product_id.first) + std::wstring(L"\\") + std::to_wstring(info.vendor_product_id.second);
    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, key.c_str(), 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      for (auto& mapping : mappings)
      {
        auto key = reg_name_for_controller_vkey(mapping.vkey);
        auto value = std::to_wstring(mapping.index.index) + L"," + std::wstring{ input_type_to_string(mapping.index.type) };

        ::RegSetValueExW(main_key, key.data(), 0, REG_SZ, (BYTE*)value.data(), value.size() * sizeof(wchar_t));
      }
      auto func = hardware_index_from_cache(main_key);
      ::RegCloseKey(main_key);
      ::RegCloseKey(user_key);
      return func;
    }

    return nullptr;
  }

  std::optional<input_type> string_to_input_type(std::wstring_view value);
  std::map<std::wstring_view, vkey>& controller_vkey_reg_names();
  std::function<std::optional<hardware_index>(vkey, controller_info::button_preference)> hardware_index_from_cache(HKEY cache)
  {
    std::map<vkey, hardware_index> mapping{};

    std::wstring buffer;
    DWORD count = 0;
    DWORD type = REG_SZ;

    for (auto& item : controller_vkey_reg_names())
    {
      buffer.clear();
      count = 0;
      if (::RegGetValueW(cache, nullptr, item.first.data(), RRF_RT_REG_SZ, &type, buffer.data(), &count) == ERROR_MORE_DATA)
      {
        buffer.resize(count);

        if (::RegGetValueW(cache, nullptr, item.first.data(), RRF_RT_REG_SZ, &type, buffer.data(), &count) != ERROR_SUCCESS)
        {
          continue;
        }

        try
        {
          std::wstring_view index = buffer.data();
          std::wstring_view input_type = index.substr(index.find(L",")).substr(1);
          index = index.substr(0, index.find(L","));

          mapping[item.second] = hardware_index{
            .type = string_to_input_type(input_type).value_or(input_type::button),
            .index = (std::uint16_t)std::stoi(std::wstring{ index })
          };
        }
        catch (const std::invalid_argument&)
        {
        }
      }
    }

    return [mapping = std::move(mapping)](vkey key, controller_info::button_preference pref) -> std::optional<hardware_index> {
      auto result = mapping.end();

      auto is_vkey_pair = [&](auto a, auto b) { return key == a || key == b; };

      auto find_mapping_pair = [&](auto pos, auto neg) {
        auto pos_result = mapping.find(pos);
        auto neg_result = mapping.find(neg);

        if (pos == key && pos_result == mapping.end() && neg_result != mapping.end() && neg_result->second.type == input_type::axis)
        {
          return neg_result;
        }

        if (neg == key && neg_result == mapping.end() && pos_result != mapping.end() && pos_result->second.type == input_type::axis)
        {
          return pos_result;
        }

        return pos == key ? pos_result : neg_result;
      };

      if (is_vkey_pair(vkey::gamepad_left_thumbstick_left, vkey::gamepad_left_thumbstick_right))
      {
        result = find_mapping_pair(vkey::gamepad_left_thumbstick_left, vkey::gamepad_left_thumbstick_right);
      }
      else if (is_vkey_pair(vkey::gamepad_left_thumbstick_up, vkey::gamepad_left_thumbstick_down))
      {
        result = find_mapping_pair(vkey::gamepad_left_thumbstick_up, vkey::gamepad_left_thumbstick_down);
      }
      else if (is_vkey_pair(vkey::gamepad_right_thumbstick_left, vkey::gamepad_right_thumbstick_right))
      {
        result = find_mapping_pair(vkey::gamepad_right_thumbstick_left, vkey::gamepad_right_thumbstick_right);
      }
      else if (is_vkey_pair(vkey::gamepad_right_thumbstick_up, vkey::gamepad_right_thumbstick_down))
      {
        result = find_mapping_pair(vkey::gamepad_right_thumbstick_up, vkey::gamepad_right_thumbstick_down);
      }
      else
      {
        result = mapping.find(key);
      }

      if (result == mapping.end())
      {
        return std::nullopt;
      }

      return result->second;
    };
  }

  std::map<std::wstring_view, vkey>& controller_vkey_reg_names()
  {
    static std::map<std::wstring_view, vkey> vkeys{
      { L"AButton", vkey::gamepad_a },
      { L"BButton", vkey::gamepad_b },
      { L"XButton", vkey::gamepad_x },
      { L"YButton", vkey::gamepad_y },
      { L"LeftBumper", vkey::gamepad_left_shoulder },
      { L"RightBumper", vkey::gamepad_right_shoulder },
      { L"LeftTrigger", vkey::gamepad_left_trigger },
      { L"RightTrigger", vkey::gamepad_right_trigger },
      { L"DPadUp", vkey::gamepad_dpad_up },
      { L"DPadDown", vkey::gamepad_dpad_down },
      { L"DPadLeft", vkey::gamepad_dpad_left },
      { L"DPadRight", vkey::gamepad_dpad_right },
      { L"LeftStickButton", vkey::gamepad_left_thumbstick_button },
      { L"LeftStickUp", vkey::gamepad_left_thumbstick_up },
      { L"LeftStickDown", vkey::gamepad_left_thumbstick_down },
      { L"LeftStickLeft", vkey::gamepad_left_thumbstick_left },
      { L"LeftStickRight", vkey::gamepad_left_thumbstick_right },
      { L"RightStickButton", vkey::gamepad_right_thumbstick_button },
      { L"RightStickUp", vkey::gamepad_right_thumbstick_up },
      { L"RightStickDown", vkey::gamepad_right_thumbstick_down },
      { L"RightStickLeft", vkey::gamepad_right_thumbstick_left },
      { L"RightStickRight", vkey::gamepad_right_thumbstick_right },
      { L"ViewButton", vkey::gamepad_view },
      { L"MenuButton", vkey::gamepad_menu },
    };
    return vkeys;
  }

  vkey controller_vkey_reg_name(std::wstring_view name)
  {
    return controller_vkey_reg_names().at(name);
  }

  std::wstring reg_name_for_controller_vkey(vkey key)
  {
    auto& names = controller_vkey_reg_names();

    auto name = std::find_if(names.begin(), names.end(), [key](auto& name) { return name.second == key; });

    if (name == names.end())
    {
      return L"";
    }

    return std::wstring{ name->first };
  }

  auto& hardware_context_names()
  {
    static std::map<std::wstring_view, hardware_context> names{
      { L"xbox", hardware_context::controller_xbox },
      { L"playstation_3", hardware_context::controller_playstation_3 },
      { L"playstation_4", hardware_context::controller_playstation_4 },
      { L"pro_controller", hardware_context::controller_nintendo },
      { L"joystick", hardware_context::joystick },
      { L"keyboard", hardware_context::keyboard },
      { L"mouse", hardware_context::mouse },
      { L"mouse_wheel", hardware_context::mouse_wheel },
      { L"steering_wheel", hardware_context::steering_wheel },
      { L"pedal", hardware_context::pedal },
      { L"joystick", hardware_context::joystick },
      { L"throttle", hardware_context::throttle },
      { L"custom", hardware_context::custom },
      { L"global", hardware_context::global },
    };
    return names;
  }

  hardware_context string_to_context(std::wstring_view value)
  {
    auto iter = hardware_context_names().find(value);

    if (iter == hardware_context_names().end())
    {
      return hardware_context::global;
    }

    return iter->second;
  }

  std::wstring_view context_to_string(hardware_context value)
  {
    auto iter = std::ranges::find_if(hardware_context_names(), [value](auto& item) { return item.second == value; });

    if (iter == hardware_context_names().end())
    {
      return L"global";
    }

    return iter->first;
  }

  auto& input_type_names()
  {
    static std::map<std::wstring_view, input_type> names{
      { L"button", input_type::button },
      { L"axis", input_type::axis },
      { L"hat", input_type::hat },
    };
    return names;
  }

  std::optional<input_type> string_to_input_type(std::wstring_view value)
  {
    auto iter = input_type_names().find(value);

    if (iter == input_type_names().end())
    {
      return std::nullopt;
    }

    return iter->second;
  }

  std::wstring_view input_type_to_string(input_type value)
  {
    auto iter = std::ranges::find_if(input_type_names(), [value](auto& item) { return item.second == value; });

    if (iter == input_type_names().end())
    {
      return L"";
    }

    return iter->first;
  }
}// namespace siege::views
