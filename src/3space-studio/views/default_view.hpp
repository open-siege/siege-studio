#ifndef DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
#define DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP

#include "graphics_view.hpp"
#include "resource/resource_explorer.hpp"

namespace studio::views
{
  class default_view : public normal_view
  {
  public:
    explicit default_view(studio::resource::file_info info);
    void setup_view(wxWindow& parent) override;

  private:
    studio::resource::file_info info;
  };
}

#endif//DARKSTARDTSCONVERTER_DEFAULT_VIEW_HPP
