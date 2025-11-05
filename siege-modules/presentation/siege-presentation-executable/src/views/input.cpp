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

  hardware_index hardware_index_for_xbox_vkey(SHORT vkey, controller_info::button_preference);
  hardware_index hardware_index_for_ps3_vkey(SHORT vkey, controller_info::button_preference);
  hardware_index hardware_index_for_ps4_vkey(SHORT vkey, controller_info::button_preference);
  hardware_context string_to_context(std::wstring_view value);
  std::wstring_view context_to_string(hardware_context value);

  std::set<std::wstring>& strings();
  std::optional<controller_info> controller_info_for_raw_input_device_handle(HANDLE handle);

  constexpr static auto vk_to_xinput_buttons = std::array<std::pair<WORD, SHORT>, 14>{
    {
      { VK_GAMEPAD_A, XINPUT_GAMEPAD_A },
      { VK_GAMEPAD_B, XINPUT_GAMEPAD_B },
      { VK_GAMEPAD_X, XINPUT_GAMEPAD_X },
      { VK_GAMEPAD_Y, XINPUT_GAMEPAD_Y },
      { VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, XINPUT_GAMEPAD_LEFT_THUMB },
      { VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, XINPUT_GAMEPAD_RIGHT_THUMB },
      { VK_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_LEFT_SHOULDER },
      { VK_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER },
      { VK_GAMEPAD_MENU, XINPUT_GAMEPAD_START },
      { VK_GAMEPAD_VIEW, XINPUT_GAMEPAD_BACK },
      { VK_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_UP },
      { VK_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN },
      { VK_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT },
      { VK_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT },
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
          ::RegCloseKey(main_key);
          ::RegCloseKey(user_key);

          result.detected_context = string_to_context(buffer);

          if (result.detected_context == hardware_context::controller_xbox)
          {
            result.get_hardware_index = hardware_index_for_xbox_vkey;
          }
          else if (result.detected_context == hardware_context::controller_playstation_3)
          {
            result.get_hardware_index = hardware_index_for_ps3_vkey;
          }
          else if (result.detected_context == hardware_context::controller_playstation_4)
          {
            result.get_hardware_index = hardware_index_for_ps4_vkey;
          }
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

  SHORT to_s16(USHORT, ULONG value_usages);
  std::pair<BYTE, BYTE> to_byte_pair(USHORT, ULONG value_usages);
  BYTE to_byte(USHORT caps, ULONG usage);

  std::optional<XINPUT_STATE> get_xinput_state(HANDLE file_handle);

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
            if (button.first >= VK_GAMEPAD_DPAD_UP && button.first <= VK_GAMEPAD_DPAD_RIGHT)
            {
              continue;
            }

            if (state.info.get_hardware_index(button.first, controller_info::prefer_button).index == usage)
            {
              result.Gamepad.wButtons |= button.second;
              break;
            }

            if (caps->has_triggers_as_buttons && state.info.get_hardware_index(VK_GAMEPAD_LEFT_TRIGGER, controller_info::prefer_button).index == usage)
            {
              result.Gamepad.bLeftTrigger = 255;
            }

            if (caps->has_triggers_as_buttons && state.info.get_hardware_index(VK_GAMEPAD_RIGHT_TRIGGER, controller_info::prefer_button).index == usage)
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
        if (state.info.get_hardware_index(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, controller_info::prefer_value).index == index)
        {
          result.Gamepad.sThumbLX = to_s16(value.BitSize, usage);
        }
        else if (state.info.get_hardware_index(VK_GAMEPAD_LEFT_THUMBSTICK_UP, controller_info::prefer_value).index == index)
        {
          result.Gamepad.sThumbLY = -to_s16(value.BitSize, usage);
        }
        else if (state.info.get_hardware_index(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, controller_info::prefer_value).index == index)
        {
          result.Gamepad.sThumbRX = to_s16(value.BitSize, usage);
        }
        else if (state.info.get_hardware_index(VK_GAMEPAD_RIGHT_THUMBSTICK_UP, controller_info::prefer_value).index == index)
        {
          result.Gamepad.sThumbRY = -to_s16(value.BitSize, usage);
        }
        else if (caps->trigger_value_count == 1 && state.info.get_hardware_index(VK_GAMEPAD_LEFT_TRIGGER, controller_info::prefer_value).index == index)
        {
          auto [left, right] = to_byte_pair(value.BitSize, usage);
          result.Gamepad.bLeftTrigger = left;
          result.Gamepad.bRightTrigger = right;
        }
        else if (caps->trigger_value_count >= 2 && state.info.get_hardware_index(VK_GAMEPAD_LEFT_TRIGGER, controller_info::prefer_value).index == index)
        {
          result.Gamepad.bLeftTrigger = to_byte(value.BitSize, usage);
        }
        else if (caps->trigger_value_count >= 2 && state.info.get_hardware_index(VK_GAMEPAD_RIGHT_TRIGGER, controller_info::prefer_value).index == index)
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
        result.get_hardware_index = hardware_index_for_ps4_vkey;
      }
      else
      {
        result.detected_context = hardware_context::controller_playstation_3;
        result.get_hardware_index = hardware_index_for_ps3_vkey;
      }
    }

    std::wstring key = L"Software\\The Siege Hub\\Siege Studio\\ControllerCache";
    HKEY user_key = nullptr;
    HKEY main_key = nullptr;
    auto access = KEY_QUERY_VALUE | KEY_READ | KEY_SET_VALUE;

    key = key + L"\\" + std::to_wstring(result.vendor_product_id.first) + std::wstring(L"\\") + std::to_wstring(result.vendor_product_id.second);
    if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, key.c_str(), 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
    {
      auto str = context_to_string(result.detected_context);
      ::RegSetValueExW(main_key, L"ControllerContext", 0, REG_SZ, (BYTE*)str.data(), str.size() * sizeof(wchar_t));

      ::RegCloseKey(main_key);
      ::RegCloseKey(user_key);
    }

    return result;
  }

  std::span<std::pair<WORD, std::uint16_t>> get_changes(const XINPUT_STATE& a, const XINPUT_STATE& b, std::span<std::pair<WORD, std::uint16_t>> buffer)
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
          buffer[result++] = (std::make_pair(button.first, b_is_set ? std::numeric_limits<std::uint16_t>::max() : 0));
        }
      }
    }

    auto process_axis = [&](auto a, auto b, auto key_positive, auto key_negative) {
      if (a == b)
      {
        return;
      }

      if (b > 0)
      {
        auto new_value = static_cast<std::uint16_t>(b);
        buffer[result++] = (std::make_pair(key_positive, static_cast<std::uint16_t>(b) * 2 + 1));
      }
      else
      {
        b = b + 1;
        buffer[result++] = (std::make_pair(key_negative, -static_cast<std::uint16_t>(b) * 2 + 1));
      }
    };

    process_axis(a.Gamepad.sThumbLX, b.Gamepad.sThumbLX, VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, VK_GAMEPAD_LEFT_THUMBSTICK_LEFT);
    process_axis(a.Gamepad.sThumbLY, b.Gamepad.sThumbLY, VK_GAMEPAD_LEFT_THUMBSTICK_UP, VK_GAMEPAD_LEFT_THUMBSTICK_DOWN);
    process_axis(a.Gamepad.sThumbRX, b.Gamepad.sThumbRX, VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT);
    process_axis(a.Gamepad.sThumbRY, b.Gamepad.sThumbRY, VK_GAMEPAD_RIGHT_THUMBSTICK_UP, VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN);

    if (a.Gamepad.bLeftTrigger != b.Gamepad.bLeftTrigger)
    {
      auto trigger = (std::uint16_t)b.Gamepad.bLeftTrigger;
      buffer[result++] = (std::make_pair(VK_GAMEPAD_LEFT_TRIGGER, trigger * 257));
    }

    if (a.Gamepad.bRightTrigger != b.Gamepad.bRightTrigger)
    {
      auto trigger = (std::uint16_t)b.Gamepad.bRightTrigger;
      buffer[result++] = (std::make_pair(VK_GAMEPAD_RIGHT_TRIGGER, trigger * 257));
    }

    return std::span(buffer.begin(), buffer.begin() + result);
  }

  hardware_context string_to_context(std::wstring_view value)
  {
    if (value == L"xbox")
    {
      return hardware_context::controller_xbox;
    }

    if (value == L"playstation_3")
    {
      return hardware_context::controller_playstation_3;
    }

    if (value == L"playstation_4")
    {
      return hardware_context::controller_playstation_4;
    }

    if (value == L"pro_controller")
    {
      return hardware_context::controller_nintendo;
    }

    return hardware_context::global;
  }

  std::wstring_view context_to_string(hardware_context value)
  {
    if (value == hardware_context::controller_xbox)
    {
      return L"xbox";
    }

    if (value == hardware_context::controller_playstation_3)
    {
      return L"playstation_3";
    }

    if (value == hardware_context::controller_playstation_4)
    {
      return L"playstation_4";
    }

    if (value == hardware_context::controller_nintendo)
    {
      return L"pro_controller";
    }

    return L"global";
  }


  constexpr USAGE to_index(USAGE value)
  {
    return value - HID_USAGE_GENERIC_X;
  }

  hardware_index hardware_index_for_xbox_vkey(SHORT vkey, controller_info::button_preference)
  {
    switch (vkey)
    {
    case VK_GAMEPAD_A:
      return { hardware_index::button, 0 };
    case VK_GAMEPAD_B:
      return { hardware_index::button, 1 };
    case VK_GAMEPAD_X:
      return { hardware_index::button, 2 };
    case VK_GAMEPAD_Y:
      return { hardware_index::button, 3 };
    case VK_GAMEPAD_LEFT_SHOULDER:
      return { hardware_index::button, 4 };
    case VK_GAMEPAD_RIGHT_SHOULDER:
      return { hardware_index::button, 5 };
    case VK_GAMEPAD_VIEW:
      return { hardware_index::button, 6 };
    case VK_GAMEPAD_MENU:
      return { hardware_index::button, 7 };
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
      return { hardware_index::button, 8 };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
      return { hardware_index::button, 9 };
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_X) };
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_Y) };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_RX) };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_RY) };
    case VK_GAMEPAD_LEFT_TRIGGER:
    case VK_GAMEPAD_RIGHT_TRIGGER:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_Z) };
    default:
      return {};
    }
  }

  hardware_index hardware_index_for_ps3_vkey(SHORT vkey, controller_info::button_preference)
  {
    switch (vkey)
    {
    case VK_GAMEPAD_X:// ps3/ps4 square
      return { hardware_index::button, 0 };
    case VK_GAMEPAD_A:// ps3/ps4 cross
      return { hardware_index::button, 1 };
    case VK_GAMEPAD_B:// ps3/ps4 circle
      return { hardware_index::button, 2 };
    case VK_GAMEPAD_Y:// ps3/ps4 triangle
      return { hardware_index::button, 3 };
    case VK_GAMEPAD_LEFT_SHOULDER:
      return { hardware_index::button, 4 };
    case VK_GAMEPAD_RIGHT_SHOULDER:
      return { hardware_index::button, 5 };
    case VK_GAMEPAD_LEFT_TRIGGER:
      return { hardware_index::button, 6 };
    case VK_GAMEPAD_RIGHT_TRIGGER:
      return { hardware_index::button, 7 };
    case VK_GAMEPAD_VIEW:
      return { hardware_index::button, 8 };
    case VK_GAMEPAD_MENU:
      return { hardware_index::button, 9 };
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
      return { hardware_index::button, 10 };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
      return { hardware_index::button, 11 };
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_X) };
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_Y) };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_Z) };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_RZ) };
    default:
      return {};
    }
  }

  hardware_index hardware_index_for_ps4_vkey(SHORT vkey, controller_info::button_preference prefer_button)
  {
    switch (vkey)
    {
    case VK_GAMEPAD_X:// ps3/ps4 square
      return { hardware_index::button, 0 };
    case VK_GAMEPAD_A:// ps3/ps4 cross
      return { hardware_index::button, 1 };
    case VK_GAMEPAD_B:// ps3/ps4 circle
      return { hardware_index::button, 2 };
    case VK_GAMEPAD_Y:// ps3/ps4 triangle
      return { hardware_index::button, 3 };
    case VK_GAMEPAD_LEFT_SHOULDER:
      return { hardware_index::button, 4 };
    case VK_GAMEPAD_RIGHT_SHOULDER:
      return { hardware_index::button, 5 };
    case VK_GAMEPAD_VIEW:
      return { hardware_index::button, 8 };
    case VK_GAMEPAD_MENU:
      return { hardware_index::button, 9 };
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
      return { hardware_index::button, 10 };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
      return { hardware_index::button, 11 };
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_X) };
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_Y) };
    case VK_GAMEPAD_LEFT_TRIGGER: {
      if (prefer_button)
      {
        return { hardware_index::button, 6 };
      }

      return { hardware_index::value, to_index(HID_USAGE_GENERIC_RX) };
    }
    case VK_GAMEPAD_RIGHT_TRIGGER: {

      if (prefer_button)
      {
        return { hardware_index::button, 7 };
      }

      return { hardware_index::value, to_index(HID_USAGE_GENERIC_RY) };
    }
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_Z) };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
      return { hardware_index::value, to_index(HID_USAGE_GENERIC_RZ) };
    default:
      return {};
    }
  }

  winmm_hardware_index map_hid_to_winmm(hardware_index index)
  {
    winmm_hardware_index result{};
    std::memcpy(&result, &index, sizeof(index));

    if (result.type == hardware_index::value)
    {
      auto x_offset = offsetof(JOYINFOEX, dwXpos);
      // derived from notes here: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)
      // and here: https://learn.microsoft.com/en-us/previous-versions/dd757112(v=vs.85)
      if (result.index == to_index(HID_USAGE_GENERIC_RZ))
      {
        result.index = offsetof(JOYINFOEX, dwRpos) - x_offset;
      }
      else if (result.index == to_index(HID_USAGE_GENERIC_RY))
      {
        result.index = offsetof(JOYINFOEX, dwUpos) - x_offset;
      }
      else if (result.index == to_index(HID_USAGE_GENERIC_RX))
      {
        result.index = offsetof(JOYINFOEX, dwVpos) - x_offset;
      }
    }
    return result;
  }

  SHORT to_s16(USHORT bit_size, ULONG usage)
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

  BYTE to_byte(USHORT bit_size, ULONG usage)
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

  std::pair<BYTE, BYTE> to_byte_pair(USHORT bit_size, ULONG usage)
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

  bool is_vkey_for_controller(WORD vkey)
  {
    return vkey >= VK_GAMEPAD_A && vkey <= VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT;
  }

  std::set<std::wstring>& strings()
  {
    static std::set<std::wstring> storage;
    return storage;
  }

}// namespace siege::views