#ifndef DARKSTARDTSCONVERTER_CONFIG_HPP
#define DARKSTARDTSCONVERTER_CONFIG_HPP

#include "view_factory.hpp"
#include "resources/resource_explorer.hpp"

namespace studio::views
{
  view_factory create_default_view_factory(const studio::resources::resource_explorer& explorer);

  studio::resources::resource_explorer create_default_resource_explorer();
}

#endif//DARKSTARDTSCONVERTER_CONFIG_HPP
