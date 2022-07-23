#include <utility>
#include <execution>

#include <wx/treelist.h>

#include "pal_mapping_view.hpp"
#include "utility.hpp"
#include "shared.hpp"
#include "bmp_shared.hpp"

namespace studio::views
{
  pal_mapping_view::pal_mapping_view(view_context context)
    : context(std::move(context))
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

    auto images = context.explorer.find_files(context.actions.get_extensions_by_category("all_images"));

    auto palettes = context.explorer.find_files(context.actions.get_extensions_by_category("all_palettes"));

    palette_data.load_palettes(context.explorer, palettes);

    auto root = table->GetRootItem();

    std::vector<std::array<std::string, 7>> loaded_data(images.size());

    std::transform(std::execution::par, images.begin(), images.end(), loaded_data.begin(), [&](const auto& image) {
      auto image_stream = context.explorer.load_file(image);

      auto bmp_data = load_image_data_for_pal_detection(image, *image_stream.second);

      auto default_palette = detect_default_palette(bmp_data.second, image, context.explorer, palette_data.loaded_palettes);

      return std::array<std::string, 7>{{ image.folder_path.string(),
            image.filename.string(),
            std::string(default_palette.first),
            std::to_string(default_palette.second),
            std::string(default_palette.first),
            std::to_string(default_palette.second),
            ""}};
    });


    for (auto& strings : loaded_data)
    {
      auto new_id = table->AppendItem(root, std::move(strings[0]), -1, -1);

      for (auto i = 1; i < strings.size(); ++i)
      {
        table->SetItemText(new_id, i, std::move(strings[i]));
      }
    }

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(table.release(), 15, wxEXPAND, 0);

    parent.SetSizer(sizer.release());
  }
}// namespace studio::views