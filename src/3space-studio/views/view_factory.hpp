#ifndef DARKSTARDTSCONVERTER_VIEW_FACTORY_HPP
#define DARKSTARDTSCONVERTER_VIEW_FACTORY_HPP

#include <istream>

#include "graphics_view.hpp"
#include "default_view.hpp"

using stream_validator = bool(std::basic_istream<std::byte>&);

using view_creator = graphics_view*(std::basic_istream<std::byte>&);

class view_factory
{
public:
  void add_file_type(stream_validator* checker, view_creator* creator)
  {
    creators.emplace(checker, creator);
  }

  void add_extension(std::string_view extension, stream_validator* checker)
  {
    extensions.emplace_back(extension);
    validators.emplace(extensions.back(), checker);
  }

  std::vector<std::string_view> get_extensions() const
  {
    return std::vector<std::string_view>(extensions.cbegin(), extensions.cend());
  }

  graphics_view* create_view(std::string_view extension, std::basic_istream<std::byte>& stream) const
  {
    auto archive_type = validators.equal_range(extension);

    for (auto it = archive_type.first; it != archive_type.second; ++it)
    {
      if (it->second(stream))
      {
        return creators.at(it->second)(stream);
      }
    }

    return create_view(stream);
  }

  graphics_view* create_view(std::basic_istream<std::byte>& stream) const
  {
    for (auto& [checker, creator] : creators)
    {
      if (checker(stream))
      {
        return creator(stream);
      }
    }

    return new default_view();
  }

private:
  std::vector<std::string> extensions;

  std::map<stream_validator*, view_creator*> creators;
  std::multimap<std::string_view, stream_validator*> validators;
};

view_factory create_default_view_factory();

#endif//DARKSTARDTSCONVERTER_VIEW_FACTORY_HPP
