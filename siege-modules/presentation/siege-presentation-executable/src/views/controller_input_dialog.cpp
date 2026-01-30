#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/extension_module.hpp>
#include "exe_shared.hpp"
#include <hidusage.h>

namespace siege::views
{
  struct selected_input
  {
    WORD vkey;
    siege::platform::hardware_context context;
  };

  struct raw_selected_input
  {
    WORD hardware_index;
    input_type input_type;
  };

  std::optional<selected_input> get_controller_input_from_dialog(win32::window_ref parent_ref)
  {
    auto result = win32::DialogBoxIndirectParamW(win32::module_ref::current_application(),
      win32::default_dialog({ .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU,
        .cx = 200,
        .cy = 100 }),
      std::move(parent_ref),
      [controllers = std::map<HANDLE, controller_state>{}](win32::window_ref dialog, INT message, WPARAM wparam, LPARAM lparam) mutable -> std::optional<LRESULT> {
        switch (message)
        {
        case WM_INITDIALOG: {
          ::SetWindowTextW(dialog, L"Press a button on your controller or joystick");

          DWORD flags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
          std::array<RAWINPUTDEVICE, 2> descriptors{ {
            { .usUsagePage = HID_USAGE_PAGE_GENERIC,
              .usUsage = HID_USAGE_GENERIC_GAMEPAD,
              .dwFlags = flags,
              .hwndTarget = dialog },
            { .usUsagePage = HID_USAGE_PAGE_GENERIC,
              .usUsage = HID_USAGE_GENERIC_JOYSTICK,
              .dwFlags = flags,
              .hwndTarget = dialog },
          }

          };

          if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
          {
            assert(false);
          }
          return TRUE;
        }
        case WM_DESTROY: {

          std::array<RAWINPUTDEVICE, 2> descriptors{ {

            { .usUsagePage = HID_USAGE_PAGE_GENERIC,
              .usUsage = HID_USAGE_GENERIC_GAMEPAD,
              .dwFlags = RIDEV_REMOVE },
            {
              .usUsagePage = HID_USAGE_PAGE_GENERIC,
              .usUsage = HID_USAGE_GENERIC_JOYSTICK,
              .dwFlags = RIDEV_REMOVE,
            } } };

          if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
          {
            assert(false);
          }
          return 0;
        }
        case WM_CLOSE: {
          ::EndDialog(dialog, 0);
          return 0;
        }
        case WM_INPUT_DEVICE_CHANGE: {
          if (wparam == GIDC_ARRIVAL)
          {
            auto handle = (HANDLE)lparam;
            auto info = controller_info_for_raw_input_device_handle(handle);

            if (info)
            {
              controllers[handle] = controller_state{ *info };
            }
          }
          else if (wparam == GIDC_REMOVAL)
          {
            controllers.erase((HANDLE)lparam);
          }

          return 0;
        }
        case WM_INPUT: {
          if (::GetPropW(dialog, L"IsProcessing") == (HANDLE)1)
          {
            return 0;
          }

          RAWINPUTHEADER header{};

          UINT size = sizeof(header);
          if (::GetRawInputData((HRAWINPUT)lparam, RID_HEADER, &header, &size, sizeof(header)) == (UINT)-1)
          {
            return 0;
          }

          auto info = controllers.find(header.hDevice);

          if (info == controllers.end())
          {
            return 0;
          }

          if (!info->second.info.get_hardware_index)
          {
            ::SetPropW(dialog, L"IsProcessing", (HANDLE)1);
            using namespace siege::platform;
            std::vector<TASKDIALOG_BUTTON> default_buttons = {
              TASKDIALOG_BUTTON{ .nButtonID = (int)hardware_context::controller_playstation_4, .pszButtonText = L"Playstation" },
              TASKDIALOG_BUTTON{ .nButtonID = (int)hardware_context::joystick, .pszButtonText = L"Joystick" },
            };

            auto result = win32::task_dialog_indirect(TASKDIALOGCONFIG{
                                                        .cbSize = sizeof(TASKDIALOGCONFIG),
                                                        .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS,
                                                        .dwCommonButtons = TDCBF_CLOSE_BUTTON,
                                                        .pszWindowTitle = L"Select Controller Layout",
                                                        .pszMainInstruction = L"An unknown controller was detected. Please specify the layout below: ",
                                                        .cButtons = (UINT)default_buttons.size(),
                                                        .pButtons = default_buttons.data(),
                                                      },
              [](auto window, auto message, auto wparam, auto lparam) -> HRESULT {
                if (message == TDN_BUTTON_CLICKED && (wparam >= (int)hardware_context::controller_playstation_4 && wparam <= (int)hardware_context::joystick))
                {
                  return S_OK;
                }
                return S_FALSE;
              });

            if (result)
            {
              info->second.info = detect_and_store_controller_context_from_hint(info->second.info, (hardware_context)result->buttons[0]);
              info->second.last_state = get_current_state_for_handle(info->second, (HRAWINPUT)lparam);
            }

            ::SetPropW(dialog, L"IsProcessing", (HANDLE)0);
            ::OutputDebugStringW(L"Controller button reached");

            ::EndDialog(dialog, 0);
          }

          auto new_state = get_current_state_for_handle(info->second, (HRAWINPUT)lparam);

          if (info->second.last_state.dwPacketNumber != new_state.dwPacketNumber)
          {
            // TODO get updated value and return it here
            auto changes = get_changes(info->second.last_state, new_state, info->second.buffer);

            if (changes.empty())
            {
              return 0;
            }

            info->second.last_state = std::move(new_state);


            // remove axes which have not moved enough
            changes = std::span(changes.begin(), std::remove_if(changes.begin(), changes.end(), [](auto& item) {
              return item.new_value < std::numeric_limits<std::uint16_t>::max() / 2;
            }));

            if (changes.empty())
            {
              return 0;
            }

            // get the highest values
            std::sort(changes.begin(), changes.begin(), [](const auto& a, const auto& b) {
              return a.new_value < b.new_value;
            });

            // but prefer buttons
            std::stable_partition(changes.begin(), changes.begin(), [](auto& item) {
              return item.vkey >= VK_GAMEPAD_A && item.vkey <= VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON && item.new_value != 0;
            });

            ::EndDialog(dialog, MAKELRESULT(changes.begin()->vkey, info->second.info.detected_context));
          }

          return 0;
        }
        default:
          return std::nullopt;
        }
      });

    if (!result)
    {
      return std::nullopt;
    }

    return std::make_optional<selected_input>(
      LOWORD(result),
      static_cast<siege::platform::hardware_context>(HIWORD(result)));
  }

  std::optional<raw_selected_input> get_raw_input_from_dialog(win32::window_ref parent_ref, HANDLE controller, controller_state& state, std::optional<input_type> input_restriction)
  {
    auto result = win32::DialogBoxIndirectParamW(win32::module_ref::current_application(),
      win32::default_dialog({ .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU,
        .cx = 200,
        .cy = 100 }),
      std::move(parent_ref),
      [controller, state = &state, input_restriction](win32::window_ref dialog, INT message, WPARAM wparam, LPARAM lparam) mutable -> std::optional<LRESULT> {
        switch (message)
        {
        case WM_INITDIALOG: {
          ::SetWindowTextW(dialog, L"Press a button on your controller or joystick");

          DWORD flags = RIDEV_INPUTSINK;
          std::array<RAWINPUTDEVICE, 2> descriptors{ {
            { .usUsagePage = HID_USAGE_PAGE_GENERIC,
              .usUsage = HID_USAGE_GENERIC_GAMEPAD,
              .dwFlags = flags,
              .hwndTarget = dialog },
            { .usUsagePage = HID_USAGE_PAGE_GENERIC,
              .usUsage = HID_USAGE_GENERIC_JOYSTICK,
              .dwFlags = flags,
              .hwndTarget = dialog },
          }

          };

          if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
          {
            assert(false);
          }
          return TRUE;
        }
        case WM_CLOSE: {
          ::EndDialog(dialog, 0);
          return 0;
        }
        case WM_INPUT: {
          RAWINPUTHEADER header{};

          UINT size = sizeof(header);
          if (::GetRawInputData((HRAWINPUT)lparam, RID_HEADER, &header, &size, sizeof(header)) == (UINT)-1)
          {
            return 0;
          }

          if (controller != header.hDevice)
          {
            return 0;
          }

          auto new_state = get_raw_state_for_handle(*state, (HRAWINPUT)lparam);


          if (input_restriction == std::nullopt || input_restriction == input_type::button)
          {
            for (auto i = 0U; i < sizeof(new_state.rgbButtons); ++i)
            {
              if (new_state.rgbButtons[i])
              {
                ::EndDialog(dialog, MAKELRESULT(i, input_type::button));
                return 0;
              }
            }
          }

          if (input_restriction == std::nullopt || input_restriction == input_type::axis)
          {
            auto axes = std::array{ std::make_pair(0, new_state.lX),
              std::make_pair(1, new_state.lY),
              std::make_pair(2, new_state.lZ),
              std::make_pair(3, new_state.lRx),
              std::make_pair(4, new_state.lRy),
              std::make_pair(5, new_state.lRz) };

            // remove axes which have not moved enough
            auto changes = std::span(axes.begin(), std::remove_if(axes.begin(), axes.end(), [](const auto& value) {
              return std::abs(value.second) < std::numeric_limits<std::int32_t>::max() / 2;
            }));

            if (changes.empty())
            {
              return 0;
            }

            // get the highest values
            std::sort(changes.begin(), changes.begin(), [](const auto& a, const auto& b) {
              return a.second < b.second;
            });

            ::EndDialog(dialog, MAKELRESULT(changes[0].first, input_type::axis));
          }

          return 0;
        }
        default:
          return std::nullopt;
        }
      });

    if (!result)
    {
      return std::nullopt;
    }

    return std::make_optional<raw_selected_input>(
      LOWORD(result),
      static_cast<input_type>(HIWORD(result)));
  }

}// namespace siege::views