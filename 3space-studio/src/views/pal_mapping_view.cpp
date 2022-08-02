#include <utility>
#include <execution>
#include <future>
#include <unordered_map>

#include <wx/dataview.h>
#include <wx/srchctrl.h>

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
    auto palettes = context.explorer.find_files(context.actions.get_extensions_by_category("all_palettes"));

    palette_data.load_palettes(context.explorer, palettes);

    auto panel = std::make_unique<wxPanel>(&parent);
    auto palettes_have_same_values = std::make_unique<wxCheckBox>(panel.get(), wxID_ANY, "Sync Palette Values");

    auto create_table = [this, &parent, palettes = std::move(palettes), palettes_have_same_values = palettes_have_same_values.get()]() {
      wxArrayString available_palettes;
      available_palettes.reserve(palettes.size() + 2);

      available_palettes.Add(std::string(auto_generated_name));
      available_palettes.Add("Internal");

      for (auto& palette : palettes)
      {
        available_palettes.Add(get_palette_key(context.explorer, palette));
      }

      auto table = std::unique_ptr<wxDataViewListCtrl>(new wxDataViewListCtrl(&parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES | wxDV_MULTIPLE));

      table->AppendTextColumn("Image Folder", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
      table->AppendTextColumn("Image", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

      auto selectionColumn = std::make_unique<wxDataViewColumn>("Default Palette",
        std::make_unique<wxDataViewChoiceRenderer>(available_palettes).release(),
        table->GetColumnCount(),
        wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
      table->AppendColumn(selectionColumn.release());


      selectionColumn = std::make_unique<wxDataViewColumn>("Default Palette Index",
        std::make_unique<wxDataViewSpinRenderer>(0, 255).release(),
        table->GetColumnCount(),
        wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

      table->AppendColumn(selectionColumn.release());

      selectionColumn = std::make_unique<wxDataViewColumn>("Selected Palette",
        std::make_unique<wxDataViewChoiceRenderer>(available_palettes).release(),
        table->GetColumnCount(),
        wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
      table->AppendColumn(selectionColumn.release());

      selectionColumn = std::make_unique<wxDataViewColumn>("Selected Palette Index",
        std::make_unique<wxDataViewSpinRenderer>(0, 255).release(),
        table->GetColumnCount(),
        wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

      table->AppendColumn(selectionColumn.release());

      table->AppendTextColumn("Actions", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

      table->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, [this, table = table.get()](wxDataViewEvent& event) {
        table->EditItem(event.GetItem(), event.GetDataViewColumn());
      });

      table->Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, [this, table = table.get(), palettes_have_same_values](wxDataViewEvent& event) {
        constexpr auto default_palette_key_col = 2;
        constexpr auto default_palette_index_col = 3;
        constexpr auto selected_palette_key_col = 4;
        constexpr auto selected_palette_index_col = 5;

        auto* model = event.GetModel();

        wxVariant default_palette_key;
        model->GetValue(default_palette_key, event.GetItem(), default_palette_key_col);

        wxVariant default_palette_index;
        model->GetValue(default_palette_index, event.GetItem(), default_palette_index_col);

        wxVariant selected_palette_key;
        model->GetValue(selected_palette_key, event.GetItem(), selected_palette_key_col);

        wxVariant selected_palette_index;
        model->GetValue(selected_palette_index, event.GetItem(), selected_palette_index_col);

        auto palette_iter = palette_data.loaded_palettes.find(default_palette_key.GetString().utf8_string());

        if (palette_iter != palette_data.loaded_palettes.end())
        {
          if (default_palette_index.GetInteger() >= palette_iter->second.second.size())
          {
            default_palette_index = palette_iter->second.second.empty() ? long{} : long(palette_iter->second.second.size() - 1);
            model->SetValue(default_palette_index, event.GetItem(), default_palette_index_col);
          }
        }

        palette_iter = palette_data.loaded_palettes.find(default_palette_key.GetString().utf8_string());

        if (palette_iter != palette_data.loaded_palettes.end())
        {
          if (selected_palette_index.GetInteger() >= palette_iter->second.second.size())
          {
            selected_palette_index = palette_iter->second.second.empty() ? long{} : long(palette_iter->second.second.size() - 1);
            model->SetValue(selected_palette_index, event.GetItem(), selected_palette_index_col);
          }
        }

        if (palettes_have_same_values->GetValue())
        {
          auto* column = event.GetDataViewColumn();

          if (column->GetModelColumn() == default_palette_key_col || column->GetModelColumn() == default_palette_index_col)
          {
            selected_palette_key = default_palette_key;
            selected_palette_index = default_palette_index;
            model->SetValue(selected_palette_key, event.GetItem(), selected_palette_key_col);
            model->SetValue(selected_palette_index, event.GetItem(), selected_palette_index_col);
          }
          else if (column->GetModelColumn() == selected_palette_key_col || column->GetModelColumn() == selected_palette_index_col)
          {
            default_palette_key = selected_palette_key;
            default_palette_index = selected_palette_index;
            model->SetValue(default_palette_key, event.GetItem(), default_palette_key_col);
            model->SetValue(default_palette_index, event.GetItem(), default_palette_index_col);
          }
        }

        auto file_info_for_item = [](const wxDataViewModel& model, const wxDataViewItem& item) {
          wxVariant folder_path;
          model.GetValue(folder_path, item, 0);

          wxVariant filename;
          model.GetValue(filename, item, 1);
          studio::resources::file_info info;
          info.filename = filename.GetString().utf8_string();
          info.folder_path = folder_path.GetString().utf8_string();

          return info;
        };

        auto info = file_info_for_item(*model, event.GetItem());

        auto* column = event.GetDataViewColumn();

        if (palettes_have_same_values->GetValue() || column->GetModelColumn() == default_palette_key_col || column->GetModelColumn() == default_palette_index_col)
        {
          set_default_palette(context.explorer, info,
            default_palette_key.GetString().utf8_string(),
            default_palette_index.GetInteger());
        }

        if (palettes_have_same_values->GetValue() || column->GetModelColumn() == selected_palette_key_col || column->GetModelColumn() == selected_palette_index_col)
        {
          set_selected_palette(context.explorer, info,
            selected_palette_key.GetString().utf8_string(),
            selected_palette_index.GetInteger());
        }

        wxDataViewItemArray selections;
        table->GetSelections(selections);

        for (auto& selection : selections)
        {
          if (selection != event.GetItem())
          {
            info = file_info_for_item(*model, selection);

            if (palettes_have_same_values->GetValue() || column->GetModelColumn() == default_palette_key_col || column->GetModelColumn() == default_palette_index_col)
            {
              model->SetValue(default_palette_key, selection, default_palette_key_col);
              model->SetValue(default_palette_index, selection, default_palette_index_col);

              set_default_palette(context.explorer, info,
                default_palette_key.GetString().utf8_string(),
                default_palette_index.GetInteger());
            }

            if (palettes_have_same_values->GetValue() || column->GetModelColumn() == selected_palette_key_col || column->GetModelColumn() == selected_palette_index_col)
            {
              model->SetValue(selected_palette_key, selection, selected_palette_key_col);
              model->SetValue(selected_palette_index, selection, selected_palette_index_col);

              set_selected_palette(context.explorer, info,
                selected_palette_key.GetString().utf8_string(),
                selected_palette_index.GetInteger());
            }
          }
        }
      });

      return table;
    };

    auto table = create_table();
    auto search_table = create_table();
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto images = context.explorer.find_files(context.actions.get_extensions_by_category("all_images"));

    auto search = std::make_unique<wxSearchCtrl>(panel.get(), wxID_ANY);
    search->ShowSearchButton(true);
    search->ShowCancelButton(true);

    auto open_in_tab = std::make_unique<wxButton>(panel.get(), wxID_ANY, "Open In New Tab");

    open_in_tab->Bind(wxEVT_BUTTON, [this, table = table.get(), images](wxCommandEvent& event) {
      wxDataViewItemArray selections;
      table->GetSelections(selections);

      auto* model = table->GetModel();

      for (auto& selection : selections)
      {
        wxVariant temp;
        model->GetValue(temp, selection, 0);
        auto folder_path = temp.GetString().utf8_string();

        model->GetValue(temp, selection, 1);
        auto filename = temp.GetString().utf8_string();

        auto possible_item = std::find_if(images.begin(), images.end(), [&](const auto& info) {
          return info.filename == filename && info.folder_path == folder_path;
        });

        if (possible_item != images.end())
        {
          context.actions.open_new_tab(*possible_item);
        }
      }
    });


    panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());

    panel->GetSizer()->Add(palettes_have_same_values.release(), 2, wxEXPAND, 0);
    panel->GetSizer()->Add(open_in_tab.release(), 2, wxEXPAND, 0);
    panel->GetSizer()->AddStretchSpacer(8);
    panel->GetSizer()->Add(search.get(), 4, wxEXPAND, 0);

    std::unordered_map<std::string, wxDataViewItem> table_rows;
    wxVector<wxVariant> results;
    results.reserve(7);

    for (auto& image : images)
    {
      results.push_back(image.folder_path.string());
      results.push_back(image.filename.string());
      results.push_back("?");
      results.push_back(0);
      results.push_back("?");
      results.push_back(0);

      results.push_back("");

      table->AppendItem(results);
      results.clear();
      table_rows.emplace((image.folder_path / image.filename).string(), table->RowToItem(table->GetItemCount() - 1));
    }

    auto cancel = [search = search.get(), sizer = sizer.get(), table = table.get(), search_table = search_table.get()](wxCommandEvent& event) {
      search->Clear();
      sizer->Hide(search_table);
      sizer->Show(table);
      sizer->Layout();
    };

    search->Bind(wxEVT_SEARCH, [sizer = sizer.get(),
                                 table = table.get(),
                                 search_table = search_table.get(),
                                 images,
                                 table_rows,
                                 cancel,
                                 this](wxCommandEvent& event) {
      if (event.GetString().empty())
      {
        cancel(event);
        return;
      }

      search_table->DeleteAllItems();

      auto search_value = shared::to_lower(event.GetString().utf8_string());

      wxVector<wxVariant> results;
      results.reserve(7);

      for (auto& image : images)
      {
        auto filename = shared::to_lower(image.filename.string());
        auto folder_name = shared::to_lower(std::filesystem::relative(image.folder_path, context.explorer.get_search_path()).string());

        if (filename.find(search_value) == std::string::npos && folder_name.find(search_value) == std::string::npos)
        {
          continue;
        }

        auto key = (image.folder_path / image.filename).string();
        auto row = table->ItemToRow(table_rows.at(key));

        wxVariant temp;

        for (auto i = 0u; i < table->GetColumnCount(); ++i)
        {
          table->GetValue(temp, row, i);
          results.push_back(temp);
        }

        search_table->AppendItem(results);
        results.clear();
      }

      sizer->Hide(table);
      sizer->Show(search_table);
      sizer->Layout();
    });

    search.release()->Bind(wxEVT_SEARCH_CANCEL, cancel);

    pending_load = std::async(std::launch::async, [this, table = table.get(), images = std::move(images), table_rows = std::move(table_rows)]() mutable {

      struct palette_row
      {
        wxDataViewItem item;
        std::pair<std::string_view, std::size_t> default_palette;
        std::pair<std::string_view, std::size_t> selected_palette;
      };

      auto distance = 16u;


      while (!images.empty())
      {
        auto begin = images.begin();
        auto end = begin;

        if (std::distance(end, images.end()) < distance)
        {
          end = images.end();
        }
        else
        {
          std::advance(end, distance);
        }

        std::vector<palette_row> temp(std::distance(begin, end));
        std::transform(std::execution::par_unseq, begin, end, temp.begin(), [&](const auto& image) -> palette_row {
          auto image_stream = context.explorer.load_file(image);

          auto bmp_data = load_image_data_for_pal_detection(image, *image_stream.second);

          const auto default_palette = detect_default_palette(bmp_data.second, image, context.explorer, palette_data.loaded_palettes);

          const auto selected_palette = selected_palette_from_settings(image, context.explorer, palette_data.loaded_palettes).value_or(default_palette);
          auto key = (image.folder_path / image.filename).string();
          return { table_rows.at(key), default_palette, selected_palette };
        });

        table->CallAfter([table = table, temp = std::move(temp)]() {
          auto* model = table->GetModel();

          // Duplicated because of ice errors when values are static
          constexpr auto default_palette_key_col = 2;
          constexpr auto default_palette_index_col = 3;
          constexpr auto selected_palette_key_col = 4;
          constexpr auto selected_palette_index_col = 5;

          for (auto& row : temp)
          {
            auto& [item, default_palette, selected_palette] = row;
            model->SetValue(std::string(default_palette.first), item, default_palette_key_col);
            model->SetValue(long(default_palette.second), item, default_palette_index_col);
            model->SetValue(std::string(selected_palette.first), item, selected_palette_key_col);
            model->SetValue(long(selected_palette.second), item, selected_palette_index_col);
          }
        });

        images.erase(begin, end);
      }
    });

    sizer->Add(panel.release(), 4, wxEXPAND, 0);
    sizer->Add(search_table.get(), 96, wxEXPAND, 0);
    sizer->Add(table.release(), 96, wxEXPAND, 0);
    sizer->Hide(search_table.release());

    parent.SetSizer(sizer.release());
  }
}// namespace studio::views