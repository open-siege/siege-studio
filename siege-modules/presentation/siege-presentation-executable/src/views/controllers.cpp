#include <vector>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <hidusage.h>
#include <xinput.h>

namespace siege::views
{
  using namespace siege::platform;

  std::uint16_t hardware_index_for_xbox_vkey(SHORT vkey);
  std::uint16_t hardware_index_for_ps3_vkey(SHORT vkey);

  struct controller_info
  {
    hardware_context detected_context;
    std::string_view backend;
    std::uint16_t (*get_hardware_index)(SHORT vkey);
  };


  std::vector<controller_info> get_connected_controllers()
  {
    std::vector<controller_info> results;

    std::array<RAWINPUTDEVICELIST, 64> controllers{};

    UINT size = controllers.size();
    ::GetRawInputDeviceList(controllers.data(), &size, sizeof(RAWINPUTDEVICELIST));

    auto is_hid = [](auto& entry) {
      return entry.dwType == RIM_TYPEHID;
    };

    std::stable_partition(controllers.begin(), controllers.end(), is_hid);

    std::wstring device_name(2048, 0);

    for (auto& controller : controllers)
    {
      if (controller.dwType != RIM_TYPEHID)
      {
        break;
      }
      RID_DEVICE_INFO info{};
      UINT info_size = sizeof(info);

      if (::GetRawInputDeviceInfoW(controller.hDevice, RIDI_DEVICEINFO, &info, &info_size) != info_size)
      {
        continue;
      }
      if (!(info.hid.usUsage == HID_USAGE_GENERIC_GAMEPAD || info.hid.usUsage == HID_USAGE_GENERIC_JOYSTICK))
      {
        continue;
      }

      info_size = device_name.size();

      if (auto real_size = ::GetRawInputDeviceInfoW(controller.hDevice, RIDI_DEVICENAME, device_name.data(), &info_size); real_size <= 0)
      {
        continue;
      }
      else if (real_size < 2048)
      {
        device_name.resize(real_size);
      }

      if (device_name.rfind(L"IG_") != std::wstring_view::npos)
      {
        results.emplace_back(hardware_context::controller_xbox, "user32", hardware_index_for_xbox_vkey);
      }
      else
      {
        // TODO more robust checking here
        results.emplace_back(hardware_context::controller_playstation_3, "user32", hardware_index_for_ps3_vkey);
      }

      device_name.clear();
      device_name.resize(info_size);
    }

    // When connecting through remote desktop, it is possible that no HID devices are reported by
    // Raw input, so we have to query through other APIs.
    if (!std::any_of(controllers.begin(), controllers.end(), is_hid))
    {
      auto version_and_name = win32::get_xinput_version();

      if (version_and_name)
      {
        win32::module xinput_module;
        xinput_module.reset(::LoadLibraryExW(version_and_name->second.data(), nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0));

        std::add_pointer_t<decltype(::XInputGetState)> xinput_get_state = xinput_module.GetProcAddress<decltype(xinput_get_state)>("XInputGetState");
        XINPUT_STATE temp{};

        for (auto i = 0; i < 4; ++i)
        {
          if (xinput_get_state(i, &temp) == S_OK)
          {
            results.emplace_back(hardware_context::controller_xbox, "xinput", hardware_index_for_xbox_vkey);
          }
        }
      }
    }

    return results;
  }

  std::uint16_t hardware_index_for_xbox_vkey(SHORT vkey)
  {
    switch (vkey)
    {
    case VK_GAMEPAD_A:
      return 0;
    case VK_GAMEPAD_B:
      return 1;
    case VK_GAMEPAD_X:
      return 2;
    case VK_GAMEPAD_Y:
      return 3;
    case VK_GAMEPAD_LEFT_SHOULDER:
      return 4;
    case VK_GAMEPAD_RIGHT_SHOULDER:
      return 5;
    case VK_GAMEPAD_VIEW:
      return 6;
    case VK_GAMEPAD_MENU:
      return 7;
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
      return 8;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
      return 9;
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
      return 0;
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
      return 1;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
      return 4;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
      return 3;
    case VK_GAMEPAD_LEFT_TRIGGER:
    case VK_GAMEPAD_RIGHT_TRIGGER:
      return 2;
    default:
      return 0;
    }
  }

  std::uint16_t hardware_index_for_ps3_vkey(SHORT vkey)
  {
    switch (vkey)
    {
    case VK_GAMEPAD_X:// ps3/ps4 square
      return 0;
    case VK_GAMEPAD_A:// ps3/ps4 cross
      return 1;
    case VK_GAMEPAD_B:// ps3/ps4 circle
      return 2;
    case VK_GAMEPAD_Y:// ps3/ps4 triangle
      return 3;
    case VK_GAMEPAD_LEFT_SHOULDER:
      return 4;
    case VK_GAMEPAD_RIGHT_SHOULDER:
      return 5;
    case VK_GAMEPAD_LEFT_TRIGGER:
      return 6;
    case VK_GAMEPAD_RIGHT_TRIGGER:
      return 7;
    case VK_GAMEPAD_VIEW:
      return 8;
    case VK_GAMEPAD_MENU:
      return 9;
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
      return 10;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
      return 11;
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
      return 0;
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
      return 1;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
      return 2;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
      return 3;
    default:
      return 0;
    }
  }

}// namespace siege::views