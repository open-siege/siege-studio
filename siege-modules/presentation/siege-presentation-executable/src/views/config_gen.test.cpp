#include <catch2/catch_test_macros.hpp>

import std;

import siege.platform;
import siege.platform.extension;

import exe.shared;

using namespace siege::views;
using namespace siege::platform;
using namespace std::literals;

namespace
{
  game_action make_action(std::string_view name, decltype(game_action::type) type = game_action::digital);
  controller_info make_controller(hardware_context context,
    std::function<std::optional<hardware_index>(vkey, controller_info::button_preference)> resolve);
  controller_info make_simple_pad(hardware_context context = hardware_context::controller_xbox);
  bool has_fallback(const config_intent& intent, vkey from, hardware_context from_context, vkey to, hardware_context to_context);
  bool has_action_binding(const config_intent& intent, vkey key, std::uint16_t hardware_index, std::string_view action_name);
  bool has_controller_action_binding(const config_intent& intent);

  // Minimal mock: only the game_extension surface config_gen needs.
  struct mock_game_extension final : game_extension
  {
    std::vector<game_action> actions_storage;
    std::vector<const wchar_t*> backends_storage;
    std::optional<game_command_line_caps> caps_storage;
    std::function<bool(const hardware_context_caps&, const input_mapping_ex&)> mapping_valid =
      [](const hardware_context_caps&, const input_mapping_ex&) { return true; };

    std::span<const game_action> game_actions() const override
    {
      return actions_storage;
    }

    std::span<const wchar_t* const> controller_input_backends() const override
    {
      return backends_storage;
    }

    std::span<const wchar_t* const> network_backends() const override
    {
      return {};
    }

    const std::optional<game_command_line_caps>& caps() const override
    {
      return caps_storage;
    }

    bool is_input_mapping_valid(const hardware_context_caps& caps, const input_mapping_ex& mapping) const override
    {
      return mapping_valid(caps, mapping);
    }
  };
}

TEST_CASE("without an extension, manual controller fallbacks drive input simulation", "[config_gen][no-extension]")
{
  user_intent user{
    .controller_fallbacks = {
      {
        .from_vkey = vkey::gamepad_a,
        .from_context = hardware_context::controller_xbox,
        .to_vkey = vkey::space,
        .to_context = hardware_context::keyboard,
      },
      {
        .from_vkey = vkey::gamepad_b,
        .from_context = hardware_context::controller_xbox,
        .to_vkey = vkey::escape,
        .to_context = hardware_context::keyboard,
      },
    },
  };

  auto intent = generate_config(user);

  REQUIRE(intent.should_disable_controller);
  REQUIRE(intent.action_bindings.empty());
  REQUIRE(intent.controller_to_send_input_mappings.size() == 2);
  REQUIRE(has_fallback(intent, vkey::gamepad_a, hardware_context::controller_xbox, vkey::space, hardware_context::keyboard));
  REQUIRE(has_fallback(intent, vkey::gamepad_b, hardware_context::controller_xbox, vkey::escape, hardware_context::keyboard));
}

TEST_CASE("extension with no actions also uses manual controller fallbacks", "[config_gen][extension]")
{
  user_intent user{
    .controller_fallbacks = {
      {
        .from_vkey = vkey::gamepad_a,
        .from_context = hardware_context::controller_xbox,
        .to_vkey = vkey::enter,
        .to_context = hardware_context::keyboard,
      },
    },
  };

  mock_game_extension extension{};

  auto intent = generate_config(user, extension);

  REQUIRE(intent.should_disable_controller);
  REQUIRE(intent.action_bindings.empty());
  REQUIRE(intent.controller_to_send_input_mappings.size() == 1);
  REQUIRE(has_fallback(intent, vkey::gamepad_a, hardware_context::controller_xbox, vkey::enter, hardware_context::keyboard));
}

TEST_CASE("extension with actions but no controller backends falls back controller → keyboard by action", "[config_gen][extension]")
{
  mock_game_extension extension{};
  extension.actions_storage = {
    make_action("Jump"),
    make_action("Fire"),
  };

  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::space, .context = hardware_context::keyboard, .action_index = 0 },
      { .vkey = vkey::lbutton, .context = hardware_context::mouse, .action_index = 1 },
    },
    .controller_actions = {
      { .vkey = vkey::gamepad_a, .context = hardware_context::controller_xbox, .action_index = 0 },
      { .vkey = vkey::gamepad_b, .context = hardware_context::controller_xbox, .action_index = 1 },
    },
  };

  auto intent = generate_config(user, extension);

  REQUIRE(intent.should_disable_controller);
  REQUIRE_FALSE(intent.action_bindings.empty());
  REQUIRE(has_fallback(intent, vkey::gamepad_a, {}, vkey::space, hardware_context::keyboard));
  REQUIRE(has_fallback(intent, vkey::gamepad_b, {}, vkey::lbutton, hardware_context::mouse));
}

TEST_CASE("extension with controller support configures bindable pads and keeps the backend enabled", "[config_gen][extension]")
{
  mock_game_extension extension{};
  extension.actions_storage = { make_action("Jump") };
  extension.backends_storage = { L"dinput8" };
  extension.mapping_valid = [](const hardware_context_caps& caps, const input_mapping_ex&) {
    return caps.hardware_index == 0;
  };

  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::space, .context = hardware_context::keyboard, .action_index = 0 },
    },
    .controller_actions = {
      { .vkey = vkey::gamepad_a, .context = hardware_context::controller_xbox, .action_index = 0 },
    },
    .get_connected_controllers = [] {
      return std::vector{ make_simple_pad() };
    },
  };

  auto intent = generate_config(user, extension);

  REQUIRE_FALSE(intent.should_disable_controller);
  REQUIRE(has_action_binding(intent, vkey::gamepad_a, 0, "Jump"));
  REQUIRE(intent.controller_to_send_input_mappings.empty());
}

TEST_CASE("when mappings cannot be unbound (hardcoded axes), disable the backend and simulate everything", "[config_gen][extension]")
{
  // Games like Uprising hard-code axes: both binding and unbinding that hardware fail
  // (is_bindable and is_unbindable are false). can_bind requires every mapping to be
  // bindable OR unbindable; one hardcoded axis forces a full backend disable + simulate.
  mock_game_extension extension{};
  extension.actions_storage = {
    make_action("Steer", game_action::analog),
    make_action("Fire"),
  };
  extension.backends_storage = { L"dinput8" };
  extension.mapping_valid = [](const hardware_context_caps&, const input_mapping_ex& mapping) {
    if (mapping.hardware_input_type == controller_input_type::axis)
    {
      return false;
    }
    return true;
  };

  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::left, .context = hardware_context::keyboard, .action_index = 0 },
      { .vkey = vkey::lbutton, .context = hardware_context::mouse, .action_index = 1 },
    },
    .controller_actions = {
      { .vkey = vkey::gamepad_left_thumbstick_left, .context = hardware_context::controller_xbox, .action_index = 0 },
      { .vkey = vkey::gamepad_a, .context = hardware_context::controller_xbox, .action_index = 1 },
    },
    .get_connected_controllers = [] {
      return std::vector{ make_simple_pad() };
    },
  };

  auto intent = generate_config(user, extension);

  REQUIRE(intent.should_disable_controller);
  REQUIRE_FALSE(has_controller_action_binding(intent));
  REQUIRE(has_fallback(intent, vkey::gamepad_left_thumbstick_left, {}, vkey::left, hardware_context::keyboard));
  REQUIRE(has_fallback(intent, vkey::gamepad_a, {}, vkey::lbutton, hardware_context::mouse));
}

TEST_CASE("when a binding is rejected but the pad slot is supported, that action falls back to simulation", "[config_gen][extension]")
{
  mock_game_extension extension{};
  extension.actions_storage = {
    make_action("Jump"),
    make_action("Fire"),
  };
  extension.backends_storage = { L"dinput8" };
  extension.mapping_valid = [](const hardware_context_caps& caps, const input_mapping_ex& mapping) {
    if (caps.hardware_index != 0)
    {
      return false;
    }
    if (mapping.action_name[0] == '\0')
    {
      return true;
    }
    return std::string_view(mapping.action_name.data()) == "Jump";
  };

  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::space, .context = hardware_context::keyboard, .action_index = 0 },
      { .vkey = vkey::lbutton, .context = hardware_context::mouse, .action_index = 1 },
    },
    .controller_actions = {
      { .vkey = vkey::gamepad_a, .context = hardware_context::controller_xbox, .action_index = 0 },
      { .vkey = vkey::gamepad_b, .context = hardware_context::controller_xbox, .action_index = 1 },
    },
    .get_connected_controllers = [] {
      return std::vector{ make_simple_pad() };
    },
  };

  auto intent = generate_config(user, extension);

  REQUIRE_FALSE(intent.should_disable_controller);
  REQUIRE(has_action_binding(intent, vkey::gamepad_a, 0, "Jump"));
  REQUIRE_FALSE(has_action_binding(intent, vkey::gamepad_b, 1, "Fire"));
  REQUIRE(has_fallback(intent, vkey::gamepad_b, hardware_context::controller_xbox, vkey::lbutton, hardware_context::mouse));
  REQUIRE_FALSE(has_fallback(intent, vkey::gamepad_a, hardware_context::controller_xbox, vkey::space, hardware_context::keyboard));
}

TEST_CASE("when the game accepts every controller slot, bind mappings for each pad", "[config_gen][extension][multi-controller]")
{
  mock_game_extension extension{};
  extension.actions_storage = {
    make_action("Jump"),
    make_action("Fire"),
  };
  extension.backends_storage = { L"dinput8" };
  extension.mapping_valid = [](const hardware_context_caps& caps, const input_mapping_ex& mapping) {
    if (mapping.action_name[0] == '\0')
    {
      return true;
    }
    return caps.hardware_index < 2;
  };

  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::space, .context = hardware_context::keyboard, .action_index = 0 },
      { .vkey = vkey::lbutton, .context = hardware_context::mouse, .action_index = 1 },
    },
    .controller_actions = {
      { .vkey = vkey::gamepad_a, .context = hardware_context::controller_xbox, .action_index = 0 },
      { .vkey = vkey::gamepad_b, .context = hardware_context::controller_xbox, .action_index = 1 },
    },
    .get_connected_controllers = [] {
      return std::vector{ make_simple_pad(), make_simple_pad() };
    },
  };

  auto intent = generate_config(user, extension);

  REQUIRE_FALSE(intent.should_disable_controller);
  REQUIRE(intent.controller_to_send_input_mappings.empty());
  REQUIRE(std::ranges::count_if(intent.action_bindings, [](const auto& binding) {
            return binding.vkey == vkey::gamepad_a || binding.vkey == vkey::gamepad_b;
          })
          == 4);
}

TEST_CASE("unresolved hardware mappings fall back without disabling an otherwise supported pad", "[config_gen][extension]")
{
  mock_game_extension extension{};
  extension.actions_storage = {
    make_action("Jump"),
    make_action("Fire"),
  };
  extension.backends_storage = { L"dinput8" };
  extension.mapping_valid = [](const hardware_context_caps& caps, const input_mapping_ex&) {
    return caps.hardware_index == 0;
  };

  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::space, .context = hardware_context::keyboard, .action_index = 0 },
      { .vkey = vkey::lbutton, .context = hardware_context::mouse, .action_index = 1 },
    },
    .controller_actions = {
      { .vkey = vkey::gamepad_a, .context = hardware_context::controller_xbox, .action_index = 0 },
      // Left trigger is not mapped by make_simple_pad → unresolved hardware index.
      { .vkey = vkey::gamepad_left_trigger, .context = hardware_context::controller_xbox, .action_index = 1 },
    },
    .get_connected_controllers = [] {
      return std::vector{ make_simple_pad() };
    },
  };

  auto intent = generate_config(user, extension);

  REQUIRE_FALSE(intent.should_disable_controller);
  REQUIRE(has_action_binding(intent, vkey::gamepad_a, 0, "Jump"));
  REQUIRE(has_fallback(intent, vkey::gamepad_left_trigger, {}, vkey::lbutton, hardware_context::mouse));
  REQUIRE_FALSE(has_fallback(intent, vkey::gamepad_a, {}, vkey::space, hardware_context::keyboard));
}

TEST_CASE("winmm backends remap hardware indices; non-winmm keeps the HID index", "[config_gen][extension][hardware-index]")
{
  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::right, .context = hardware_context::mouse, .action_index = 0 },
    },
    .controller_actions = {
      { .vkey = vkey::gamepad_left_thumbstick_right, .context = hardware_context::controller_xbox, .action_index = 0 },
    },
    .get_connected_controllers = [] {
      return std::vector{ make_simple_pad() };
    },
  };

  SECTION("dinput keeps the HID axis index")
  {
    mock_game_extension extension{};
    extension.actions_storage = { make_action("LookX", game_action::analog) };
    extension.backends_storage = { L"dinput8" };
    extension.mapping_valid = [](const hardware_context_caps& caps, const input_mapping_ex&) {
      return caps.hardware_index == 0;
    };

    auto intent = generate_config(user, extension);

    REQUIRE_FALSE(intent.should_disable_controller);
    REQUIRE(has_action_binding(intent, vkey::gamepad_left_thumbstick_right, 0, "LookX"));
  }

  SECTION("winmm remaps through map_hid_to_winmm")
  {
    mock_game_extension extension{};
    extension.actions_storage = { make_action("LookX", game_action::analog) };
    extension.backends_storage = { L"winmm" };
    extension.mapping_valid = [](const hardware_context_caps& caps, const input_mapping_ex&) {
      return caps.hardware_index == 0;
    };

    auto intent = generate_config(user, extension);
    auto expected = map_hid_to_winmm(
      hardware_index{ .type = input_type::axis, .index = 0 },
      mapped_axes(make_simple_pad()));

    REQUIRE_FALSE(intent.should_disable_controller);
    REQUIRE(has_action_binding(intent, vkey::gamepad_left_thumbstick_right, expected.index, "LookX"));
  }
}

TEST_CASE("no connected controllers with a controller-capable extension falls back to simulation", "[config_gen][extension]")
{
  mock_game_extension extension{};
  extension.actions_storage = { make_action("Jump") };
  extension.backends_storage = { L"dinput8" };

  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::space, .context = hardware_context::keyboard, .action_index = 0 },
    },
    .controller_actions = {
      { .vkey = vkey::gamepad_a, .context = hardware_context::controller_xbox, .action_index = 0 },
    },
    .get_connected_controllers = [] { return std::vector<controller_info>{}; },
  };

  auto intent = generate_config(user, extension);

  REQUIRE(intent.should_disable_controller);
  REQUIRE(has_fallback(intent, vkey::gamepad_a, {}, vkey::space, hardware_context::keyboard));
}

TEST_CASE("keyboard and mouse only leaves the controller backend enabled with no simulation", "[config_gen][extension]")
{
  mock_game_extension extension{};
  extension.actions_storage = { make_action("Jump") };
  extension.backends_storage = { L"dinput8" };
  extension.mapping_valid = [](const hardware_context_caps& caps, const input_mapping_ex&) {
    return caps.hardware_index == 0;
  };

  user_intent user{
    .keyboard_mouse_actions = {
      { .vkey = vkey::space, .context = hardware_context::keyboard, .action_index = 0 },
    },
    .get_connected_controllers = [] {
      return std::vector{ make_simple_pad() };
    },
  };

  auto intent = generate_config(user, extension);

  REQUIRE_FALSE(intent.should_disable_controller);
  REQUIRE(intent.controller_to_send_input_mappings.empty());
  REQUIRE_FALSE(has_controller_action_binding(intent));
}

namespace
{
  game_action make_action(std::string_view name, decltype(game_action::type) type)
  {
    game_action action{};
    action.type = type;
    action.action_name = {};
    std::copy_n(name.data(), std::min(name.size(), action.action_name.size() - 1), action.action_name.begin());
    return action;
  }

  controller_info make_controller(hardware_context context,
    std::function<std::optional<hardware_index>(vkey, controller_info::button_preference)> resolve)
  {
    controller_info info{};
    info.detected_context = context;
    info.get_hardware_index = std::move(resolve);
    return info;
  }

  // Xbox-like pad: A/B are buttons 0/1, left stick X is axis 0.
  controller_info make_simple_pad(hardware_context context)
  {
    return make_controller(context, [](vkey key, controller_info::button_preference preference) -> std::optional<hardware_index> {
      if (key == vkey::gamepad_a)
      {
        return hardware_index{ .type = input_type::button, .index = 0 };
      }
      if (key == vkey::gamepad_b)
      {
        return hardware_index{ .type = input_type::button, .index = 1 };
      }
      if (key == vkey::gamepad_left_thumbstick_left || key == vkey::gamepad_left_thumbstick_right)
      {
        if (preference == controller_info::prefer_axis)
        {
          return hardware_index{ .type = input_type::axis, .index = 0 };
        }
      }
      return std::nullopt;
    });
  }

  bool has_fallback(const config_intent& intent, vkey from, hardware_context from_context, vkey to, hardware_context to_context)
  {
    return std::ranges::any_of(intent.controller_to_send_input_mappings, [&](const auto& mapping) {
      return mapping.from_vkey == from && mapping.from_context == from_context
             && mapping.to_vkey == to && mapping.to_context == to_context;
    });
  }

  bool has_action_binding(const config_intent& intent, vkey key, std::uint16_t hardware_index, std::string_view action_name)
  {
    return std::ranges::any_of(intent.action_bindings, [&](const auto& binding) {
      return binding.vkey == key && binding.hardware_index == hardware_index
             && std::string_view(binding.action_name.data()) == action_name;
    });
  }

  bool has_controller_action_binding(const config_intent& intent)
  {
    return std::ranges::any_of(intent.action_bindings, [](const auto& binding) {
      return is_for_controller(binding.context);
    });
  }
}
