#ifndef DARKSTARDTSCONVERTER_CONFIG_HPP
#define DARKSTARDTSCONVERTER_CONFIG_HPP

#include "view_factory.hpp"
#include "siege/resource/resource_explorer.hpp"

namespace siege::views
{
  view_factory create_default_view_factory(const siege::resource::resource_explorer& explorer);

  siege::resource::resource_explorer create_default_resource_explorer();
}

#endif//DARKSTARDTSCONVERTER_CONFIG_HPP
