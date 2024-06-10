#ifndef DARKSTAR_SCRIPT_DISPATCH_HPP
#define DARKSTAR_SCRIPT_DISPATCH_HPP

#include <string_view>
#include <unordered_set>

namespace siege::extension
{
  std::unordered_set<std::string_view> GetGameFunctionNames(std::string_view string_section, const auto& function_name_ranges)
  {
    std::unordered_set<std::string_view> functions;

    for (auto& pair : function_name_ranges)
    {
      auto first_index = string_section.find(pair.first.data(), 0, pair.first.size() + 1);

      if (first_index != std::string_view::npos)
      {
        auto second_index = string_section.find(pair.second.data(), first_index, pair.second.size() + 1);

        if (second_index != std::string_view::npos)
        {
          auto second_ptr = string_section.data() + second_index;
          auto end = second_ptr + std::strlen(second_ptr) + 1;

          for (auto start = string_section.data() + first_index; start != end; start += std::strlen(start) + 1)
          {
            std::string_view temp(start);

            if (temp.size() == 1)
            {
              continue;
            }

            if (!std::all_of(temp.begin(), temp.end(), [](auto c) { return c == '+' || c == '-' || std::isalnum(c) != 0; }))
            {
              break;
            }

            functions.emplace(temp);
          }
        }
      }

      return functions;
    }
  }
}// namespace siege::extension

#endif