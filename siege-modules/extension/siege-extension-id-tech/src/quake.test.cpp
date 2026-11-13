#include <catch2/catch_test_macros.hpp>

import std;
import win32.module;
import siege.platform.extension;

namespace fs = std::filesystem;
namespace str = std::ranges;

fs::path self_path();
fs::path extension_path(std::string_view name);
std::string read_text(fs::path);
bool has_once(std::string_view, std::string_view);

TEST_CASE("apply_prelaunch_settings accepts an empty executable path")
{
  siege::platform::game_extension_module extension(extension_path("siege-extension-quake.dll"));

  siege::platform::game_command_line_args args{};
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
    siege::platform::game_command_line_args args{};
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

    siege::platform::game_command_line_args args{};
    REQUIRE(extension.apply_prelaunch_settings(self_path(), args) == std::errc{});

    auto config_text = read_text(id1 / "config.cfg");

    REQUIRE(has_once(config_text, R"(joyadvaxisx "0")"));
    for (auto i = 1; i <= 4; ++i)
    {
      REQUIRE(has_once(config_text, "JOY" + std::to_string(i)));
    }

    // TODO autoexec.cfg
  }

  using siege::platform::vkey;
  using siege::platform::game_command_line_args;
  using siege::platform::hardware_context;

  SECTION("generates correct bindings for keyboard and mouse inputs")
  {
    game_command_line_args args{
      .action_bindings = std::array<game_command_line_args::input_mapping, 256>{ {
        {.vkey = vkey{'W'},.action_name = "+forward"},
        {.vkey = vkey{'A'},.action_name = "+moveleft"},
        {.vkey = vkey{'S'},.action_name = "+back"},
        {.vkey = vkey{'D'},.action_name = "+moveright"},
        { .vkey = vkey::lbutton, .context = hardware_context::mouse, .action_name = "+attack" },
      } }

    };
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
    game_command_line_args args{
      .action_bindings = std::array<game_command_line_args::input_mapping, 256>{ {
        { .vkey = vkey{ 'W' }, .action_name = "+forward" },
        { .vkey = vkey{ 'A' }, .action_name = "+moveleft" },
        { .vkey = vkey{ 'S' }, .action_name = "+back" },
        { .vkey = vkey{ 'D' }, .action_name = "+moveright" },
        { .vkey = vkey::lbutton, .context = hardware_context::mouse, .action_name = "+attack" },
      } }

    };
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
    game_command_line_args args{
      .action_bindings = std::array<game_command_line_args::input_mapping, 256>{ {
        { .vkey = vkey::gamepad_right_trigger, .hardware_index = 5, .context = hardware_context::controller_playstation_3, .action_name = "+attack" },
        { .vkey = vkey::gamepad_a, .hardware_index = 0, .context = hardware_context::controller_playstation_3, .action_name = "+moveup" },
        { .vkey = vkey::gamepad_b, .hardware_index = 1, .context = hardware_context::controller_playstation_3, .action_name = "+movedown" },
        { .vkey = vkey::gamepad_right_thumbstick_button, .hardware_index = 9, .context = hardware_context::controller_playstation_3, .action_name = "+melee-attack" },
        { .vkey = vkey::gamepad_left_thumbstick_up, .hardware_index = 1, .context = hardware_context::controller_playstation_3, .action_name = "+forward" },
        { .vkey = vkey::gamepad_left_thumbstick_down, .hardware_index = 1, .context = hardware_context::controller_playstation_3, .action_name = "+back" },
        { .vkey = vkey::gamepad_left_thumbstick_left, .hardware_index = 0, .context = hardware_context::controller_playstation_3, .action_name = "+moveleft" },
        { .vkey = vkey::gamepad_left_thumbstick_right, .hardware_index = 0, .context = hardware_context::controller_playstation_3, .action_name = "+moveright" },
        { .vkey = vkey::gamepad_right_thumbstick_up, .hardware_index = 2, .context = hardware_context::controller_playstation_3, .action_name = "+lookup" },
        { .vkey = vkey::gamepad_right_thumbstick_down, .hardware_index = 2, .context = hardware_context::controller_playstation_3, .action_name = "+lookdown" },
        { .vkey = vkey::gamepad_right_thumbstick_left, .hardware_index = 3, .context = hardware_context::controller_playstation_3, .action_name = "+left" },
        { .vkey = vkey::gamepad_right_thumbstick_right, .hardware_index = 3, .context = hardware_context::controller_playstation_3, .action_name = "+right" },
      } }

    };
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
