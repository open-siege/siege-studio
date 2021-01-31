#include "view_factory.hpp"
#include "vol_view.hpp"
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

  void view_factory::add_extension(std::string_view extension, stream_validator* checker)
  {
    validators.emplace(*extensions.emplace(extension).first, checker);
  }

  [[nodiscard]] std::vector<std::string_view> view_factory::get_extensions() const
  {
    return std::vector<std::string_view>(extensions.cbegin(), extensions.cend());
  }

  std::unique_ptr<studio_view> view_factory::create_view(const studio::resources::file_info& file_info, std::basic_istream<std::byte>& stream, const studio::resources::resource_explorer& manager) const
  {
    auto archive_type = validators.equal_range(shared::to_lower(file_info.filename.extension().string()));

    for (auto it = archive_type.first; it != archive_type.second; ++it)
    {
      if (it->second(stream))
      {
        return creators.at(it->second)(file_info, stream, manager);
      }
    }

    for (auto& [checker, creator] : creators)
    {
      if (auto allowed = no_fallback_allowed.find(checker); allowed != no_fallback_allowed.end())
      {
        return std::make_unique<default_view>(file_info);
      }

      if (checker(stream))
      {
        return creator(file_info, stream, manager);
      }
    }

    return std::make_unique<default_view>(file_info);
  }

  std::unique_ptr<studio_view> view_factory::create_default_view(const studio::resources::file_info& file_info, std::basic_istream<std::byte>& stream, const studio::resources::resource_explorer& manager) const
  {
    return std::make_unique<default_view>(file_info);
  }
}// namespace studio::views