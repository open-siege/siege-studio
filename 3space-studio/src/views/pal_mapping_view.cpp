#include <utility>
#include <execution>

#include <wx/dataview.h>

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
    auto images = context.explorer.find_files(context.actions.get_extensions_by_category("all_images"));

    auto palettes = context.explorer.find_files(context.actions.get_extensions_by_category("all_palettes"));

    palette_data.load_palettes(context.explorer, palettes);

    wxArrayString available_palettes;
    available_palettes.reserve(palettes.size() + 2);

    available_palettes.Add(std::string(auto_generated_name));
    available_palettes.Add("Internal");

    for (auto& palette : palettes)
    {
      available_palettes.Add(get_palette_key(context.explorer, palette));
    }

    std::vector<wxVector<wxVariant>> loaded_data(images.size());

    std::transform(std::execution::par, images.begin(), images.end(), loaded_data.begin(), [&](const auto& image) {
      auto image_stream = context.explorer.load_file(image);

      auto bmp_data = load_image_data_for_pal_detection(image, *image_stream.second);

      auto default_palette = detect_default_palette(bmp_data.second, image, context.explorer, palette_data.loaded_palettes);

      auto selected_palette = selected_palette_from_settings(image, context.explorer, palette_data.loaded_palettes);

      wxVector<wxVariant> results;
      results.reserve(7);

      results.push_back(image.folder_path.string());
      results.push_back(image.filename.string());
      results.push_back(std::string(default_palette.first));
      results.push_back(std::to_string(default_palette.second));

      if (selected_palette.has_value())
      {
        results.push_back(std::string(selected_palette.value().first));
        results.push_back(std::to_string(selected_palette.value().second));
      }
      else
      {
        results.push_back(std::string(default_palette.first));
        results.push_back(std::to_string(default_palette.second));
      }

      results.push_back("");
      return results;
    });

    auto table = std::unique_ptr<wxDataViewListCtrl>(new wxDataViewListCtrl(&parent, wxID_ANY));
    table->AppendTextColumn("Image Folder", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    table->AppendTextColumn("Image", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

    auto selectionColumn = std::make_unique<wxDataViewColumn>("Default Palette",
      std::make_unique<wxDataViewChoiceRenderer>(available_palettes).release(),
      table->GetColumnCount(),
      wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    table->AppendColumn(selectionColumn.release());

    table->AppendTextColumn("Default Palette Index", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

    selectionColumn = std::make_unique<wxDataViewColumn>("Selected Palette",
      std::make_unique<wxDataViewChoiceRenderer>(available_palettes).release(),
      table->GetColumnCount(),
      wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    table->AppendColumn(selectionColumn.release());

    table->AppendTextColumn("Selected Palette Index", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    table->AppendTextColumn("Actions", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);


    for (auto& strings : loaded_data)
    {
      table->AppendItem(strings);
    }

    table->Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, [&](wxDataViewEvent& event) {
      auto* model = event.GetModel();

      wxVariant folder_path;
      model->GetValue(folder_path, event.GetItem(), 0);

      wxVariant filename;
      model->GetValue(filename, event.GetItem(), 1);

      wxVariant default_palette_key;
      model->GetValue(default_palette_key, event.GetItem(), 2);

      wxVariant default_palette_index;
      model->GetValue(default_palette_index, event.GetItem(), 3);


      wxVariant selected_palette_key;
      model->GetValue(selected_palette_key, event.GetItem(), 4);

      wxVariant selected_palette_index;
      model->GetValue(selected_palette_index, event.GetItem(), 5);

      auto palette_iter = palette_data.loaded_palettes.find(default_palette_key.GetString().utf8_string());

      studio::resources::file_info info;
      info.filename = filename.GetString().utf8_string();
      info.folder_path = folder_path.GetString().utf8_string();

      if (palette_iter != palette_data.loaded_palettes.end())
      {
        if (default_palette_index.GetInteger() >= palette_iter->second.second.size())
        {
          default_palette_index = palette_iter->second.second.empty() ? std::string("0") : std::to_string(palette_iter->second.second.size() - 1);
          model->SetValue(default_palette_index, event.GetItem(), 3);
        }
        set_default_palette(context.explorer,info,
          default_palette_key.GetString().utf8_string(),
          default_palette_index.GetInteger());
      }

      palette_iter = palette_data.loaded_palettes.find(default_palette_key.GetString().utf8_string());

      if (palette_iter != palette_data.loaded_palettes.end())
      {
        if (selected_palette_index.GetInteger() >= palette_iter->second.second.size())
        {
          selected_palette_index = palette_iter->second.second.empty() ? std::string("0") : std::to_string(palette_iter->second.second.size() - 1);
          model->SetValue(selected_palette_index, event.GetItem(), 5);
        }

        set_selected_palette(context.explorer,info,
          selected_palette_key.GetString().utf8_string(),
          selected_palette_index.GetInteger());
      }
    });

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(table.release(), 15, wxEXPAND, 0);

    parent.SetSizer(sizer.release());
  }
}// namespace studio::views