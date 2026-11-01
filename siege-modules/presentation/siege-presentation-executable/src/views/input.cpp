module;
#include <siege/platform/windows.hpp>
#include <hidusage.h>
#include <xinput.h>
#include <joystickapi.h>
#include <hidsdi.h>
#include <versionhelpers.h>

module exe.shared;

import std;
import win32.window;
import win32.dialog;
import win32.capabilities;

namespace siege::views
{
  using namespace siege::platform;
  using vkey = siege::platform::vkey;

  std::optional<hardware_index> hardware_index_for_xbox_vkey(vkey key, controller_info::button_preference);
  std::optional<hardware_index> hardware_index_for_ps3_vkey(vkey key, controller_info::button_preference);
  std::optional<hardware_index> hardware_index_for_ps4_vkey(vkey key, controller_info::button_preference);
  std::optional<hardware_index> hardware_index_for_joystick_vkey(vkey key, controller_info::button_preference);

  std::function<std::optional<hardware_index>(vkey, controller_info::button_preference)> hardware_index_from_cache(HKEY cache);
  std::function<std::optional<hardware_index>(vkey, controller_info::button_preference)> hardware_index_lookup_from_context(hardware_context context, HKEY cache);

  std::wstring_view context_to_string(hardware_context value);
  hardware_context string_to_context(std::wstring_view value);

  std::set<std::wstring>& strings();
  std::optional<controller_info> controller_info_for_raw_input_device_handle(HANDLE handle);

  constexpr static auto vk_to_xinput_buttons = std::array<std::pair<vkey, SHORT>, 14>{
    {
      { vkey::gamepad_a, XINPUT_GAMEPAD_A },
      { vkey::gamepad_b, XINPUT_GAMEPAD_B },
      { vkey::gamepad_x, XINPUT_GAMEPAD_X },
      { vkey::gamepad_y, XINPUT_GAMEPAD_Y },
      { vkey::gamepad_left_thumbstick_button, XINPUT_GAMEPAD_LEFT_THUMB },
      { vkey::gamepad_right_thumbstick_button, XINPUT_GAMEPAD_RIGHT_THUMB },
      { vkey::gamepad_left_shoulder, XINPUT_GAMEPAD_LEFT_SHOULDER },
      { vkey::gamepad_right_shoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER },
      { vkey::gamepad_menu, XINPUT_GAMEPAD_START },
      { vkey::gamepad_view, XINPUT_GAMEPAD_BACK },
      { vkey::gamepad_dpad_up, XINPUT_GAMEPAD_DPAD_UP },
      { vkey::gamepad_dpad_down, XINPUT_GAMEPAD_DPAD_DOWN },
      { vkey::gamepad_dpad_left, XINPUT_GAMEPAD_DPAD_LEFT },
      { vkey::gamepad_dpad_right, XINPUT_GAMEPAD_DPAD_RIGHT },
    }
  };

  // Get the connected controllers with as much
  // useful information as possible.
  // The most important thing is that we figure out the hardware
  // context to use as a default and then the preferred input device
  // for games which make use of it.
  std::vector<controller_info> get_connected_controllers()
  {
    std::vector<controller_info> results;

    // TODO make this dynamic
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

      auto temp = controller_info_for_raw_input_device_handle(controller.hDevice);

      if (temp)
      {
        results.emplace_back(std::move(*temp));
      }
    }

    // When connecting through remote desktop, it is possible that no HID devices are reported by
    // Raw input, so we have to query through other APIs.
    if (!std::any_of(controllers.begin(), controllers.end(), is_hid))
    {
      auto version_and_name = win32::get_xinput_version();

      // TODO use the undocumented API to get the vendor + product ID to match
      // the controller with existing devices if they are not already detected.
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
            results.emplace_back(controller_info{
              .detected_context = hardware_context::controller_xbox,
              .backend = "xinput",
              .get_hardware_index = hardware_index_for_xbox_vkey });
          }
        }
      }
    }

    static auto winmm_module = []() {
      win32::module winmm_module;
      winmm_module.reset(::LoadLibraryExW(L"winmm.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0));
      return winmm_module;
    }();

    static auto* get_dev_caps = winmm_module.GetProcAddress<std::add_pointer_t<decltype(::joyGetDevCapsW)>>("joyGetDevCapsW");
    static auto* get_num_devs = winmm_module.GetProcAddress<std::add_pointer_t<decltype(::joyGetNumDevs)>>("joyGetNumDevs");

    if (get_dev_caps && get_num_devs)
    {
      auto num_devs = get_num_devs();

      ::JOYCAPSW caps{};
      for (auto i = 0; i < num_devs; ++i)
      {
        auto state = get_dev_caps(i, &caps, sizeof(caps));
        if (state == JOYERR_NOERROR)
        {
          auto iter = std::find_if(results.begin(), results.end(), [&](auto& item) {
            return item.vendor_product_id.first == caps.wMid && item.vendor_product_id.second == caps.wPid;
          });

          if (iter != results.end())
          {
            iter->is_system_preferred = true;
          }
          break;
        }
      }
    }

    std::stable_partition(results.begin(), results.end(), [](const auto& item) { return item.is_system_preferred; });

    return results;
  }
  std::optional<controller_info> controller_info_for_raw_input_handle(HRAWINPUT handle)
  {
    RAWINPUTHEADER header{};

    UINT size = sizeof(header);

    // TODO handle error case
    ::GetRawInputData(handle, RID_HEADER, &header, &size, sizeof(header));

    return controller_info_for_raw_input_device_handle(header.hDevice);
  }
  std::optional<controller_info> controller_info_for_raw_input_device_handle(HANDLE handle)
  {
    controller_info result{ .backend = "user32" };

    RID_DEVICE_INFO device_info{ .cbSize = sizeof(RID_DEVICE_INFO) };
    UINT size = sizeof(device_info);

    if (::GetRawInputDeviceInfoW(handle, RIDI_DEVICEINFO, &device_info, &size) > 0 && device_info.dwType == RIM_TYPEHID)
    {
      result.vendor_product_id = std::make_pair(device_info.hid.dwVendorId, device_info.hid.dwProductId);
    }

    if (!(device_info.hid.usUsage == HID_USAGE_GENERIC_GAMEPAD || device_info.hid.usUsage == HID_USAGE_GENERIC_JOYSTICK))
    {
      return std::nullopt;
    }

    std::vector<wchar_t> device_buffer(255, '\0');
    size = (UINT)device_buffer.size();

    if (::GetRawInputDeviceInfoW(handle, RIDI_DEVICENAME, device_buffer.data(), &size) > 0)
    {
      std::wstring_view device_path(device_buffer.data());

      if (!device_path.empty())
      {
        result.device_path = *strings().emplace(device_path).first;
      }

      // If we have IG_ then we are an xbox controller:
      // see https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-and-directinput
      if (device_path.rfind(L"IG_") != std::wstring_view::npos)
      {
        result.detected_context = hardware_context::controller_xbox;
        result.get_hardware_index = hardware_index_for_xbox_vkey;
      }
      else
      {
        // TODO ask the user to specify the layout of their input device
        std::wstring key = L"Software\\The Siege Hub\\Siege Studio\\ControllerCache";
        HKEY user_key = nullptr;
        HKEY main_key = nullptr;
        auto access = KEY_QUERY_VALUE | KEY_READ;

        key = key + L"\\" + std::to_wstring(result.vendor_product_id.first) + std::wstring(L"\\") + std::to_wstring(result.vendor_product_id.second);
        if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, key.c_str(), 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
        {
          DWORD type = REG_SZ;
          std::wstring buffer(64, L'\0');
          DWORD count = buffer.size() * sizeof(wchar_t);
          ::RegGetValueW(main_key, nullptr, L"ControllerContext", RRF_RT_REG_SZ, &type, buffer.data(), &count);
          if (buffer.contains(L'\0'))
          {
            buffer.resize(buffer.find(L'\0'));
          }

          result.detected_context = string_to_context(buffer);
          result.get_hardware_index = hardware_index_lookup_from_context(result.detected_context, main_key);
          ::RegCloseKey(main_key);
          ::RegCloseKey(user_key);
        }
      }

      static auto hid_module = []() {
        win32::module hid_module;
        hid_module.reset(::LoadLibraryExW(L"hid.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0));
        return hid_module;
      }();

      static auto* get_product_string = hid_module.GetProcAddress<std::add_pointer_t<decltype(::HidD_GetProductString)>>("HidD_GetProductString");

      if (get_product_string)
      {
        auto file_handle = ::CreateFileW(device_path.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (file_handle == INVALID_HANDLE_VALUE)
        {
          file_handle = ::CreateFileW(device_path.data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        }
        std::vector<wchar_t> device_buffer(255, '\0');
        get_product_string(file_handle, device_buffer.data(), device_buffer.size());
        ::CloseHandle(file_handle);
        std::wstring_view device_name(device_buffer.data());
        if (!device_name.empty())
        {
          result.device_name = *strings().emplace(device_name).first;
        }
      }

      return result;
    }

    return std::nullopt;
  }

  bool populate_caps(controller_state& state);

  struct hid_caps
  {
    HIDP_CAPS caps;
    int trigger_value_count = 0;
    bool has_triggers_as_buttons = false;
    std::vector<HIDP_BUTTON_CAPS> button_caps;
    std::map<HIDP_BUTTON_CAPS*, std::vector<USAGE>> button_usages;
    std::vector<HIDP_VALUE_CAPS> value_caps;
    std::vector<ULONG> value_usages;

    std::vector<char> buffer;
    std::vector<char> secondary;

    XINPUT_STATE last_state;

    std::shared_ptr<void> file_handle;
  };

  constexpr SHORT to_s16(USHORT, ULONG value_usages);
  constexpr LONG to_s32(USHORT, ULONG value_usages);
  constexpr std::pair<BYTE, BYTE> to_byte_pair(USHORT, ULONG value_usages);
  constexpr BYTE to_byte(USHORT caps, ULONG usage);

  std::optional<XINPUT_STATE> get_xinput_state(HANDLE file_handle);
  std::set<std::uint32_t> hardware_buttons(const controller_state& state)
  {
    if (!state.caps.has_value())
    {
      return {};
    }

    const hid_caps* caps = std::any_cast<hid_caps>(&state.caps);

    std::set<std::uint32_t> results;
    for (auto& button_cap : caps->button_caps)
    {
      results.emplace(button_cap.Range.UsageMin);
    }

    return results;
  }
  std::set<std::uint32_t> hardware_axes(const controller_state& state)
  {
    if (!state.caps.has_value())
    {
      return {};
    }

    const hid_caps* caps = std::any_cast<hid_caps>(&state.caps);
    std::set<std::uint32_t> results;

    for (auto& value_cap : caps->value_caps)
    {
      if (!(value_cap.Range.UsageMin >= HID_USAGE_GENERIC_X && value_cap.Range.UsageMin <= HID_USAGE_GENERIC_SLIDER))
      {
        continue;
      }
      results.emplace(value_cap.Range.UsageMin - HID_USAGE_GENERIC_X);
    }

    return results;
  }
  std::uint32_t hardware_hat_count(const controller_state& state)
  {
    if (!state.caps.has_value())
    {
      return 0;
    }

    const hid_caps* caps = std::any_cast<hid_caps>(&state.caps);

    auto count = std::count_if(caps->value_caps.begin(), caps->value_caps.end(), [](auto& value_cap) {
      return value_cap.Range.UsageMin == HID_USAGE_GENERIC_HATSWITCH;
    });

    return static_cast<std::uint32_t>(count);
  }
  DIJOYSTATE get_raw_state_for_handle(controller_state& state, HRAWINPUT handle)
  {
    DIJOYSTATE result{};
    hid_caps* caps = std::any_cast<hid_caps>(&state.caps);

    UINT buffer_size{};
    ::GetRawInputData(handle, RID_INPUT, nullptr, &buffer_size, sizeof(RAWINPUTHEADER));

    if (!buffer_size)
    {
      return result;
    }

    caps->buffer.resize(buffer_size);
    caps->buffer.resize(::GetRawInputData(handle, RID_INPUT, caps->buffer.data(), &buffer_size, sizeof(RAWINPUTHEADER)));

    if (caps->buffer.empty())
    {
      return result;
    }

    std::span<RAWINPUT> input_data((RAWINPUT*)caps->buffer.data(), caps->buffer.size() / sizeof(RAWINPUT));

    for (auto& raw_input : input_data)
    {
      if (raw_input.header.dwType != RIM_TYPEHID)
      {
        continue;
      }

      ::GetRawInputDeviceInfoW(raw_input.header.hDevice, RIDI_PREPARSEDDATA, nullptr, &buffer_size);
      caps->secondary.resize(buffer_size);
      ::GetRawInputDeviceInfoW(raw_input.header.hDevice, RIDI_PREPARSEDDATA, caps->secondary.data(), &buffer_size);
      PHIDP_PREPARSED_DATA preparsed = (PHIDP_PREPARSED_DATA)caps->secondary.data();

      for (auto& [button, usages] : caps->button_usages)
      {
        if (button->UsagePage != HID_USAGE_PAGE_BUTTON)
        {
          continue;
        }

        auto count = (button->Range.UsageMax - button->Range.UsageMin + 1);
        usages.resize(count);
        ULONG size = usages.size();
        auto status = ::HidP_GetUsages(HidP_Input, button->UsagePage, 0, usages.data(), &size, preparsed, (PCHAR)raw_input.data.hid.bRawData, raw_input.data.hid.dwSizeHid);
        usages.resize(size);

        if (status != HIDP_STATUS_SUCCESS)
        {
          continue;
        }

        for (auto i = 0; i < usages.size(); ++i)
        {
          auto usage = usages[i] - button->Range.UsageMin;
          if (usage >= 32)
          {
            break;
          }
          result.rgbButtons[usage] = 255;
        }
      }

      for (auto i = 0; i < caps->value_caps.size(); ++i)
      {
        auto& value = caps->value_caps[i];
        auto& usage = caps->value_usages[i];
        auto status = ::HidP_GetUsageValue(HidP_Input, value.UsagePage, 0, value.Range.UsageMin, &usage, preparsed, (PCHAR)raw_input.data.hid.bRawData, raw_input.data.hid.dwSizeHid);

        if (status != HIDP_STATUS_SUCCESS)
        {
          continue;
        }

        if (value.Range.UsageMin == HID_USAGE_GENERIC_HATSWITCH && value.BitSize == 4)
        {
          if (usage == 0 && value.LogicalMin != 0)
          {
            continue;
          }

          usage = usage - value.LogicalMin;
          // For indicators that have only five positions, the value for a controller is - 1, 0, 9,000, 18,000, or 27,000.
          /*if (usage == 0 || usage == 1 || usage == 7)
          {
            result.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;
          }

          if (usage == 1 || usage == 2 || usage == 3)
          {
            result.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
          }

          if (usage == 3 || usage == 4 || usage == 5)
          {
            result.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
          }

          if (usage == 5 || usage == 6 || usage == 7)
          {
            result.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
          }*/

          continue;
        }

        switch (value.Range.UsageMin)
        {
        case HID_USAGE_GENERIC_X: {
          result.lX = to_s32(value.BitSize, usage);
          break;
        }
        case HID_USAGE_GENERIC_Y: {
          result.lY = to_s32(value.BitSize, usage);
          break;
        }
        case HID_USAGE_GENERIC_Z: {
          result.lZ = to_s32(value.BitSize, usage);
          break;
        }
        case HID_USAGE_GENERIC_RX: {
          result.lRx = to_s32(value.BitSize, usage);
          break;
        }
        case HID_USAGE_GENERIC_RY: {
          result.lRy = to_s32(value.BitSize, usage);
          break;
        }
        case HID_USAGE_GENERIC_RZ: {
          result.lRz = to_s32(value.BitSize, usage);
          break;
        }
        default:
          break;
        }
      }
    }

    return result;
  }
  XINPUT_STATE get_current_state_for_handle(controller_state& state, HRAWINPUT handle)
  {
    if (!populate_caps(state))
    {
      return state.last_state;
    }

    if (state.info.get_hardware_index == nullptr)
    {
      return state.last_state;
    }

    hid_caps* caps = std::any_cast<hid_caps>(&state.caps);

    if (caps->file_handle)
    {
      auto xinput_state = get_xinput_state(caps->file_handle.get());

      if (xinput_state)
      {
        caps->last_state.Gamepad = xinput_state->Gamepad;
        return *xinput_state;
      }
    }

    UINT buffer_size{};
    ::GetRawInputData(handle, RID_INPUT, nullptr, &buffer_size, sizeof(RAWINPUTHEADER));

    if (!buffer_size)
    {
      return state.last_state;
    }
    caps->buffer.resize(buffer_size);
    caps->buffer.resize(::GetRawInputData(handle, RID_INPUT, caps->buffer.data(), &buffer_size, sizeof(RAWINPUTHEADER)));

    if (caps->buffer.empty())
    {
      return state.last_state;
    }

    XINPUT_STATE result{};

    std::span<RAWINPUT> input_data((RAWINPUT*)caps->buffer.data(), caps->buffer.size() / sizeof(RAWINPUT));

    for (auto& raw_input : input_data)
    {
      if (raw_input.header.dwType != RIM_TYPEHID)
      {
        continue;
      }

      ::GetRawInputDeviceInfoW(raw_input.header.hDevice, RIDI_PREPARSEDDATA, nullptr, &buffer_size);
      caps->secondary.resize(buffer_size);
      ::GetRawInputDeviceInfoW(raw_input.header.hDevice, RIDI_PREPARSEDDATA, caps->secondary.data(), &buffer_size);
      PHIDP_PREPARSED_DATA preparsed = (PHIDP_PREPARSED_DATA)caps->secondary.data();

      for (auto& [button, usages] : caps->button_usages)
      {
        if (button->UsagePage != HID_USAGE_PAGE_BUTTON)
        {
          continue;
        }

        auto count = (button->Range.UsageMax - button->Range.UsageMin + 1);
        usages.resize(count);
        ULONG size = usages.size();
        auto status = ::HidP_GetUsages(HidP_Input, button->UsagePage, 0, usages.data(), &size, preparsed, (PCHAR)raw_input.data.hid.bRawData, raw_input.data.hid.dwSizeHid);
        usages.resize(size);

        if (status != HIDP_STATUS_SUCCESS)
        {
          continue;
        }

        for (auto i = 0; i < usages.size(); ++i)
        {
          auto usage = usages[i] - button->Range.UsageMin;
          for (auto& button : vk_to_xinput_buttons)
          {
            if (button.first >= vkey::gamepad_dpad_up && button.first <= vkey::gamepad_dpad_right)
            {
              continue;
            }

            if (auto index = state.info.get_hardware_index(button.first, controller_info::prefer_button); index && index->index == usage)
            {
              result.Gamepad.wButtons |= button.second;
              break;
            }

            if (auto index = state.info.get_hardware_index(vkey::gamepad_left_trigger, controller_info::prefer_button);
              caps->has_triggers_as_buttons && index && index->index == usage)
            {
              result.Gamepad.bLeftTrigger = 255;
            }

            if (auto index = state.info.get_hardware_index(vkey::gamepad_right_trigger, controller_info::prefer_button);
              caps->has_triggers_as_buttons && index && index->index == usage)
            {
              result.Gamepad.bRightTrigger = 255;
            }
          }
        }
      }

      for (auto i = 0; i < caps->value_caps.size(); ++i)
      {
        auto& value = caps->value_caps[i];
        auto& usage = caps->value_usages[i];
        auto status = ::HidP_GetUsageValue(HidP_Input, value.UsagePage, 0, value.Range.UsageMin, &usage, preparsed, (PCHAR)raw_input.data.hid.bRawData, raw_input.data.hid.dwSizeHid);

        if (status != HIDP_STATUS_SUCCESS)
        {
          continue;
        }

        if (value.Range.UsageMin == HID_USAGE_GENERIC_HATSWITCH && value.BitSize == 4)
        {
          if (usage == 0 && value.LogicalMin != 0)
          {
            continue;
          }

          usage = usage - value.LogicalMin;
          if (usage == 0 || usage == 1 || usage == 7)
          {
            result.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;
          }

          if (usage == 1 || usage == 2 || usage == 3)
          {
            result.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
          }

          if (usage == 3 || usage == 4 || usage == 5)
          {
            result.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
          }

          if (usage == 5 || usage == 6 || usage == 7)
          {
            result.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
          }

          continue;
        }

        auto index = value.Range.UsageMin - HID_USAGE_GENERIC_X;

        if (auto ll_index = state.info.get_hardware_index(vkey::gamepad_left_thumbstick_left, controller_info::prefer_axis); ll_index && ll_index->index == index)
        {
          result.Gamepad.sThumbLX = to_s16(value.BitSize, usage);
        }
        else if (auto lu_index = state.info.get_hardware_index(vkey::gamepad_left_thumbstick_up, controller_info::prefer_axis); lu_index && lu_index->index == index)
        {
          result.Gamepad.sThumbLY = -to_s16(value.BitSize, usage);
        }
        else if (auto rl_index = state.info.get_hardware_index(vkey::gamepad_right_thumbstick_left, controller_info::prefer_axis); rl_index && rl_index->index == index)
        {
          result.Gamepad.sThumbRX = to_s16(value.BitSize, usage);
        }
        else if (auto ru_index = state.info.get_hardware_index(vkey::gamepad_right_thumbstick_up, controller_info::prefer_axis); ru_index && ru_index->index == index)
        {
          result.Gamepad.sThumbRY = -to_s16(value.BitSize, usage);
        }
        else if (auto lt_index = state.info.get_hardware_index(vkey::gamepad_left_trigger, controller_info::prefer_axis); caps->trigger_value_count == 1 && lt_index && lt_index->index == index)
        {
          auto [left, right] = to_byte_pair(value.BitSize, usage);
          result.Gamepad.bLeftTrigger = left;
          result.Gamepad.bRightTrigger = right;
        }
        else if (caps->trigger_value_count >= 2 && lt_index && lt_index->index == index)
        {
          result.Gamepad.bLeftTrigger = to_byte(value.BitSize, usage);
        }
        else if (auto rt_index = state.info.get_hardware_index(vkey::gamepad_right_trigger, controller_info::prefer_axis);
          caps->trigger_value_count >= 2 && rt_index && rt_index->index == index)
        {
          result.Gamepad.bRightTrigger = to_byte(value.BitSize, usage);
        }
      }
    }

    if (std::memcmp(&result.Gamepad, &caps->last_state.Gamepad, sizeof(result.Gamepad)) == 0)
    {
      result.dwPacketNumber = caps->last_state.dwPacketNumber;
    }
    else
    {
      caps->last_state.Gamepad = result.Gamepad;
      result.dwPacketNumber = caps->last_state.dwPacketNumber = caps->last_state.dwPacketNumber + 1;
    }

    return result;
  }

  std::optional<XINPUT_STATE> get_xinput_state(HANDLE file_handle)
  {
    std::array<BYTE, 3> request = { { 0x01, 0x01, 0x00 } };

    std::array<BYTE, 29> response{};
    DWORD size;
    if (!DeviceIoControl(file_handle, 0x8000e00c, request.data(), (DWORD)request.size(), response.data(), (DWORD)response.size(), &size, NULL) || size != response.size())
    {
      return std::nullopt;
    }

    XINPUT_STATE result{};
    std::memcpy(&result.dwPacketNumber, response.data() + 5, sizeof(result.dwPacketNumber));

    std::memcpy(&result.Gamepad, response.data() + 11, sizeof(result.Gamepad));

    return result;
  }

  bool populate_caps(controller_state& state)
  {
    if (state.caps.has_value())
    {
      return true;
    }

    HANDLE handle = INVALID_HANDLE_VALUE;
    bool keep_handle = false;

    if (state.info.detected_context == hardware_context::controller_xbox && state.info.device_path.contains(L"IG_"))
    {
      handle = ::CreateFileW(state.info.device_path.data(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
      keep_handle = true;
    }
    else
    {
      handle = ::CreateFileW(state.info.device_path.data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    }

    PHIDP_PREPARSED_DATA preparsed = nullptr;
    std::shared_ptr<void> deferred{
      nullptr, [&](...) {
        if (!keep_handle)
        {
          ::CloseHandle(handle);
        }
        if (preparsed)
        {
          ::HidD_FreePreparsedData(preparsed);
        }
      }
    };

    hid_caps caps{};

    if (!::HidD_GetPreparsedData(handle, &preparsed))
    {
      return false;
    }

    auto status = ::HidP_GetCaps(preparsed, &caps.caps);

    if (status != HIDP_STATUS_SUCCESS)
    {
      return false;
    }

    auto caps_size = caps.caps.NumberInputButtonCaps;
    caps.button_caps.resize(caps_size);
    status = ::HidP_GetButtonCaps(HidP_Input, caps.button_caps.data(), &caps_size, preparsed);

    if (status != HIDP_STATUS_SUCCESS)
    {
      return false;
    }

    caps_size = caps.caps.NumberInputValueCaps;
    caps.value_caps.resize(caps_size);
    status = ::HidP_GetValueCaps(HidP_Input, caps.value_caps.data(), &caps_size, preparsed);

    if (status != HIDP_STATUS_SUCCESS)
    {
      return false;
    }

    for (auto& button : caps.button_caps)
    {
      caps.button_usages[&button].reserve(button.Range.UsageMax - button.Range.UsageMin + 1);
    }

    caps.value_usages.resize(caps.value_caps.size());

    caps.has_triggers_as_buttons = state.info.detected_context != hardware_context::controller_xbox;

    if (state.info.detected_context == hardware_context::controller_playstation_4)
    {
      caps.trigger_value_count = 2;
    }
    else if (state.info.detected_context == hardware_context::controller_xbox)
    {
      caps.trigger_value_count = 1;
    }
    if (keep_handle)
    {
      caps.file_handle = std::shared_ptr<void>(handle, CloseHandle);
    }
    state.caps = std::move(caps);

    return true;
  }

  bool is_non_controller_context(hardware_context hint)
  {
    return (int)hint >= (int)hardware_context::global && (int)hint <= (int)hardware_context::mouse_wheel;
  }
  controller_info store_controller_context(const controller_info& info, siege::platform::hardware_context context)
  {
    if (is_non_controller_context(context))
    {
      return info;
    }

    auto result = info;

    result.detected_context = context;

    std::wstring key = L"Software\\The Siege Hub\\Siege Studio\\ControllerCache";
    HKEY user_key = nullptr;
    HKEY main_key = nullptr;
    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_SET_VALUE;

    key = key + L"\\" + std::to_wstring(result.vendor_product_id.first) + std::wstring(L"\\") + std::to_wstring(result.vendor_product_id.second);
    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, key.c_str(), 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      auto str = context_to_string(result.detected_context);
      ::RegSetValueExW(main_key, L"ControllerContext", 0, REG_SZ, (BYTE*)str.data(), str.size() * sizeof(wchar_t));

      result.get_hardware_index = hardware_index_lookup_from_context(result.detected_context, main_key);

      ::RegCloseKey(main_key);
      ::RegCloseKey(user_key);
    }

    return result;
  }
  controller_info detect_and_store_controller_context_from_hint(const controller_info& info, hardware_context hint)
  {
    if (is_non_controller_context(hint))
    {
      return info;
    }

    auto result = info;

    result.detected_context = hint;

    if (hint == hardware_context::controller_playstation_3 || hint == hardware_context::controller_playstation_4)
    {
      // TODO get device caps instead
      if (result.device_name.contains(L"4"))
      {
        result.detected_context = hardware_context::controller_playstation_4;
      }
      else
      {
        result.detected_context = hardware_context::controller_playstation_3;
      }
    }

    return store_controller_context(result, hint);
  }
  std::set<std::uint32_t> mapped_buttons(const controller_info& info)
  {
    if (!info.get_hardware_index)
    {
      return {};
    }
    std::set<std::uint32_t> indexes;
    for (auto i = std::to_underlying(vkey::gamepad_a); i <= std::to_underlying(vkey::gamepad_right_thumbstick_left); ++i)
    {
      auto value = info.get_hardware_index(static_cast<vkey>(i), controller_info::button_preference::prefer_button);

      if (!value)
      {
        continue;
      }
      if (value->type != input_type::button)
      {
        continue;
      }
      indexes.emplace(value->index);
    }

    return indexes;
  }
  std::set<std::uint32_t> mapped_axes(const controller_info& info)
  {
    if (!info.get_hardware_index)
    {
      return {};
    }
    std::set<std::uint32_t> indexes;
    for (auto i = std::to_underlying(vkey::gamepad_a); i <= std::to_underlying(vkey::gamepad_right_thumbstick_left); ++i)
    {
      auto value = info.get_hardware_index(static_cast<vkey>(i), controller_info::button_preference::prefer_button);

      if (!value)
      {
        continue;
      }
      if (value->type != input_type::axis)
      {
        continue;
      }
      indexes.emplace(value->index);
    }

    return indexes;
  }
  std::uint32_t mapped_hat_count(const controller_info& info)
  {
    if (!info.get_hardware_index)
    {
      return 0;
    }

    std::set<siege::views::hardware_index> indexes;
    for (auto i = std::to_underlying(vkey::gamepad_a); i <= std::to_underlying(vkey::gamepad_right_thumbstick_left); ++i)
    {
      auto value = info.get_hardware_index(static_cast<vkey>(i), controller_info::button_preference::prefer_button);

      if (!value)
      {
        continue;
      }
      if (value->type != input_type::hat)
      {
        continue;
      }
      indexes.emplace(*value);
    }

    return (std::uint32_t)indexes.size();
  }
  std::span<input_state_change> get_changes(const XINPUT_STATE& a, const XINPUT_STATE& b, std::span<input_state_change> buffer)
  {
    std::size_t result = 0;
    if (a.Gamepad.wButtons != b.Gamepad.wButtons)
    {
      for (auto& button : vk_to_xinput_buttons)
      {
        bool a_is_set = a.Gamepad.wButtons & button.second;
        bool b_is_set = b.Gamepad.wButtons & button.second;

        if (a_is_set != b_is_set)
        {
          buffer[result++] = input_state_change{
            .vkey = button.first,
            .new_value = b_is_set ? std::numeric_limits<std::uint16_t>::max() : std::uint16_t{ 0 },
            .old_value = a_is_set ? std::numeric_limits<std::uint16_t>::max() : std::uint16_t{ 0 },
          };
        }
      }
    }

    auto process_axis = [&](std::int16_t a, std::int16_t b, vkey key_positive, vkey key_negative) {
      if (a == b)
      {
        return;
      }

      auto scale_value = [](auto value) { return static_cast<std::uint16_t>(static_cast<std::uint16_t>(value) * 2 + 1); };

      if (b > 0)
      {
        buffer[result++] = input_state_change{
          .vkey = key_positive,
          .new_value = scale_value(b),
          .old_value = a > 0 ? scale_value(a) : std::uint16_t{ 0 }
        };
        buffer[result++] = input_state_change{
          .vkey = key_negative,
          .new_value = 0,
          .old_value = a < 0 ? scale_value(-static_cast<std::uint16_t>(a)) : std::uint16_t{ 0 }
        };
      }
      else
      {
        b = b + 1;
        buffer[result++] = input_state_change{
          .vkey = key_negative,
          .new_value = scale_value(-static_cast<std::uint16_t>(b)),
          .old_value = a < 0 ? scale_value(-static_cast<std::uint16_t>(a)) : std::uint16_t{ 0 }
        };

        buffer[result++] = input_state_change{
          .vkey = key_positive,
          .new_value = 0,
          .old_value = a > 0 ? scale_value(a) : std::uint16_t{ 0 }
        };
      }
    };

    process_axis(a.Gamepad.sThumbLX, b.Gamepad.sThumbLX, vkey::gamepad_left_thumbstick_right, vkey::gamepad_left_thumbstick_left);
    process_axis(a.Gamepad.sThumbLY, b.Gamepad.sThumbLY, vkey::gamepad_left_thumbstick_up, vkey::gamepad_left_thumbstick_down);
    process_axis(a.Gamepad.sThumbRX, b.Gamepad.sThumbRX, vkey::gamepad_right_thumbstick_right, vkey::gamepad_right_thumbstick_left);
    process_axis(a.Gamepad.sThumbRY, b.Gamepad.sThumbRY, vkey::gamepad_right_thumbstick_up, vkey::gamepad_right_thumbstick_down);

    if (a.Gamepad.bLeftTrigger != b.Gamepad.bLeftTrigger)
    {
      auto trigger_b = (std::uint16_t)b.Gamepad.bLeftTrigger;
      auto trigger_a = (std::uint16_t)b.Gamepad.bLeftTrigger;
      buffer[result++] = input_state_change{
        .vkey = vkey::gamepad_left_trigger,
        .new_value = static_cast<std::uint16_t>(trigger_b * 257),
        .old_value = static_cast<std::uint16_t>(trigger_a * 257)
      };
    }

    if (a.Gamepad.bRightTrigger != b.Gamepad.bRightTrigger)
    {
      auto trigger_b = (std::uint16_t)b.Gamepad.bRightTrigger;
      auto trigger_a = (std::uint16_t)a.Gamepad.bRightTrigger;
      buffer[result++] = input_state_change{
        .vkey = vkey::gamepad_right_trigger,
        .new_value = static_cast<std::uint16_t>(trigger_b * 257),
        .old_value = static_cast<std::uint16_t>(trigger_a * 257)
      };
    }

    return std::span(buffer.begin(), buffer.begin() + result);
  }

  constexpr USAGE to_index(USAGE value)
  {
    return value - HID_USAGE_GENERIC_X;
  }

  winmm_hardware_index map_hid_to_winmm(hardware_index index, const std::set<std::uint32_t>& axis_indexes)
  {
    winmm_hardware_index result{};
    result.index = index.index;
    result.type = index.type;

    if (result.type == input_type::axis)
    {
      static_assert(sizeof(JOYINFOEX::dwXpos) == sizeof(JOYINFOEX::dwRpos));
      static_assert(sizeof(JOYINFOEX::dwXpos) == sizeof(JOYINFOEX::dwRpos));
      static_assert(sizeof(JOYINFOEX::dwXpos) == sizeof(JOYINFOEX::dwUpos));
      static_assert(sizeof(JOYINFOEX::dwXpos) == sizeof(JOYINFOEX::dwVpos));
      constexpr auto x_offset = offsetof(JOYINFOEX, dwXpos);
      constexpr auto r_offset = offsetof(JOYINFOEX, dwRpos);
      constexpr auto u_offset = offsetof(JOYINFOEX, dwUpos);
      constexpr auto v_offset = offsetof(JOYINFOEX, dwVpos);

      // derived from notes here: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)
      // and here: https://learn.microsoft.com/en-us/previous-versions/dd757112(v=vs.85)
      const auto is_rotation = [](auto index) {
        return index == to_index(HID_USAGE_GENERIC_RZ) || index == to_index(HID_USAGE_GENERIC_RY) || index == to_index(HID_USAGE_GENERIC_RX);
      };

      std::map<std::uint32_t, std::uint32_t> rotations;

      std::uint32_t to_add = 0;
      for (auto iter = axis_indexes.rbegin(); iter != axis_indexes.rend(); ++iter)
      {
        if (!is_rotation(*iter))
        {
          continue;
        }

        if (rotations.size() == 3)
        {
          continue;
        }

        auto r_index = (r_offset - x_offset) / sizeof(JOYINFOEX::dwXpos);

        rotations.emplace(*iter, r_index + to_add);
        ++to_add;
      }

      if (is_rotation(result.index))
      {
        if (rotations.size() == 1)
        {
          result.index = (r_offset - x_offset) / sizeof(JOYINFOEX::dwXpos);
        }
        else
        {
          auto iter = rotations.find(result.index);

          if (iter != rotations.end())
          {
            result.index = iter->second;
          }
        }
      }
    }
    return result;
  }

  constexpr LONG to_s32(USHORT bit_size, ULONG usage)
  {
    if (bit_size == 16)
    {
      std::int64_t scaled = (std::int64_t)usage * (65535LL + 2);

      return (std::int32_t)(scaled + std::numeric_limits<std::int32_t>::min());
    }

    if (bit_size == 8)
    {
      return to_s32(16, usage * 257);
    }

    return 0;
  }

  static_assert(to_s32(16, 65535) == std::numeric_limits<std::int32_t>::max());
  static_assert(to_s32(16, 0) == std::numeric_limits<std::int32_t>::min());
  static_assert(to_s32(8, 255) == std::numeric_limits<std::int32_t>::max());
  static_assert(to_s32(8, 0) == std::numeric_limits<std::int32_t>::min());

  constexpr SHORT to_s16(USHORT bit_size, ULONG usage)
  {
    if (bit_size == 16)
    {
      return static_cast<std::int16_t>(usage - 32768);
    }

    if (bit_size == 8)
    {
      return to_s16(16, usage * 257);
    }

    return 0;
  }

  static_assert(to_s16(16, 65535) == std::numeric_limits<std::int16_t>::max());
  static_assert(to_s16(16, 0) == std::numeric_limits<std::int16_t>::min());
  static_assert(to_s16(8, 255) == std::numeric_limits<std::int16_t>::max());
  static_assert(to_s16(8, 0) == std::numeric_limits<std::int16_t>::min());

  constexpr BYTE to_byte(USHORT bit_size, ULONG usage)
  {
    if (bit_size == 16)
    {
      usage = usage / 257;
      if (usage > 255)
      {
        usage = 255;
      }

      return (BYTE)usage;
    }

    if (bit_size == 8)
    {
      return (BYTE)usage;
    }

    return 0;
  }

  static_assert(to_byte(16, 65535) == 255);
  static_assert(to_byte(16, 0) == 0);
  static_assert(to_byte(8, 255) == 255);
  static_assert(to_byte(8, 0) == 0);

  constexpr std::pair<BYTE, BYTE> to_byte_pair(USHORT bit_size, ULONG usage)
  {
    if (bit_size == 16)
    {
      auto value = to_s16(bit_size, usage);

      if (value > 0)
      {
        value = value * 2 + 1;
        return { (BYTE)(value / 257), 0 };
      }
      else
      {
        value = -(value + 1);
        value = value * 2 + 1;
        return { 0, (BYTE)(value / 257) };
      }
    }
    // TODO other bit sizes
    return {};
  }

  bool is_vkey_for_controller(vkey key)
  {
    return key >= vkey::gamepad_a && key <= vkey::gamepad_right_thumbstick_left;
  }

  bool is_vkey_for_mouse(vkey key)
  {
    return key == vkey::lbutton || key == vkey::rbutton || key == vkey::mbutton || key == vkey::xbutton1 || key == vkey::xbutton2;
  }

  bool is_vkey_for_keyboard(vkey key)
  {
    return !is_vkey_for_mouse(key) && !is_vkey_for_controller(key);
  }

  std::set<std::wstring>& strings()
  {
    static std::set<std::wstring> storage;
    return storage;
  }
  std::wstring category_for_vkey(vkey key, siege::platform::hardware_context context)
  {
    using namespace siege::platform;
    if (key >= vkey::lbutton && key <= vkey::xbutton2)
    {
      return L"Mouse";
    }

    if (context == hardware_context::mouse || context == hardware_context::mouse_wheel)
    {
      return L"Mouse";
    }

    if (key >= vkey::gamepad_a && key <= vkey::gamepad_right_thumbstick_left)
    {
      return L"Controller";
    }

    if (context == hardware_context::controller_xbox || context == hardware_context::controller_playstation_3 || context == hardware_context::controller_playstation_4 || context == hardware_context::controller_nintendo || context == hardware_context::steering_wheel || context == hardware_context::pedal || context == hardware_context::joystick || context == hardware_context::throttle)
    {
      return L"Controller";
    }

    return L"Keyboard";
  }
  std::wstring label_for_vkey(vkey key, siege::platform::controller_context context)
  {
    static std::map<std::wstring_view, vkey> xbox_labels{
      { L"A Button", vkey::gamepad_a },
      { L"B Button", vkey::gamepad_b },
      { L"X Button", vkey::gamepad_x },
      { L"Y Button", vkey::gamepad_y },
      { L"Left Bumper", vkey::gamepad_left_shoulder },
      { L"Right Bumper", vkey::gamepad_right_shoulder },
      { L"Left Trigger", vkey::gamepad_left_trigger },
      { L"Right Trigger", vkey::gamepad_right_trigger },
      { L"D-pad Up", vkey::gamepad_dpad_up },
      { L"D-pad Down", vkey::gamepad_dpad_down },
      { L"D-pad Left", vkey::gamepad_dpad_left },
      { L"D-pad Right", vkey::gamepad_dpad_right },
      { L"Left Stick Button", vkey::gamepad_left_thumbstick_button },
      { L"Left Stick Up", vkey::gamepad_left_thumbstick_up },
      { L"Left Stick Down", vkey::gamepad_left_thumbstick_down },
      { L"Left Stick Left", vkey::gamepad_left_thumbstick_left },
      { L"Left Stick Right", vkey::gamepad_left_thumbstick_right },
      { L"Right Stick Button", vkey::gamepad_right_thumbstick_button },
      { L"Right Stick Up", vkey::gamepad_right_thumbstick_up },
      { L"Right Stick Down", vkey::gamepad_right_thumbstick_down },
      { L"Right Stick Left", vkey::gamepad_right_thumbstick_left },
      { L"Right Stick Right", vkey::gamepad_right_thumbstick_right },
      { L"View Button", vkey::gamepad_view },
      { L"Menu Button", vkey::gamepad_menu },
    };

    static std::map<std::wstring_view, vkey> playstation_labels{
      { L"Cross Button", vkey::gamepad_a },
      { L"Circle Button", vkey::gamepad_b },
      { L"Square Button", vkey::gamepad_x },
      { L"Triangle Button", vkey::gamepad_y },
      { L"L1", vkey::gamepad_left_shoulder },
      { L"R1", vkey::gamepad_right_shoulder },
      { L"L2", vkey::gamepad_left_trigger },
      { L"R2", vkey::gamepad_right_trigger },
      { L"L3", vkey::gamepad_left_thumbstick_button },
      { L"R3", vkey::gamepad_right_thumbstick_button },
      { L"Select Button", vkey::gamepad_view },
      { L"Start Button", vkey::gamepad_menu },
    };

    if (context == controller_context::controller_playstation_3 || context == controller_context::controller_playstation_4)
    {
      auto label = std::find_if(playstation_labels.begin(), playstation_labels.end(), [key](auto& name) { return name.second == key; });

      if (label != playstation_labels.end())
      {
        return std::wstring{ label->first };
      }
    }

    auto label = std::find_if(xbox_labels.begin(), xbox_labels.end(), [key](auto& name) { return name.second == key; });

    if (label == xbox_labels.end())
    {
      return L"";
    }

    return std::wstring{ label->first };
  }

  std::wstring label_for_vkey(vkey key, siege::platform::hardware_context context)
  {
    constexpr static auto directions = std::array<std::wstring_view, 4>{
      {
        std::wstring_view(L"Left"),
        std::wstring_view(L"Up"),
        std::wstring_view(L"Right"),
        std::wstring_view(L"Down"),
      }
    };

    if (context == siege::platform::hardware_context::mouse)
    {
      if (key >= vkey::left && key <= vkey::down)
      {
        std::wstring result(L"Move ");
        result.append(directions[std::to_underlying(key) - std::to_underlying(vkey::left)]);
        return result;
      }
    }

    if (context == siege::platform::hardware_context::mouse_wheel)
    {
      if (key >= vkey::left && key <= vkey::down)
      {
        std::wstring result(L"Scroll ");
        result.append(directions[std::to_underlying(key) - std::to_underlying(vkey::left)]);
        return result;
      }
    }

    if (key >= vkey::numpad_0 && key <= vkey::numpad_9)
    {
      std::wstring result(L"Numpad ");
      result.push_back(L'0' + static_cast<wchar_t>(std::to_underlying(key) - std::to_underlying(vkey::numpad_0)));
      return result;
    }

    if (key >= vkey::f1 && key <= vkey::f9)
    {
      std::wstring result(1, 'F');
      result.push_back(static_cast<wchar_t>(L'1' + std::to_underlying(key) - std::to_underlying(vkey::f1)));
      return result;
    }

    if (key >= vkey::f10 && key <= vkey::f19)
    {
      std::wstring result(1, 'F');
      result.push_back(L'1');
      result.push_back(static_cast<wchar_t>(L'0' + std::to_underlying(key) - std::to_underlying(vkey::f10)));
      return result;
    }

    if (key >= vkey::f20 && key <= vkey::f24)
    {
      std::wstring result(1, 'F');
      result.push_back(L'2');
      result.push_back(L'0' + static_cast<wchar_t>(std::to_underlying(key) - std::to_underlying(vkey::f20)));
      return result;
    }

    if (key >= vkey::gamepad_a && key <= vkey::gamepad_right_thumbstick_left)
    {
      return label_for_vkey(key, (siege::platform::controller_context)context);
    }

    switch (key)
    {
    case vkey::lbutton:
      return L"Left Button";
    case vkey::rbutton:
      return L"Right Button";
    case vkey::mbutton:
      return L"Middle Button";
    case vkey::xbutton1:
      return L"Extra Button 1";
    case vkey::xbutton2:
      return L"Extra Button 2";
    case vkey::tab:
      return L"Tab";
    case vkey::control:
      return L"Left/Right Control";
    case vkey::left_control:
      return L"Left Control";
    case vkey::right_control:
      return L"Right Control";
    case vkey::alt:
      return L"Left/Right Alt";
    case vkey::left_alt:
      return L"Left Alt";
    case vkey::right_alt:
      return L"Right Alt";
    case vkey::shift:
      return L"Left/Right Shift";
    case vkey::left_shift:
      return L"Left Shift";
    case vkey::right_shift:
      return L"Right Shift";
    case vkey::left_win:
      return L"Left Windows Key";
    case vkey::right_win:
      return L"Right Windows Key";
    case vkey::enter:
      return L"Enter";
    case vkey::up:
      return L"Up Arrow";
    case vkey::down:
      return L"Down Arrow";
    case vkey::left:
      return L"Left Arrow";
    case vkey::right:
      return L"Right Arrow";
    case vkey::space:
      return L"Spacebar";
    case vkey::snapshot:
      return L"Print Screen";
    case vkey::caps_lock:
      return L"Caps Lock";
    case vkey::num_lock:
      return L"Num Lock";
    case vkey::scroll_lock:
      return L"Scroll Lock";
    case vkey::home:
      return L"Home";
    case vkey::del:
      return L"Delete";
    case vkey::escape:
      return L"Escape";
    case vkey::insert:
      return L"Insert";
    case vkey::page_up:
      return L"Page Down";
    case vkey::page_down:
      return L"Page Up";
    case vkey::pause:
      return L"Pause";
    case vkey::backspace:
      return L"Backspace";
    default:
      return std::wstring(1, ::MapVirtualKeyW(std::to_underlying(key), MAPVK_VK_TO_CHAR));
    }
  }
  std::function<std::optional<hardware_index>(vkey, controller_info::button_preference)> hardware_index_lookup_from_context(hardware_context context, HKEY cache)
  {
    if (context == hardware_context::controller_xbox)
    {
      return hardware_index_for_xbox_vkey;
    }
    else if (context == hardware_context::controller_playstation_3)
    {
      return hardware_index_for_ps3_vkey;
    }
    else if (context == hardware_context::controller_playstation_4)
    {
      return hardware_index_for_ps4_vkey;
    }
    else if (context == hardware_context::joystick)
    {
      return hardware_index_for_joystick_vkey;
    }
    else if (context == hardware_context::custom)
    {
      return hardware_index_from_cache(cache);
    }

    return nullptr;
  }
}// namespace siege::views
