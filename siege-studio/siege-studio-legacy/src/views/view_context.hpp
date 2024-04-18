#ifndef OPEN_SIEGE_VIEW_CONTEXT_HPP
#define OPEN_SIEGE_VIEW_CONTEXT_HPP

#include <functional>
#include <vector>
#include <string_view>
#include "siege/resource/resource_explorer.hpp"

namespace siege::views
{
    struct view_actions
    {
      std::function<std::vector<std::string_view>(std::string_view)> get_extensions_by_category;
      std::function<void(const siege::platform::file_info&)> open_new_tab;
    };

    struct view_context
    {
      siege::platform::file_info file_info;
      const siege::resource::resource_explorer& explorer;
      const view_actions& actions;
    };
}

#endif// OPEN_SIEGE_VIEW_CONTEXT_HPP
