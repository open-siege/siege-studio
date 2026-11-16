module;
#include <siege/platform/windows.hpp>
#include <cassert>
#include <hidusage.h>
#include <xinput.h>

export module exe.input_injector;

import std;
import win32.capabilities;
import win32.common_controls;
import win32.window;
import siege.extension.input_filter;
import siege.platform;
import exe.shared;

namespace siege::views
{
  using vkey = siege::platform::vkey;
  using siege::views::controller_state;
  using siege::platform::hardware_context;

  export struct input_injector_args
  {
    siege::platform::owning_packaged_args args;
    std::wstring script_host;
    bool simulate_mouse;
    bool simulate_keyboard;
    std::function<HRESULT(siege::platform::packaged_args& game_args, PROCESS_INFORMATION* process_info)> launch_game_with_extension;
    std::function<void()> on_process_closed;

    double refresh_rate_hz = 60.0;

    // in mickeys
    double max_turn_speed = 750.0;

    double response_curve_exponent = 2.0;
  };

  export std::any bind_to_window(win32::window_ref, input_injector_args);

  enum class input_state
  {
    down,
    up
  };

  ::INPUT vk_to_input(vkey key, hardware_context context, std::uint32_t device_id, input_state state, int intensity);

  struct input_injector
  {
    HWND target_window;
    input_injector_args injector_args;
    HANDLE process_handle{};
    HANDLE thread_handle{};
    PROCESS_INFORMATION child_process{};
    std::vector<INPUT> simulated_inputs;

    std::set<HANDLE> registered_mice;
    std::set<HANDLE> registered_keyboards;
    std::map<HANDLE, controller_state> registered_controllers;
    std::map<std::string_view, bool> action_states;

    std::atomic<POINT> mouse_velocity{};
    HANDLE mouse_timer_handle{};
    HANDLE mouse_stop_event{};
    std::jthread mouse_thread;

    static_assert(std::atomic<POINT>::is_always_lock_free);

    input_injector(HWND target_window, input_injector_args args) : target_window(target_window), injector_args(std::move(args))
    {
      simulated_inputs.reserve(64);
      siege::init_active_input_state();

      win32::set_window_subclass(target_window, sub_class_callback, (UINT_PTR)this, (DWORD_PTR)this);

      DWORD flags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
      std::array<RAWINPUTDEVICE, 4> descriptors{ {
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_GAMEPAD,
          .dwFlags = flags,
          .hwndTarget = target_window },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_JOYSTICK,
          .dwFlags = flags,
          .hwndTarget = target_window },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_MOUSE,
          .dwFlags = flags,
          .hwndTarget = target_window },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_KEYBOARD,
          .dwFlags = flags | RIDEV_NOLEGACY,
          .hwndTarget = target_window },
      } };

      if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        auto error = GetLastError();
        DebugBreak();
      }

      try
      {
        siege::platform::packaged_args final_args = this->injector_args.args;
        if (this->injector_args.launch_game_with_extension(final_args, &child_process) == S_OK && child_process.hProcess)
        {
          auto& state = siege::get_active_input_state();

          for (auto& group : state.groups)
          {
            group.process_id = child_process.dwProcessId;
            group.thread_id = child_process.dwThreadId;
          }

          process_handle = child_process.hProcess;
          thread_handle = child_process.hThread;
        }
      }
      catch (...)
      {
      }

      mouse_stop_event = ::CreateEventW(nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, nullptr);

      if (!mouse_stop_event)
      {
        return;
      }

      mouse_timer_handle = ::CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);

      if (!mouse_timer_handle)
      {
        // in case we older than on windows 10
        mouse_timer_handle = ::CreateWaitableTimerExW(nullptr, nullptr, 0, TIMER_ALL_ACCESS);
      }

      const double refresh_hz = std::clamp(injector_args.refresh_rate_hz, 1.0, 60.0);
      const double tick_seconds = 1.0 / refresh_hz;
      
      if (mouse_timer_handle)
      {
        const LONG period_ms = std::clamp((LONG)std::lround(1000.0 / refresh_hz), 1L, 16L);
        LARGE_INTEGER due_time{};
        due_time.QuadPart = -1;
        ::SetWaitableTimer(mouse_timer_handle, &due_time, period_ms, nullptr, nullptr, FALSE);

        mouse_thread = std::jthread([this, tick_seconds] {
          std::array<HANDLE, 2> waits{ mouse_stop_event, mouse_timer_handle };

          // we keep these around
          // because smaller movements
          // will have left over units
          // that can be carried over into
          // future intervals.
          double remainder_x = 0.0;
          double remainder_y = 0.0;

          while (true)
          {
            auto signalled = ::WaitForMultipleObjects((DWORD)waits.size(), waits.data(), FALSE, INFINITE);

            // only caring about the timer and nothing else
            if (signalled != WAIT_OBJECT_0 + 1)
            {
              break;
            }

            const POINT velocity = mouse_velocity.load(std::memory_order_relaxed);

            if (velocity.x == 0 && velocity.y == 0)
            {
              remainder_x = 0.0;
              remainder_y = 0.0;
              continue;
            }

            // the new value
            // will contain anything that
            // couldn't be simulated last time
            // to help smooth out the movement.
            remainder_x += velocity.x * tick_seconds;
            remainder_y += velocity.y * tick_seconds;
            const LONG dx = (LONG)remainder_x;
            const LONG dy = (LONG)remainder_y;
            remainder_x -= dx;
            remainder_y -= dy;

            if (dx == 0 && dy == 0)
            {
              continue;
            }

            ::INPUT mouse_move{ .type = INPUT_MOUSE };
            mouse_move.mi.dwFlags = MOUSEEVENTF_MOVE;
            mouse_move.mi.dx = dx;
            mouse_move.mi.dy = dy;

            ::SendInput(1, &mouse_move, sizeof(INPUT));
          }
        });
      }
    }

    ~input_injector()
    {
      win32::remove_window_subclass(target_window, sub_class_callback, (UINT_PTR)this);

      std::array<RAWINPUTDEVICE, 4> descriptors{ {
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_GAMEPAD,
          .dwFlags = RIDEV_REMOVE },
        {
          .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_JOYSTICK,
          .dwFlags = RIDEV_REMOVE,
        },
        {
          .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_MOUSE,
          .dwFlags = RIDEV_REMOVE,
        },
        {
          .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_KEYBOARD,
          .dwFlags = RIDEV_REMOVE,
        },
      } };

      if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        auto error = GetLastError();
        DebugBreak();
        // registration failed. Call GetLastError for the cause of the error.
      }

      if (mouse_stop_event)
      {
        ::SetEvent(mouse_stop_event);
      }

      if (mouse_thread.joinable())
      {
        mouse_thread.join();
      }

      if (mouse_timer_handle)
      {
        ::CancelWaitableTimer(mouse_timer_handle);
        ::CloseHandle(mouse_timer_handle);
      }

      if (mouse_stop_event)
      {
        ::CloseHandle(mouse_stop_event);
      }

      ::CloseHandle(process_handle);
      ::CloseHandle(thread_handle);
    }

    std::uint16_t normalise(std::int16_t value)
    {
      auto result = (std::uint16_t)32768 + (std::uint16_t)value;

      if (value < 0)
      {
        return std::numeric_limits<std::uint16_t>::max() - result;
      }

      return result;
    }

    auto calculate_deadzone(std::pair<short, short> x_y_pair, int deadzone)
    {
      auto [x, y] = x_y_pair;
      if (std::abs(x) < deadzone)
      {
        x = 0;
      }

      if (std::abs(y) < deadzone)
      {
        y = 0;
      }

      return std::make_pair(x, y);
    }

    LRESULT wm_input(win32::window_ref, win32::input_message message)
    {
      DWORD exit_code = 0;
      if (::GetExitCodeProcess(child_process.hProcess, &exit_code) && exit_code != STILL_ACTIVE)
      {
        if (injector_args.on_process_closed)
        {
          injector_args.on_process_closed();
        }
        return 0;
      }

      RAWINPUTHEADER header{};

      UINT size = sizeof(header);
      if (::GetRawInputData(message.handle, RID_HEADER, &header, &size, sizeof(header)) == (UINT)-1)
      {
        return 0;
      }

      auto device_id = siege::find_device_id(header.hDevice);

      if (!device_id)
      {
        return 0;
      }

      if (header.dwType == RIM_TYPEKEYBOARD)
      {
        // TODO only send keyboard/mouse input if input filtering is enabled
        // RAWINPUT input{};
        // UINT size = sizeof(input);

        // if (::GetRawInputData(message.handle, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) > 0)
        //{
        //   INPUT temp{ .type = INPUT_KEYBOARD };
        //   temp.ki.dwExtraInfo = *device_id;
        //   //    temp.ki.wVk = input.data.keyboard.VKey;
        //   temp.ki.wScan = input.data.keyboard.MakeCode;
        //   temp.ki.dwFlags = KEYEVENTF_SCANCODE;

        //  if (input.data.keyboard.Flags & RI_KEY_BREAK)
        //  {
        //    temp.ki.dwFlags |= KEYEVENTF_KEYUP;
        //  }

        //  if (input.data.keyboard.Flags & RI_KEY_E0)
        //  {
        //    temp.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        //  }

        //  ::SendInput(1, &temp, sizeof(temp));
        // }

        return 0;
      }

      if (header.dwType == RIM_TYPEMOUSE)
      {
        RAWINPUT input{};
        UINT size = sizeof(input);

        if (::GetRawInputData(message.handle, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) > 0)
        {
          //        INPUT temp{ .type = INPUT_MOUSE };
          //          ::SendInput(1, &temp, sizeof(temp));
        }
        return 0;
      }

      simulated_inputs.clear();

      auto info = registered_controllers.find(header.hDevice);

      if (info == registered_controllers.end())
      {
        return 0;
      }

      XINPUT_STATE temp{};

      auto& mappings = injector_args.args.controller_to_send_input_mappings;

      auto new_state = siege::views::get_current_state_for_handle(info->second, message.handle);

      if (new_state.dwPacketNumber == info->second.last_state.dwPacketNumber)
      {
        return 0;
      }

      auto changes = siege::views::get_changes(info->second.last_state, new_state, std::span{ info->second.buffer });

      if (changes.size() == 0)
      {
        return 0;
      }

      std::unordered_map<vkey, input_state_change> x_mouse_changes;
      std::unordered_map<vkey, input_state_change> y_mouse_changes;

      for (auto mapped_vkey : changes)
      {
        for (auto& mapping : mappings)
        {
          if (mapping.from_vkey == vkey::none || mapping.to_vkey == vkey::none)
          {
            break;
          }

          if (mapping.from_vkey != mapped_vkey.vkey)
          {
            continue;
          }

          if (!(mapping.to_context == hardware_context::global || mapping.to_context == hardware_context::keyboard || mapping.to_context == hardware_context::mouse))
          {
            continue;
          }

          constexpr static auto dead_zone = std::numeric_limits<std::uint16_t>::max() / 4;

          if (mapping.to_context == hardware_context::mouse && !injector_args.simulate_mouse)
          {
            continue;
          }

          if (is_vkey_for_mouse(mapping.to_vkey) && mapping.to_context == hardware_context::global && !injector_args.simulate_mouse)
          {
            continue;
          }

          if (mapping.to_context == hardware_context::keyboard && !injector_args.simulate_keyboard)
          {
            continue;
          }

          if (is_vkey_for_keyboard(mapping.to_vkey) && mapping.to_context == hardware_context::global && !injector_args.simulate_keyboard)
          {
            continue;
          }

          if (mapping.to_context == hardware_context::mouse && (mapping.to_vkey == vkey::left || mapping.to_vkey == vkey::right))
          {
            x_mouse_changes[mapping.to_vkey] = mapped_vkey;
            continue;
          }

          if (mapping.to_context == hardware_context::mouse && (mapping.to_vkey == vkey::up || mapping.to_vkey == vkey::down))
          {
            y_mouse_changes[mapping.to_vkey] = mapped_vkey;
            continue;
          }

          if (mapped_vkey.new_value > dead_zone && mapped_vkey.old_value < dead_zone)
          {
            simulated_inputs.emplace_back(vk_to_input(mapping.to_vkey, mapping.to_context, *device_id, input_state::down, mapped_vkey.new_value));
          }
          else if (mapped_vkey.new_value < dead_zone && mapped_vkey.old_value > dead_zone)
          {
            simulated_inputs.emplace_back(vk_to_input(mapping.to_vkey, mapping.to_context, *device_id, input_state::up, mapped_vkey.new_value));
          }
        }
      }

      if (!(x_mouse_changes.empty() && y_mouse_changes.empty()))
      {
        constexpr static auto dead_zone = std::numeric_limits<std::uint16_t>::max() / 8;

        const auto to_velocity = [max_speed = injector_args.max_turn_speed, exponent = injector_args.response_curve_exponent](std::uint16_t magnitude, bool negative) {
          const double fraction = (double)magnitude / std::numeric_limits<std::uint16_t>::max();
          const double shaped = std::pow(fraction, exponent);
          const double speed = shaped * max_speed;
          return (LONG)std::lround(negative ? -speed : speed);
        };

        // seed from current velocity so an axis with no change this packet keeps moving
        POINT velocity = mouse_velocity.load(std::memory_order_relaxed);

        if (!x_mouse_changes.empty())
        {
          bool reset_x = std::all_of(x_mouse_changes.begin(), x_mouse_changes.end(), [](auto& item) {
            return item.second.new_value < dead_zone;
          });

          if (reset_x)
          {
            velocity.x = 0;
          }
          else
          {
            auto max_x = std::max_element(x_mouse_changes.begin(), x_mouse_changes.end(), [](const auto& a, const auto& b) {
              return a.second.new_value < b.second.new_value;
            });

            if (max_x->first == vkey::right)
            {
              velocity.x = to_velocity(max_x->second.new_value, false);
            }
            else if (max_x->first == vkey::left)
            {
              velocity.x = to_velocity(max_x->second.new_value, true);
            }
            else
            {
              assert(false);
              velocity.x = 0;
            }
          }
        }

        if (!y_mouse_changes.empty())
        {
          bool reset_y = std::all_of(y_mouse_changes.begin(), y_mouse_changes.end(), [](auto& item) {
            return item.second.new_value < dead_zone;
          });

          if (reset_y)
          {
            velocity.y = 0;
          }
          else
          {
            auto max_y = std::max_element(y_mouse_changes.begin(), y_mouse_changes.end(), [](const auto& a, const auto& b) {
              return a.second.new_value < b.second.new_value;
            });

            if (max_y->first == vkey::down)
            {
              velocity.y = to_velocity(max_y->second.new_value, false);
            }
            else if (max_y->first == vkey::up)
            {
              velocity.y = to_velocity(max_y->second.new_value, true);
            }
            else
            {
              assert(false);
              velocity.y = 0;
            }
          }
        }

        mouse_velocity.store(velocity, std::memory_order_relaxed);
      }

      if (!simulated_inputs.empty())
      {
        const auto size = ::SendInput(simulated_inputs.size(), simulated_inputs.data(), sizeof(INPUT));

        assert(size == simulated_inputs.size());
      }

      info->second.last_state = new_state;

      return 0;
    }

    LRESULT wm_input_device_change(win32::input_device_change_message message)
    {
      DWORD exit_code = 0;
      if (::GetExitCodeProcess(child_process.hProcess, &exit_code) && exit_code != STILL_ACTIVE)
      {
        if (injector_args.on_process_closed)
        {
          injector_args.on_process_closed();
        }
        return 0;
      }

      if (message.code == GIDC_ARRIVAL)
      {
        auto& state = siege::get_active_input_state();

        auto device_iter = std::find_if(state.devices.begin(), state.devices.end(), [](auto& device) {
          return device.id == 0;
        });

        // Max devices reached
        if (device_iter == state.devices.end())
        {
          return 0;
        }

        RID_DEVICE_INFO device_info{};
        UINT size = sizeof(device_info);

        if (::GetRawInputDeviceInfoW(message.device_handle, RIDI_DEVICEINFO, &device_info, &size) > 0)
        {
          update_device_id(RAWINPUTDEVICELIST{
            .hDevice = message.device_handle,
            .dwType = device_info.dwType,
          });

          if (device_info.dwType == RIM_TYPEMOUSE)
          {
            registered_mice.insert(message.device_handle);
            return 0;
          }
          else if (device_info.dwType == RIM_TYPEKEYBOARD)
          {
            registered_keyboards.insert(message.device_handle);
            return 0;
          }
        }

        auto info = siege::views::controller_info_for_raw_input_device_handle(message.device_handle);

        if (info)
        {
          registered_controllers[message.device_handle] = controller_state{ *info };
        }
      }
      else if (message.code == GIDC_REMOVAL)
      {
        remove_device(message.device_handle);
        registered_mice.erase(message.device_handle);
        registered_keyboards.erase(message.device_handle);
        registered_controllers.erase(message.device_handle);
      }
      return 0;
    }

    static LRESULT __stdcall sub_class_callback(
      HWND hwnd,
      UINT message,
      WPARAM wparam,
      LPARAM lparam,
      UINT_PTR uIdSubclass,
      DWORD_PTR dwRefData)
    {
      if (message == WM_INPUT && uIdSubclass)
      {
        auto* self = (input_injector*)uIdSubclass;
        return self->wm_input(win32::window_ref(hwnd), win32::input_message(wparam, lparam));
      }

      if (message == WM_INPUT_DEVICE_CHANGE && uIdSubclass)
      {
        auto* self = (input_injector*)uIdSubclass;
        return self->wm_input_device_change(win32::input_device_change_message(wparam, lparam));
      }

      if (message == WM_NCDESTROY)
      {
        win32::remove_window_subclass(hwnd, sub_class_callback, uIdSubclass);
      }

      return win32::def_subclass_proc(hwnd, message, wparam, lparam);
    }
  };

  ::INPUT vk_to_input(vkey key, hardware_context context, std::uint32_t device_id, input_state state, int intensity)
  {
    ::INPUT result{};

    if (key == vkey::lbutton)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_LEFTDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (key == vkey::rbutton)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_RIGHTDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (key == vkey::mbutton)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_MIDDLEUP : MOUSEEVENTF_MIDDLEDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (key == vkey::xbutton1)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = XBUTTON1;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_XUP : MOUSEEVENTF_XDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (key == vkey::xbutton2)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = XBUTTON2;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_XUP : MOUSEEVENTF_XDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (key >= vkey::gamepad_a && key <= vkey::gamepad_right_thumbstick_left)
    {
      result.type = INPUT_KEYBOARD;
      result.ki.wVk = static_cast<WORD>(std::to_underlying(key));

      result.ki.dwFlags = state == input_state::up ? KEYEVENTF_KEYUP : 0;
      result.ki.dwExtraInfo = MAKELPARAM(device_id, intensity);

      return result;
    }

    // TODO handle extended codes correctly
    result.type = INPUT_KEYBOARD;
    result.ki.wScan = ::MapVirtualKeyW(std::to_underlying(key), MAPVK_VK_TO_VSC);
    result.ki.dwFlags = state == input_state::up ? KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP : KEYEVENTF_SCANCODE;
    result.ki.dwExtraInfo = device_id;

    if (context == hardware_context::keyboard && (key == vkey::home || key == vkey::insert || key == vkey::del || key == vkey::end || key == vkey::page_down || key == vkey::page_up))
    {
      result.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }

    if (context == hardware_context::keypad && (key == vkey::enter))
    {
      result.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }

    return result;
  }

  std::any bind_to_window(win32::window_ref ref, input_injector_args args)
  {
    return std::make_any<std::shared_ptr<input_injector>>(std::make_shared<input_injector>(std::move(ref), std::move(args)));
  }

}// namespace siege::views
