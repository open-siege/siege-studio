module;
#include <cassert>

module exe.shared;

import std;

import siege.platform;
import siege.platform.extension;

namespace stl = std::ranges;
using siege::platform::game_action;
using siege::platform::game_command_line_args;
using siege::platform::hardware_context;
using siege::platform::hardware_context_caps;
using siege::platform::input_mapping_ex;
using siege::platform::vkey;
using controller_to_send_input_mapping = game_command_line_args::controller_to_send_input_mapping;

namespace siege::views
{
  void add_fallback(std::span<const input_action_binding> keyboard_mouse_actions,
    std::vector<controller_to_send_input_mapping>& controller_to_send_input_mappings,
    const input_action_binding& bound_action,
    hardware_context from_context = {});


  config_intent generate_config(const user_intent& user)
  {
    config_intent intent{ .should_disable_controller = true };
    intent.controller_to_send_input_mappings = user.controller_fallbacks;
    return intent;
  }

  config_intent generate_config(const user_intent& user, const siege::platform::game_extension& extension)
  {
    auto actions = extension.game_actions();
    auto backends = extension.controller_input_backends();

    config_intent intent{};
    intent.action_bindings.reserve(actions.size());

    if (actions.empty())
    {
      intent.controller_to_send_input_mappings = user.controller_fallbacks;
      intent.should_disable_controller = true;
      return intent;
    }

    for (auto& action : user.keyboard_mouse_actions)
    {
      if (action.action_index >= actions.size())
      {
        continue;
      }

      auto [caps, binding] = get_full_info_for_binding(action.context, action.vkey, actions[action.action_index]);

      if (extension.is_input_mapping_valid(caps, binding))
      {
        intent.action_bindings.emplace_back(binding);
      }
    }

    if (backends.empty())
    {
      intent.should_disable_controller = true;
      stl::for_each(user.controller_actions, [&](auto& action) {
        add_fallback(user.keyboard_mouse_actions, intent.controller_to_send_input_mappings, action);
      });
      return intent;
    }

    auto controllers = user.get_connected_controllers ? user.get_connected_controllers() : std::vector<controller_info>{};

    if (controllers.empty())
    {
      intent.should_disable_controller = true;
      stl::for_each(user.controller_actions, [&](auto& action) {
        add_fallback(user.keyboard_mouse_actions, intent.controller_to_send_input_mappings, action);
      });
      // TODO disable controller input
      // or trigger controller config refresh
      return intent;
    }

    const bool uses_winmm = stl::any_of(backends, [](std::wstring_view value) {
      return value == L"winmm";
    });

    std::uint32_t controller_index = 0;

    struct bound_mapping
    {
      input_action_binding bound_action;
      input_mapping_ex binding;
      bool is_unbindable = false;
      bool is_bindable = false;

      std::uint32_t controller_index = 0;
      controller_info* controller_info = nullptr;
    };

    std::vector<bound_mapping> bindings{};

    for (auto& controller : controllers)
    {
      auto _ = std::shared_ptr<void>{
        nullptr, [&](...) { controller_index++; }
      };

      if (controller.get_hardware_index == nullptr)
      {
        continue;
      }

      if (!siege::platform::is_for_controller(controller.detected_context))
      {
        continue;
      }

      for (auto& bound_action : user.controller_actions)
      {
        if (bound_action.action_index >= actions.size())
        {
          continue;
        }

        auto preference = actions[bound_action.action_index].type == game_action::analog
                            ? controller_info::prefer_axis
                            : controller_info::prefer_button;
        auto hardware_index = controller.get_hardware_index(bound_action.vkey, preference);

        if (!hardware_index)
        {
          add_fallback(user.keyboard_mouse_actions, intent.controller_to_send_input_mappings, bound_action);
          continue;
        }

        auto& binding = bindings.emplace_back(bound_action);
        binding.binding.vkey = bound_action.vkey;
        binding.binding.context = controller.detected_context;
        binding.binding.action_name = actions[bound_action.action_index].action_name;

        if (uses_winmm)
        {
          auto mapped = map_hid_to_winmm(*hardware_index, mapped_axes(controller));
          binding.binding.hardware_index = mapped.index;
          binding.binding.hardware_input_type = mapped.type;
        }
        else
        {
          binding.binding.hardware_index = hardware_index->index;
          binding.binding.hardware_input_type = hardware_index->type;
        }
        binding.controller_index = controller_index;
        binding.controller_info = &controller;
      }
    }

    for (auto& binding : bindings)
    {
      assert(binding.controller_info != nullptr);

      auto empty_binding = binding.binding;
      empty_binding.action_name.fill('\0');

      auto buttons = mapped_buttons(*binding.controller_info);
      auto axes = mapped_axes(*binding.controller_info);
      hardware_context_caps controller_caps{
        .context = binding.controller_info->detected_context,
        .hardware_index = binding.controller_index,
        .button_count = buttons.empty() ? 0 : *buttons.rbegin() + 1,
        .axis_count = axes.empty() ? 0 : *axes.rbegin() + 1,
        .hat_count = mapped_hat_count(*binding.controller_info),
      };

      binding.is_unbindable = extension.is_input_mapping_valid(controller_caps, empty_binding);
      binding.is_bindable = extension.is_input_mapping_valid(controller_caps, binding.binding);
    }

    const auto can_bind_game_backend = stl::all_of(bindings, [](auto& binding) {
      return binding.is_bindable || binding.is_unbindable;
    });

    if (can_bind_game_backend)
    {
      intent.should_disable_controller = false;
      for (auto& binding : bindings)
      {
        if (binding.is_bindable)
        {
          intent.action_bindings.emplace_back(binding.binding);
        }
        else
        {
          add_fallback(user.keyboard_mouse_actions,
            intent.controller_to_send_input_mappings,
            binding.bound_action,
            binding.controller_info->detected_context);
        }
      }
    }
    else
    {
      intent.should_disable_controller = true;
      stl::for_each(bindings, [&](auto& binding) {
        add_fallback(user.keyboard_mouse_actions, intent.controller_to_send_input_mappings, binding.bound_action);
      });
    }

    return intent;
  }

  user_intent create_user_intent(std::any& state)
  {
    auto bound_actions = get_action_bindings(state).subspan(1);// skip the sentinel

    auto controller_actions = std::vector(bound_actions.begin(), bound_actions.end());
    controller_actions.erase(std::remove_if(controller_actions.begin(), controller_actions.end(), [](auto& item) {
      return !is_vkey_for_controller(item.vkey);
    }),
      controller_actions.end());

    auto keyboard_mouse_actions = std::vector(bound_actions.begin(), bound_actions.end());
    keyboard_mouse_actions.erase(std::remove_if(keyboard_mouse_actions.begin(), keyboard_mouse_actions.end(), [](auto& item) {
      return is_vkey_for_controller(item.vkey);
    }),
      keyboard_mouse_actions.end());

    auto simulation = get_input_simulation_bindings(state).subspan(1);
    std::vector<controller_to_send_input_mapping> controller_fallbacks(simulation.begin(), simulation.end());
    return user_intent{
      .keyboard_mouse_actions = std::move(keyboard_mouse_actions),
      .controller_actions = std::move(controller_actions),
      .controller_fallbacks = std::move(controller_fallbacks),
      .get_connected_controllers = get_connected_controllers
    };
  }

  void add_fallback(std::span<const input_action_binding> keyboard_mouse_actions,
    std::vector<controller_to_send_input_mapping>& controller_to_send_input_mappings,
    const input_action_binding& bound_action,
    hardware_context from_context)
  {
    auto action_iter = stl::find_if(keyboard_mouse_actions, [&](auto& other) {
      return other.action_index == bound_action.action_index
             && (other.vkey != bound_action.vkey && !(other.context == bound_action.context || other.context == from_context));
    });

    if (action_iter == keyboard_mouse_actions.end())
    {
      return;
    }

    if (stl::any_of(controller_to_send_input_mappings, [&](auto& mapping) {
          return mapping.from_context == from_context && mapping.from_vkey == bound_action.vkey
                 && mapping.to_context == action_iter->context && mapping.to_vkey == action_iter->vkey;
        }))
    {
      return;
    }

    auto& mapping = controller_to_send_input_mappings.emplace_back();

    mapping.from_context = from_context;
    mapping.from_vkey = bound_action.vkey;
    mapping.to_context = action_iter->context;
    mapping.to_vkey = action_iter->vkey;
  }
}// namespace siege::views
