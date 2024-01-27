#include <utility>
#include <array>
#include <algorithm>
#include <cassert>
#include <imgui.h>
#include "win32_controls.hpp"

struct imgui_window
{
    constexpr static auto key_mapping = std::array{
        std::make_pair(VK_LEFT, ImGuiKey_LeftArrow),
        std::make_pair(VK_RIGHT, ImGuiKey_RightArrow),
        std::make_pair(VK_UP, ImGuiKey_UpArrow),
        std::make_pair(VK_DOWN, ImGuiKey_DownArrow),
        std::make_pair(VK_LSHIFT, ImGuiKey_LeftShift),
        std::make_pair(VK_LCONTROL, ImGuiKey_LeftCtrl),
        std::make_pair(VK_LMENU, ImGuiKey_LeftAlt),
        std::make_pair(VK_LWIN, ImGuiKey_LeftSuper),
        std::make_pair(VK_RSHIFT, ImGuiKey_RightShift),
        std::make_pair(VK_RCONTROL, ImGuiKey_RightCtrl),
        std::make_pair(VK_RMENU, ImGuiKey_RightAlt),
        std::make_pair(VK_RWIN, ImGuiKey_RightSuper),
        std::make_pair(VK_TAB, ImGuiKey_Tab),
        std::make_pair(VK_DELETE, ImGuiKey_Delete),
        std::make_pair(VK_BACK, ImGuiKey_Backspace),
        std::make_pair(VK_SPACE, ImGuiKey_Space),
        std::make_pair(VK_RETURN, ImGuiKey_Enter),
        std::make_pair(VK_ESCAPE, ImGuiKey_Escape),
        std::make_pair(VK_CAPITAL, ImGuiKey_CapsLock),
        std::make_pair(VK_SCROLL, ImGuiKey_ScrollLock),
        std::make_pair(VK_SNAPSHOT, ImGuiKey_PrintScreen)
        };

    constexpr static auto numpad_mapping = std::array{
        std::make_pair(VK_NUMPAD0, ImGuiKey_Keypad0),
        std::make_pair(VK_NUMPAD9, ImGuiKey_Keypad9),
    };

    constexpr static auto number_mapping = std::array{
        std::make_pair(int('0'), ImGuiKey_0),
        std::make_pair(int('9'), ImGuiKey_9),
    };

    constexpr static auto letter_mapping =  std::array{
        std::make_pair(int('A'), ImGuiKey_A),
        std::make_pair(int('Z'), ImGuiKey_Z),
    };

    constexpr static auto function_mapping = std::array{
        std::make_pair(VK_F1, ImGuiKey_F1),
        std::make_pair(VK_F12, ImGuiKey_F12),
    };

    constexpr static auto mapping_pairs = std::array<decltype(number_mapping), 4>{ { numpad_mapping, number_mapping, letter_mapping, function_mapping } };

    constexpr static ImGuiKey map_key(int value)
    {
        auto result = std::find_if(key_mapping.begin(), key_mapping.end(), [&](const auto& pair) { return pair.first == value; });

        if (result != key_mapping.end())
        {
            return result->second;
        }
        
        for (auto& pair_mapping : mapping_pairs)
        {
            auto& first = pair_mapping[0];
            auto& second = pair_mapping[1];

            if (value >= first.first && value <= second.first)
            {
                auto diff = second.first - first.first;

                return ImGuiKey(int(first.second) + diff);
            }
        }

        return ImGuiKey::ImGuiKey_None;
    }

    void init()
    {
        assert(imgui_window::map_key(VK_SPACE) == ImGuiKey::ImGuiKey_Space);
        assert(map_key(VK_UP) == ImGuiKey::ImGuiKey_UpArrow);
        assert(map_key('0') == ImGuiKey::ImGuiKey_0);
        assert(map_key('1') == ImGuiKey::ImGuiKey_1);
        assert(map_key('5') == ImGuiKey::ImGuiKey_5);
        assert(map_key('8') == ImGuiKey::ImGuiKey_8);
        assert(map_key('9') == ImGuiKey::ImGuiKey_9);

        assert(map_key('A') == ImGuiKey::ImGuiKey_A);
        assert(map_key('B') == ImGuiKey::ImGuiKey_B);
        assert(map_key('M') == ImGuiKey::ImGuiKey_M);
        assert(map_key('X') == ImGuiKey::ImGuiKey_X);
        assert(map_key('Z') == ImGuiKey::ImGuiKey_Z);

        assert(map_key(VK_NUMPAD0) == ImGuiKey::ImGuiKey_Keypad0);
        assert(map_key(VK_NUMPAD5) == ImGuiKey::ImGuiKey_Keypad5);
        assert(map_key(VK_NUMPAD9) == ImGuiKey::ImGuiKey_Keypad9);

        ImGuiIO& io = ImGui::GetIO();
        io.BackendPlatformName = "ImguiSiegeWin32";
    }

    std::optional<win32::lresult_t> on_keyboard_key_up(win32::keyboard_key_up_message& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(map_key(event.virtual_key_code), false);

        return 0;
    }

    std::optional<win32::lresult_t> on_keyboard_key_down(win32::keyboard_key_down_message& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(map_key(event.virtual_key_code), true);

        return 0;
    }

    std::optional<win32::lresult_t> on_keyboard_key_down(win32::keyboard_char_message& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharacterUTF16(event.translated_char);

        return 0;
    }

    std::optional<win32::lresult_t> on_mouse_move(win32::mouse_move_message& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(event.x, event.y);

        return 0;
    }

    std::optional<win32::lresult_t> on_mouse_leave(win32::mouse_leave_message& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        return 0;
    }

    std::optional<win32::lresult_t> on_mouse_button_down(win32::mouse_button_down_message& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(int(event.source), true);
        return 0;
    }

    std::optional<win32::lresult_t> on_mouse_button_up(win32::mouse_button_up_message& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(int(event.source), false);
        return 0;
    }

    std::optional<win32::lresult_t> on_mouse_button_double_click(win32::mouse_button_double_click_message& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(int(event.source), true);
        return 0;
    }

    std::optional<win32::lresult_t> on_mouse_hover(win32::mouse_hover_message& event)
    {
        return 0;
    }

};