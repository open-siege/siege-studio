#include <map>
#include <string>
#include <algorithm>
#include <istream>
#include <siege/configuration/id_tech.hpp>
#include <siege/platform/stream.hpp>
#include "cfg_controller.hpp"

namespace siege::views
{
  bool cfg_controller::is_cfg(std::istream& stream)
  {
    return siege::configuration::is_ascii_text_config(stream);
  }

  std::size_t cfg_controller::load_config(std::istream& stream) noexcept
  {
    using namespace siege::configuration::id_tech;
    
    auto size = siege::platform::get_stream_size(stream);

    auto result = id_tech_2::load_config(stream, size);

    if (!result)
    {
      return 0;
    }

    text_config.emplace(std::move(*result));

    return 1;
  }

  std::map<std::wstring, std::wstring> cfg_controller::get_key_values()
  {
    std::map<std::wstring, std::wstring> result;


    if (text_config)
    {
      auto keys = text_config->keys();

      for (auto& key : keys)
      {
        auto first = key.at(0);
        auto value = text_config->find(key).at(0);

        std::wstring final_key(first.size(), L'\0');
        std::wstring final_value(value.size(), L'\0');

        std::transform(first.begin(), first.end(), final_key.begin(), [](char value) { return (wchar_t) value; });
        std::transform(value.begin(), value.end(), final_value.begin(), [](char value) { return (wchar_t)value; });

        result.emplace(std::move(final_key), std::move(final_value));
      }

    }

    return result;
  }
}// namespace siege::views