#ifndef DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP

#include "graphics_view.hpp"
#include "resources/resource_explorer.hpp"

namespace studio::views
{
  class pal_mapping_view
  {
  public:
    explicit pal_mapping_view(studio::resources::file_info, std::basic_istream<std::byte>&, const studio::resources::resource_explorer&);
    void setup_view(wxWindow& parent);

  private:
    studio::resources::file_info info;
    const studio::resources::resource_explorer& explorer;
  };
}

#endif//DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP
