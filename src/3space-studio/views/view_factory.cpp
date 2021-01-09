#include "view_factory.hpp"
#include "vol_view.hpp"
#include "dts_io.hpp"

void view_factory::add_file_type(stream_validator* checker, view_creator* creator)
{
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

graphics_view* view_factory::create_view(const shared::archive::file_info& file_info, std::basic_istream<std::byte>& stream, const studio::fs::resource_explorer& manager) const
{
  auto archive_type = validators.equal_range(to_lower(file_info.filename.extension().string()));

  for (auto it = archive_type.first; it != archive_type.second; ++it)
  {
    if (it->second(stream))
    {
      return creators.at(it->second)(file_info, stream, manager);
    }
  }

  for (auto& [checker, creator] : creators)
  {
    if (checker(stream))
    {
      return creator(file_info, stream, manager);
    }
  }

  return new default_view(file_info);
}