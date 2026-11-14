#include <catch2/catch_test_macros.hpp>

import std;
import win32.module;
import siege.platform;
import siege.platform.extension;

namespace fs = std::filesystem;
namespace str = std::ranges;

using siege::platform::game_command_line_args_ex;
using siege::platform::hardware_context;
using siege::platform::input_mapping_ex;
using siege::platform::vkey;

fs::path self_path();
fs::path extension_path(std::string_view name);
std::string read_text(fs::path);
bool has_once(std::string_view, std::string_view);

input_mapping_ex make_binding(vkey key, std::string_view action, hardware_context context = {}, std::uint16_t hardware_index = 0);

game_command_line_args_ex args_with_bindings(std::span<input_mapping_ex> bindings)
{
  return game_command_line_args_ex{
    .input_mapping_struct_size = sizeof(input_mapping_ex),
    .action_bindings_size = bindings.size(),
    .action_bindings = reinterpret_cast<std::byte*>(bindings.data()),
  };
}

TEST_CASE("apply_prelaunch_settings accepts an empty executable path")
{
  siege::platform::game_extension_module extension(extension_path("siege-extension-quake.dll"));

  game_command_line_args_ex args{};
  REQUIRE(extension.apply_prelaunch_settings({}, args) == std::errc::invalid_argument);
}

TEST_CASE("apply_prelaunch_settings writes relevant config files correctly")
{
  siege::platform::game_extension_module extension(extension_path("siege-extension-quake.dll"));

  auto id1 = fs::current_path() / "Id1";
  fs::remove_all(id1);
  fs::create_directories(id1);

  auto _ = std::shared_ptr<void>{
    nullptr, [id1](...) {
      std::error_code last_error;
      fs::remove_all(id1, last_error);
    }
  };

  SECTION("empty config") {
    game_command_line_args_ex args{};
    REQUIRE(extension.apply_prelaunch_settings(self_path(), args) == std::errc{});

    auto config_text = read_text(id1 / "config.cfg");

    REQUIRE_FALSE(config_text.contains("\r\nbind"));
    REQUIRE(has_once(config_text, R"(joyadvaxisx "0")"));
    REQUIRE(has_once(config_text, R"(joyadvaxisv "0")"));
    REQUIRE(has_once(config_text, R"(joyadvaxisu "0")"));
    REQUIRE(has_once(config_text, R"(joyadvaxisr "0")"));
    REQUIRE(has_once(config_text, R"(joyadvaxisy "0")"));

    for (auto i = 1; i <= 4; ++i)
    {
      REQUIRE(has_once(config_text, "JOY" + std::to_string(i)));
    }

    for (auto i = 5; i <= 15; ++i)
    {
      REQUIRE(has_once(config_text, "AUX" + std::to_string(i)));
    }

    REQUIRE(has_once(config_text, "unbind AUX29"));
    REQUIRE(has_once(config_text, R"(alias +melee-attack "impulse 1; +attack")"));
    REQUIRE(has_once(config_text, R"(alias -melee-attack "impulse 12; -attack")"));

    auto autoexec_text = read_text(id1 / "autoexec.cfg");
    REQUIRE(autoexec_text.contains("alias +melee-attack \"impulse 1; +attack\""));
  }

  SECTION("does not duplicate keys from existing files") {
    std::ofstream config_existing(id1 / "config.cfg", std::ios::binary | std::ios::trunc);
    config_existing << "unbind AUX29\r\n";
    config_existing << "unbind JOY1\r\n";
    config_existing << "unbind JOY2\r\n";
    config_existing << "unbind JOY3\r\n";
    config_existing << "unbind JOY4\r\n";
    config_existing << "joyadvaxisx \"0\"\r\n";
    config_existing.flush();
    config_existing.close();

    game_command_line_args_ex args{};
    REQUIRE(extension.apply_prelaunch_settings(self_path(), args) == std::errc{});

    auto config_text = read_text(id1 / "config.cfg");

    REQUIRE(has_once(config_text, R"(joyadvaxisx "0")"));
    for (auto i = 1; i <= 4; ++i)
    {
      REQUIRE(has_once(config_text, "JOY" + std::to_string(i)));
    }

    // TODO autoexec.cfg
  }

  SECTION("generates correct bindings for keyboard and mouse inputs")
  {
    std::array<input_mapping_ex, 5> bindings{ {
      make_binding(vkey{ 'W' }, "+forward"),
      make_binding(vkey{ 'A' }, "+moveleft"),
      make_binding(vkey{ 'S' }, "+back"),
      make_binding(vkey{ 'D' }, "+moveright"),
      make_binding(vkey::lbutton, "+attack", hardware_context::mouse),
    } };
    auto args = args_with_bindings(bindings);
    REQUIRE(extension.apply_prelaunch_settings(self_path(), args) == std::errc{});

    auto config_text = read_text(id1 / "config.cfg");

    REQUIRE(has_once(config_text, "bind W +forward"));
    REQUIRE(has_once(config_text, "bind A +moveleft"));
    REQUIRE(has_once(config_text, "bind S +back"));
    REQUIRE(has_once(config_text, "bind D +moveright"));
    REQUIRE(has_once(config_text, "bind MOUSE1 +attack"));

    auto joystick_is_not_configured = !config_text.contains("joystick") || has_once(config_text, "joystick \"0\"");
    REQUIRE(joystick_is_not_configured);
  }

  SECTION("generates correct bindings for keyboard and mouse inputs")
  {
    std::array<input_mapping_ex, 5> bindings{ {
      make_binding(vkey{ 'W' }, "+forward"),
      make_binding(vkey{ 'A' }, "+moveleft"),
      make_binding(vkey{ 'S' }, "+back"),
      make_binding(vkey{ 'D' }, "+moveright"),
      make_binding(vkey::lbutton, "+attack", hardware_context::mouse),
    } };
    auto args = args_with_bindings(bindings);
    REQUIRE(extension.apply_prelaunch_settings(self_path(), args) == std::errc{});

    auto config_text = read_text(id1 / "config.cfg");

    REQUIRE(has_once(config_text, "bind W +forward"));
    REQUIRE(has_once(config_text, "bind A +moveleft"));
    REQUIRE(has_once(config_text, "bind S +back"));
    REQUIRE(has_once(config_text, "bind D +moveright"));
    REQUIRE(has_once(config_text, "bind MOUSE1 +attack"));

    auto joystick_is_not_configured = !config_text.contains("joystick") || has_once(config_text, "joystick \"0\"");
    REQUIRE(joystick_is_not_configured);
  }

  // TODO controller tests
  SECTION("generates correct bindings for controller inputs")
  {
    auto ps3 = hardware_context::controller_playstation_3;
    std::array<input_mapping_ex, 12> bindings{ {
      make_binding(vkey::gamepad_right_trigger, "+attack", ps3, 5),
      make_binding(vkey::gamepad_a, "+moveup", ps3, 0),
      make_binding(vkey::gamepad_b, "+movedown", ps3, 1),
      make_binding(vkey::gamepad_right_thumbstick_button, "+melee-attack", ps3, 9),
      make_binding(vkey::gamepad_left_thumbstick_up, "+forward", ps3, 1),
      make_binding(vkey::gamepad_left_thumbstick_down, "+back", ps3, 1),
      make_binding(vkey::gamepad_left_thumbstick_left, "+moveleft", ps3, 0),
      make_binding(vkey::gamepad_left_thumbstick_right, "+moveright", ps3, 0),
      make_binding(vkey::gamepad_right_thumbstick_up, "+lookup", ps3, 2),
      make_binding(vkey::gamepad_right_thumbstick_down, "+lookdown", ps3, 2),
      make_binding(vkey::gamepad_right_thumbstick_left, "+left", ps3, 3),
      make_binding(vkey::gamepad_right_thumbstick_right, "+right", ps3, 3),
    } };
    auto args = args_with_bindings(bindings);
    REQUIRE(extension.apply_prelaunch_settings(self_path(), args) == std::errc{});

    auto config_text = read_text(id1 / "config.cfg");

    REQUIRE(has_once(config_text, "bind JOY1 +moveup"));
    REQUIRE(has_once(config_text, "bind JOY2 +movedown"));
    REQUIRE(has_once(config_text, "bind AUX6 +attack"));
    REQUIRE(has_once(config_text, "bind AUX10 +melee-attack"));
    REQUIRE(has_once(config_text, "joyadvaxisx \"3\""));
    REQUIRE(has_once(config_text, "joyadvaxisy \"1\""));
    REQUIRE(has_once(config_text, "joyadvaxisz \"2\""));
    REQUIRE(has_once(config_text, "joyadvaxisr \"4\""));
    REQUIRE(has_once(config_text, "joyadvaxisu \"0\""));
  }

}

fs::path self_path()
{
  return std::filesystem::path(win32::module_ref::current_application().GetModuleFileName());
}

fs::path extension_path(std::string_view name)
{
  return self_path().parent_path() / name;
}

std::string read_text(fs::path path)
{
  std::ifstream file(path, std::ios::binary);
  return std::string(std::istreambuf_iterator<char>(file), {});
}

bool has_once(std::string_view text, std::string_view value)
{
  auto index = text.find(value);

  if (index == std::string_view::npos)
  {
    return false;
  }

  return index == text.rfind(value);
}

input_mapping_ex make_binding(vkey key, std::string_view action, hardware_context context, std::uint16_t hardware_index)
{
  input_mapping_ex binding{
    .vkey = key,
    .hardware_index = hardware_index,
    .context = context,
  };

  std::copy_n(action.data(), std::min(action.size(), binding.action_name.size() - 1), binding.action_name.data());
  return binding;
}
