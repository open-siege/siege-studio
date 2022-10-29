#ifndef OPEN_SIEGE_VIEW_CONTEXT_HPP
#define OPEN_SIEGE_VIEW_CONTEXT_HPP

#include <functional>
#include <vector>
#include <string_view>
#include "resources/resource_explorer.hpp"

namespace studio::views
{
    struct view_actions
    {
      std::function<std::vector<std::string_view>(std::string_view)> get_extensions_by_category;
      std::function<void(const studio::resources::file_info&)> open_new_tab;
    };

    struct view_context
    {
      studio::resources::file_info file_info;
      const studio::resources::resource_explorer& explorer;
      const view_actions& actions;
    };
}

#endif// OPEN_SIEGE_VIEW_CONTEXT_HPP
