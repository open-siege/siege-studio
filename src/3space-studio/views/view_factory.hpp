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
    creators.emplace_back(checker, creator);
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
  std::vector<std::pair<stream_validator*, view_creator*>> creators;
};

view_factory create_default_view_factory();

#endif//DARKSTARDTSCONVERTER_VIEW_FACTORY_HPP
