#ifndef DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
#define DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP

#include "graphics_view.hpp"
#include "siege/resource/resource_explorer.hpp"

namespace siege::views
{
  class default_view
  {
  public:
    explicit default_view(siege::platform::file_info info);
    void setup_view(wxWindow& parent);

  private:
    siege::platform::file_info info;
  };
}

#endif//DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
