#ifndef DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
#define DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP

#include "graphics_view.hpp"
#include "resources/resource_explorer.hpp"

namespace studio::views
{
  class default_view
  {
  public:
    explicit default_view(studio::resources::file_info info);
    void setup_view(wxWindow& parent);

  private:
    studio::resources::file_info info;
  };
}

#endif//DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
