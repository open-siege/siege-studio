#include <utility>
#include "pal_mapping_view.hpp"

namespace studio::views
{
  pal_mapping_view::pal_mapping_view(studio::resources::file_info info)
    : info(std::move(info)) {}

  void pal_mapping_view::setup_view(wxWindow& parent)
  {

    auto horizontal = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto text = std::make_unique<wxStaticText>(&parent, wxID_ANY, "Development of this feature is still in progress.");
    text->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);


    sizer->AddStretchSpacer(4);
    sizer->Add(text.release(), 2, wxEXPAND, 0);
    sizer->AddStretchSpacer(4);

    horizontal->AddStretchSpacer(2);
    horizontal->Add(sizer.release(), 2, wxEXPAND, 0);
    horizontal->AddStretchSpacer(2);

    parent.SetSizer(horizontal.release());
  }
}// namespace studio::views