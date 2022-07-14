#include <utility>

#include <wx/treelist.h>

#include "pal_mapping_view.hpp"
#include "utility.hpp"
#include "shared.hpp"
#include "bmp_shared.hpp"

namespace studio::views
{
  pal_mapping_view::pal_mapping_view(studio::resources::file_info info, std::basic_istream<std::byte>& json_stream, const studio::resources::resource_explorer& explorer)
    : info(std::move(info)), explorer(explorer)
    {

    }

  void pal_mapping_view::setup_view(wxWindow& parent)
  {
    auto table = std::unique_ptr<wxTreeListCtrl>(new wxTreeListCtrl(&parent, wxID_ANY));
    table->AppendColumn("Image Folder", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Image", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Default Palette", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Default Palette Index", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Selected Palette", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Selected Palette Index", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    table->AppendColumn("Actions", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);

    auto extensions = explorer.execute_action("get_extensions_by_category", { "All Images" });
    auto files = explorer.find_files(std::any_cast<std::vector<std::string_view>&>(extensions));

    auto root = table->GetRootItem();


    for (const auto& file : files)
    {
      auto new_id = table->AppendItem(root, file.folder_path.string(), -1, -1);


      table->SetItemText(new_id, 1, file.filename.string());
      table->SetItemText(new_id, 2, "Default");
      table->SetItemText(new_id, 3, "0");
      table->SetItemText(new_id, 4, "Default");
      table->SetItemText(new_id, 5, "0");
      table->SetItemText(new_id, 6, "");
    }

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(table.release(), 15, wxEXPAND, 0);

    parent.SetSizer(sizer.release());
  }
}// namespace studio::views