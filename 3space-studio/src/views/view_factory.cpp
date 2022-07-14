#include "view_factory.hpp"
#include "content/dts/darkstar.hpp"

namespace studio::views
{
  void view_factory::add_file_type(stream_validator* checker, view_creator* creator, bool no_fallback)
  {
    if (no_fallback)
    {
      no_fallback_allowed.emplace(checker);
    }

    creators.emplace(checker, creator);
  }

  void view_factory::add_extension_category(std::string_view category, std::vector<std::string_view> new_extensions, bool is_interface_visible)
  {
    auto category_str = std::to_string(extension_categories.size()) + std::string(category);

    if (extension_categories.find(category_str) != extension_categories.end())
    {
      return;
    }

    std::set<std::string_view> temp;

    auto save = [&]() {
      auto existing = extension_categories.emplace(category_str, temp).first;

      if (is_interface_visible)
      {
        visible_categories.emplace(existing->first);
      }
    };

    if (new_extensions.size() == 1 && new_extensions.front() == "ALL")
    {
      auto all_exts = get_extensions();
      std::transform(all_exts.begin(), all_exts.end(), std::inserter(temp, temp.begin()), [](auto& ext) {
        return ext;
      });
      save();
    }
    else
    {
      for (const auto& extension : new_extensions)
      {
        auto existing_ext = validators.find(extension);
        if (existing_ext != validators.end())
        {
          temp.emplace(existing_ext->first);
        }
      }

      if (temp.empty())
      {
        return;
      }

      save();
    }
  }

  void view_factory::add_extension(std::string_view extension, stream_validator* checker)
  {
    validators.emplace(*extensions.emplace(extension).first, checker);
  }

  [[nodiscard]] std::vector<std::string_view> view_factory::get_extensions() const
  {
    return std::vector<std::string_view>(extensions.cbegin(), extensions.cend());
  }

  [[nodiscard]] std::vector<std::string_view> view_factory::get_extensions_by_category(std::string_view category) const
  {
    auto iter = std::find_if(extension_categories.begin(), extension_categories.end(), [&](auto& ext_category) {
      return ext_category.first == category || std::string_view(ext_category.first).substr(1) == category;
    });

    if (iter == extension_categories.end())
    {
      return {};
    }

    std::vector<std::string_view> results;
    results.reserve(iter->second.size());
    results.assign(iter->second.begin(), iter->second.end());

    return results;
  }

  [[nodiscard]] std::vector<std::string_view> view_factory::get_extension_categories(bool only_interface_visible) const
  {
      std::vector<std::string_view> results;

      if (only_interface_visible)
      {
        results.reserve(visible_categories.size());
        std::transform(visible_categories.begin(), visible_categories.end(), std::back_inserter(results), [](auto& category) {
          return category.substr(1);
        });

        return results;
      }

      results.reserve(extension_categories.size());

      std::transform(extension_categories.begin(), extension_categories.end(), std::back_inserter(results), [](auto& category) {
        return std::string_view(category.first).substr(1);
      });

      return results;
  }

  view_actions& view_factory::get_actions()
  {
    return actions;
  }

  studio_view view_factory::create_view(const studio::resources::file_info& file_info, std::basic_istream<std::byte>& stream, const studio::resources::resource_explorer& manager) const
  {
    auto search_values = std::array<std::string, 2>{{shared::to_lower(file_info.filename.extension().string()), shared::to_lower(file_info.filename.string())}};

    for (const auto& search_value : search_values)
    {
      auto archive_type = validators.equal_range(search_value);

      for (auto it = archive_type.first; it != archive_type.second; ++it)
      {
        if (it->second(stream))
        {
          return creators.at(it->second)({ file_info, manager, actions }, stream);
        }
      }
    }

    for (auto& [checker, creator] : creators)
    {
      if (auto allowed = no_fallback_allowed.find(checker); allowed != no_fallback_allowed.end())
      {
        return normal_view(default_view(file_info));
      }

      if (checker(stream))
      {
        return creator({ file_info, manager, actions }, stream);
      }
    }

    return normal_view(default_view(file_info));
  }

  studio_view  view_factory::create_default_view(const studio::resources::file_info& file_info, std::basic_istream<std::byte>& stream, const studio::resources::resource_explorer& manager) const
  {
    return normal_view(default_view(file_info));
  }
}// namespace studio::views
