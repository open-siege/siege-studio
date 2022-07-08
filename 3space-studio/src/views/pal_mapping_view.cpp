#include <utility>

#include <wx/treelist.h>

#include "pal_mapping_view.hpp"
#include "utility.hpp"
#include "shared.hpp"

namespace studio::views
{
  pal_mapping_view::pal_mapping_view(studio::resources::file_info info)
    : info(std::move(info)) {}

  void pal_mapping_view::setup_view(wxWindow& parent)
  {
    auto table = std::unique_ptr<wxTreeListCtrl>(new wxTreeListCtrl(&parent, wxID_ANY));
    table->AppendColumn("Image", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Default Palette", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Default Palette Index", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Selected Palette", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Selected Palette Index", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Actions", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);


    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(table.release(), 15, wxEXPAND, 0);

    parent.SetSizer(sizer.release());
  }
}// namespace studio::views