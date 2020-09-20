#ifndef DARKSTARDTSCONVERTER_SFML_KEYS_HPP
#define DARKSTARDTSCONVERTER_SFML_KEYS_HPP

#include <string_view>
#include <array>
#include <algorithm>
#include <utility>
#include <SFML/Window/Event.hpp>

namespace config
{
  using namespace std::literals;

  constexpr static auto letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  constexpr static std::array numpad_names = { "Numpad0"sv, "Numpad1"sv, "Numpad2"sv, "Numpad3"sv, "Numpad4"sv, "Numpad5"sv, "Numpad6"sv, "Numpad7"sv, "Numpad8"sv, "Numpad9"sv };

  constexpr auto get_numpad_key_names()
  {
    std::array<std::pair<std::string_view, sf::Keyboard::Key>, 10> keys{};

    int name_index = 0;
    for (int i = sf::Keyboard::Key::Numpad0; i < sf::Keyboard::Key::Numpad0 + 10; ++i)
    {
      keys[name_index] = std::make_pair(numpad_names[name_index], static_cast<sf::Keyboard::Key>(i));
      ++name_index;
    }

    return keys;
  }

  constexpr auto get_letter_key_names()
  {
    std::array<std::pair<std::string_view, sf::Keyboard::Key>, 26> keys{};

    for (int i = 0; i < 26; ++i)
    {
      keys[i] = std::make_pair(std::string_view(&letters[i], 1), static_cast<sf::Keyboard::Key>(i));
    }

    return keys;
  }

  constexpr static std::array special_key_names = {
    std::make_pair("Up"sv, sf::Keyboard::Up),
    std::make_pair("Down"sv, sf::Keyboard::Down),
    std::make_pair("Left"sv, sf::Keyboard::Left),
    std::make_pair("Right"sv, sf::Keyboard::Right),
    std::make_pair("Insert"sv, sf::Keyboard::Insert),
    std::make_pair("Delete"sv, sf::Keyboard::Delete),
    std::make_pair("Home"sv, sf::Keyboard::Home),
    std::make_pair("End"sv, sf::Keyboard::End),
    std::make_pair("PageUp"sv, sf::Keyboard::PageUp),
    std::make_pair("PageDown"sv, sf::Keyboard::PageDown),
    std::make_pair("Add"sv, sf::Keyboard::Add),
    std::make_pair("Subtract"sv, sf::Keyboard::Subtract),
    std::make_pair("Multiply"sv, sf::Keyboard::Multiply),
    std::make_pair("Divide"sv, sf::Keyboard::Divide),
    std::make_pair("Tab"sv, sf::Keyboard::Tab),
    std::make_pair("Space"sv, sf::Keyboard::Space),
    std::make_pair("Enter"sv, sf::Keyboard::Enter),
    std::make_pair("Backspace"sv, sf::Keyboard::Backspace),
    std::make_pair("Period"sv, sf::Keyboard::Period),
    std::make_pair("Comma"sv, sf::Keyboard::Comma)
  };


  sf::Keyboard::Key get_key_for_name(const std::string_view name)
  {
    static auto numpad_key_names = get_numpad_key_names();
    static auto letter_names = get_letter_key_names();

    auto numpad_key = find_if(numpad_key_names.begin(), numpad_key_names.end(),
      [&](const auto& value) { return value.first == name;});

    if (numpad_key != numpad_key_names.end())
    {
      return numpad_key->second;
    }

    auto special_key = find_if(special_key_names.begin(), special_key_names.end(),
                              [&](const auto& value) { return value.first == name;});

    if (special_key != special_key_names.end())
    {
      return special_key->second;
    }

    auto letter_key = find_if(letter_names.begin(), letter_names.end(),
                               [&](const auto& value) { return value.first == name;});

    if (letter_key != letter_names.end())
    {
      return letter_key->second;
    }

    return sf::Keyboard::Key::Unknown;
  }

}




#endif//DARKSTARDTSCONVERTER_SFML_KEYS_HPP
