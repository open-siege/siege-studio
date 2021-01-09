#ifndef DARKSTARDTSCONVERTER_CONFIG_HPP
#define DARKSTARDTSCONVERTER_CONFIG_HPP

#include "view_factory.hpp"
#include "resource/resource_explorer.hpp"

view_factory create_default_view_factory();

studio::resource::resource_explorer create_default_resource_explorer(const std::filesystem::path& search_path);

#endif//DARKSTARDTSCONVERTER_CONFIG_HPP
