#ifndef DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP

#include "graphics_view.hpp"
#include "view_context.hpp"
#include "resources/resource_explorer.hpp"

namespace studio::views
{
  class pal_mapping_view
  {
  public:
    explicit pal_mapping_view(view_context context);
    void setup_view(wxWindow& parent);

  private:
    view_context context;
  };
}

#endif//DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP
