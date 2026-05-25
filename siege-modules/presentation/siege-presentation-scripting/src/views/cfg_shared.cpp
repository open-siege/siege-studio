#include <map>
#include <string>
#include <algorithm>
#include <istream>
#include <memory>
#include <siege/configuration/shared.hpp>
#include <siege/configuration/id_tech.hpp>
#include <siege/platform/stream.hpp>
#include "cfg_shared.hpp"

namespace siege::views
{
  std::span<const siege::fs_string_view> get_cfg_formats() noexcept
  {
    constexpr static auto formats = std::array<siege::fs_string_view, 5>{ { FSL ".cfg", FSL ".ini", FSL ".inf", FSL "txt", FSL ".cs" } };

    return formats;
  }

  bool is_cfg(std::istream& stream) noexcept
  {
    return siege::configuration::is_ascii_text_config(stream);
  }

  std::size_t load_config(std::any& state, std::istream& stream) noexcept
  {
    using namespace siege::configuration::id_tech;

    auto size = siege::platform::get_stream_size(stream);

    auto result = id_tech_2::load_config(stream, size);

    if (!result)
    {
      return 0;
    }

    // TODO make config copyable
    state = std::make_shared<siege::configuration::text_game_config>(std::move(*result));
    return 1;
  }

  auto* self(std::any& state)
  {
    return std::any_cast<std::shared_ptr<siege::configuration::text_game_config>>(&state)->get();
  }

  std::map<std::wstring, std::wstring> get_key_values(std::any& state)
  {
    std::map<std::wstring, std::wstring> result;

    auto* text_config = self(state);
    if (!text_config)
    {
      return result;
    }

    auto keys = text_config->keys();

    for (auto& key : keys)
    {
      auto first = key.at(0);
      auto value = text_config->find(key).at(0);

      std::wstring final_key(first.size(), L'\0');
      std::wstring final_value(value.size(), L'\0');

      std::transform(first.begin(), first.end(), final_key.begin(), [](char value) { return (wchar_t)value; });
      std::transform(value.begin(), value.end(), final_value.begin(), [](char value) { return (wchar_t)value; });

      result.emplace(std::move(final_key), std::move(final_value));
    }

    return result;
  }
}// namespace siege::views