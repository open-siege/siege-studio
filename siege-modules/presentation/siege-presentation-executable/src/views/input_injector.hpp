#ifndef INPUT_INJECTOR_HPP
#define INPUT_INJECTOR_HPP

#include <siege/platform/extension_module.hpp>
#include <functional>
#include <any>

namespace siege::views
{
  struct input_injector_args
  {
    enum mode
    {
      none,
      bind_input,
      bind_action
    };

    siege::platform::game_command_line_args& args;
    std::wstring script_host;
    mode input_mode = bind_input;
    std::function<HRESULT(const siege::platform::game_command_line_args* game_args, PROCESS_INFORMATION* process_info)> launch_game_with_extension;
    std::function<void()> on_process_closed;
  };

  std::any bind_to_window(win32::window_ref, input_injector_args);
}

#endif// !INPUT_INJECTOR_HPP
