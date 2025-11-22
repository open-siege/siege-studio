#include <vector>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <hidusage.h>
#include <xinput.h>
#include <joystickapi.h>
#include <hidsdi.h>
#include <joystickapi.h>
#include "exe_shared.hpp"


namespace siege::views
{
  using namespace siege::platform;
  std::wstring reg_name_for_controller_vkey(SHORT vkey);
  std::wstring_view input_type_to_string(input_type value);
  std::wstring_view context_to_string(hardware_context value);
  std::function<std::optional<hardware_index>(SHORT, controller_info::button_preference)> hardware_index_from_cache(HKEY cache);

  struct input_mapping
  {
    hardware_index index;
    WORD vkey;
  };

  std::function<std::optional<hardware_index>(SHORT, controller_info::button_preference)> store_custom_controller_mapping(const controller_info& info, std::span<input_mapping> mappings)
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
  std::map<std::wstring_view, SHORT>& controller_vkey_reg_names();
  std::function<std::optional<hardware_index>(SHORT, controller_info::button_preference)> hardware_index_from_cache(HKEY cache)
  {
    std::map<SHORT, hardware_index> mapping{};

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

    return [mapping = std::move(mapping)](SHORT vkey, controller_info::button_preference pref) -> std::optional<hardware_index> {
      auto result = mapping.end();

      auto is_vkey_pair = [&](auto a, auto b) { return vkey == a || vkey == b; };

      auto find_mapping_pair = [&](auto pos, auto neg) {
        auto pos_result = mapping.find(pos);
        auto neg_result = mapping.find(neg);

        if (pos == vkey && pos_result == mapping.end() && neg_result != mapping.end() && neg_result->second.type == input_type::axis)
        {
          return neg_result;
        }

        if (neg == vkey && neg_result == mapping.end() && pos_result != mapping.end() && pos_result->second.type == input_type::axis)
        {
          return pos_result;
        }

        return pos == vkey ? pos_result : neg_result;
      };

      if (is_vkey_pair(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT))
      {
        result = find_mapping_pair(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT);
      }
      else if (is_vkey_pair(VK_GAMEPAD_LEFT_THUMBSTICK_UP, VK_GAMEPAD_LEFT_THUMBSTICK_DOWN))
      {
        result = find_mapping_pair(VK_GAMEPAD_LEFT_THUMBSTICK_UP, VK_GAMEPAD_LEFT_THUMBSTICK_DOWN);
      }
      else if (is_vkey_pair(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT))
      {
        result = find_mapping_pair(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT);
      }
      else if (is_vkey_pair(VK_GAMEPAD_RIGHT_THUMBSTICK_UP, VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN))
      {
        result = find_mapping_pair(VK_GAMEPAD_RIGHT_THUMBSTICK_UP, VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN);
      }
      else
      {
        result = mapping.find(vkey);
      }

      if (result == mapping.end())
      {
        return std::nullopt;
      }

      return result->second;
    };
  }

  std::map<std::wstring_view, SHORT>& controller_vkey_reg_names()
  {
    static std::map<std::wstring_view, SHORT> vkeys{
      { L"AButton", VK_GAMEPAD_A },
      { L"BButton", VK_GAMEPAD_B },
      { L"XButton", VK_GAMEPAD_X },
      { L"YButton", VK_GAMEPAD_Y },
      { L"LeftBumper", VK_GAMEPAD_LEFT_SHOULDER },
      { L"RightBumper", VK_GAMEPAD_RIGHT_SHOULDER },
      { L"LeftTrigger", VK_GAMEPAD_LEFT_TRIGGER },
      { L"RightTrigger", VK_GAMEPAD_RIGHT_TRIGGER },
      { L"DPadUp", VK_GAMEPAD_DPAD_UP },
      { L"DPadDown", VK_GAMEPAD_DPAD_DOWN },
      { L"DPadLeft", VK_GAMEPAD_DPAD_LEFT },
      { L"DPadRight", VK_GAMEPAD_DPAD_RIGHT },
      { L"LeftStickButton", VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON },
      { L"LeftStickUp", VK_GAMEPAD_LEFT_THUMBSTICK_UP },
      { L"LeftStickDown", VK_GAMEPAD_LEFT_THUMBSTICK_DOWN },
      { L"LeftStickLeft", VK_GAMEPAD_LEFT_THUMBSTICK_LEFT },
      { L"LeftStickRight", VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT },
      { L"RightStickButton", VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON },
      { L"RightStickUp", VK_GAMEPAD_RIGHT_THUMBSTICK_UP },
      { L"RightStickDown", VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN },
      { L"RightStickLeft", VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT },
      { L"RightStickRight", VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT },
      { L"ViewButton", VK_GAMEPAD_VIEW },
      { L"MenuButton", VK_GAMEPAD_MENU },
    };
    return vkeys;
  }

  SHORT controller_vkey_reg_name(std::wstring_view name)
  {
    return controller_vkey_reg_names().at(name);
  }

  std::wstring reg_name_for_controller_vkey(SHORT vkey)
  {
    auto& names = controller_vkey_reg_names();

    auto name = std::find_if(names.begin(), names.end(), [vkey](auto& name) { return name.second == vkey; });

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