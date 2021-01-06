#include <utility>
#include <execution>
#include <atomic>
#include <wx/treelist.h>
#include <wx/filepicker.h>

#include "vol_view.hpp"

vol_view::vol_view(const shared::archive::file_info& info, const studio::fs::file_system_archive& archive)
  : archive(archive)
{
  archive_path = info.folder_path / info.filename;
  files = archive.find_files(archive_path, { "ALL" });
}

void vol_view::setup_view(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext)
{
  const static std::map<shared::archive::compression_type, const char*> type_names{
    { shared::archive::compression_type::none, "None" },
    { shared::archive::compression_type::lz, "Lempel-Ziv" },
    { shared::archive::compression_type::lzh, "Lempel-Ziv w/ Huffman coding" },
    { shared::archive::compression_type::rle, "Run-Length Encoding" }
  };

  std::set<std::filesystem::path> folders;

  for (auto& file : files)
  {
    folders.emplace(file.folder_path);
  }

  auto* sizer = new wxBoxSizer(wxVERTICAL);

  auto* table = new wxTreeListCtrl(parent, wxID_ANY);
  table->AppendColumn("Filename");

  if (folders.size() > 1)
  {
    table->AppendColumn("Path");
  }
  table->AppendColumn("Size (in bytes)");
  table->AppendColumn("Compression Method");

  table->Bind(
    wxEVT_TREELIST_ITEM_ACTIVATED, [=](const wxTreeListEvent& event) {
      auto id = event.GetItem();
      std::filesystem::path path = archive_path;

      if (folders.size() > 1)
      {
        path = path / table->GetItemText(id, table->GetColumnCount() - 3).c_str().AsChar();
      }

      auto original_info = std::find_if(files.begin(), files.end(), [&](auto& info) {
        return info.folder_path == path && info.filename == table->GetItemText(id, 0).c_str().AsChar();
      });

      if (original_info != files.end())
      {
        archive.execute_action("open_new_tab", *original_info);
      }
    });

  auto root = table->GetRootItem();

  for (auto& file : files)
  {
    auto id = table->AppendItem(root, file.filename.string(), -1, -1);

    if (folders.size() > 1)
    {
      table->SetItemText(id, table->GetColumnCount() - 3, std::filesystem::relative(file.folder_path, archive_path).string());
    }

    table->SetItemText(id, table->GetColumnCount() - 2, std::to_string(file.size));
    table->SetItemText(id, table->GetColumnCount() - 1, type_names.at(file.compression_type));
  }


  auto* panel = new wxPanel(parent);

  auto* folder_picker = new wxDirPickerCtrl(panel, wxID_ANY, (archive.get_search_path() / "extracted").string());

  auto* export_button = new wxButton(panel, wxID_ANY, "Extract All Files");

  export_button->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
    should_cancel = false;
    auto* dialog = new wxDialog(parent, wxID_ANY, "Extracting Files");

    auto* dialog_sizer = new wxBoxSizer(wxVERTICAL);

    auto* gauge = new wxGauge(dialog, wxID_ANY, files.size());
    gauge->SetWindowStyle(wxGA_HORIZONTAL);

    auto* text1 = new wxStaticText(dialog, wxID_ANY, "");
    text1->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);

    auto* text2 = new wxStaticText(dialog, wxID_ANY, "");
    text2->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);
    dialog_sizer->AddStretchSpacer(2);
    dialog_sizer->Add(text1, 2, wxEXPAND, 0);
    dialog_sizer->Add(gauge, 2, wxEXPAND, 0);
    dialog_sizer->Add(text2, 2, wxEXPAND, 0);
    dialog_sizer->AddStretchSpacer(2);
    dialog->SetSizer(dialog_sizer);

    pending_save = std::async(std::launch::async, [=] {
      auto unique = std::unique_ptr<wxDialog>(dialog);
      auto dest = std::filesystem::path(folder_picker->GetPath().c_str().AsChar());
      unique->SetSize(unique->GetSize().x + dest.string().size(), unique->GetSize().y);
      unique->CenterOnScreen();
      unique->Show();
      text1->SetLabel("Extracting to\n" + (dest / archive_path.stem()).string());

      std::basic_ifstream<std::byte> archive_file(archive_path, std::ios::binary);

      for (auto& file : files)
      {
        if (should_cancel)
        {
          std::cout << "Stopping normal execution\n";
          return true;
        }
        text2->SetLabel((std::filesystem::relative(archive_path, file.folder_path) / file.filename).string());

        archive.extract_file_contents(archive_file, dest, file);
        gauge->SetValue(gauge->GetValue() + 1);
      }

      return true;
    });
    event.Skip();
  });

  auto* export_all_button = new wxButton(panel, wxID_ANY, "Extract All Volumes");

  export_all_button->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
    should_cancel = false;
    auto* dialog = new wxDialog(parent, wxID_ANY, "Extracting All Volumes");

    auto* dialog_sizer = new wxBoxSizer(wxVERTICAL);

    auto* gauge = new wxGauge(dialog, wxID_ANY, 0);
    gauge->SetWindowStyle(wxGA_HORIZONTAL);

    auto* text1 = new wxStaticText(dialog, wxID_ANY, "");
    text1->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);

    auto* text2 = new wxStaticText(dialog, wxID_ANY, "");
    text2->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);
    dialog_sizer->AddStretchSpacer(2);
    dialog_sizer->Add(text1, 2, wxEXPAND, 0);
    dialog_sizer->Add(gauge, 2, wxEXPAND, 0);
    dialog_sizer->Add(text2, 2, wxEXPAND, 0);
    dialog_sizer->AddStretchSpacer(2);
    dialog->SetSizer(dialog_sizer);

    dialog->Bind(wxEVT_CLOSE_WINDOW, [=](auto&) {
      should_cancel = true;
      std::cout << "Stopping the thread\n";
    });

    pending_save = std::async(std::launch::async, [=] {
      auto unique = std::unique_ptr<wxDialog>(dialog);
      auto dest = std::filesystem::path(folder_picker->GetPath().c_str().AsChar());
      unique->SetSize(unique->GetSize().x + dest.string().size(), unique->GetSize().y);
      unique->CenterOnScreen();
      unique->Show();
      text1->SetLabel("Extracting to\n" + dest.string());

      auto all_files = archive.find_files({ ".vol" });

      std::for_each(std::execution::par_unseq, all_files.begin(), all_files.end(), [=](const auto& volume_file) {
        static std::mutex label_mutex;
        static std::mutex gauge_mutex;

        auto file_archive_path = volume_file.folder_path / volume_file.filename;

        std::cout << (file_archive_path.string() + "\n");
        auto child_files = archive.find_files(file_archive_path, { "ALL" });

        {
          std::lock_guard<std::mutex> lock(gauge_mutex);
          gauge->SetRange(gauge->GetRange() + child_files.size());
        }

        std::basic_ifstream<std::byte> archive_file(file_archive_path, std::ios::binary);

        for (const auto& file : child_files)
        {
          if (should_cancel)
          {
            std::cout << "Stopping paralell execution\n";
            return;
          }

          {
            std::lock_guard<std::mutex> lock(label_mutex);
            text2->SetLabel((std::filesystem::relative(archive.get_search_path(), file.folder_path) / file.filename).string());
          }

          archive.extract_file_contents(archive_file, dest, file);

          {
            std::lock_guard<std::mutex> lock(gauge_mutex);
            gauge->SetValue(gauge->GetValue() + 1);
          }
        }
      });

      return true;
    });
    event.Skip();
  });

  auto* panel_sizer = new wxBoxSizer(wxHORIZONTAL);

  panel_sizer->Add(folder_picker, 4, wxEXPAND, 0);
  panel_sizer->AddStretchSpacer(1);
  panel_sizer->Add(export_button, 2, wxEXPAND, 0);
  panel_sizer->AddStretchSpacer(1);
  panel_sizer->Add(export_all_button, 2, wxEXPAND, 0);
  panel_sizer->AddStretchSpacer(12);

  panel->SetSizer(panel_sizer);

  sizer->Add(panel, 1, wxEXPAND, 0);
  sizer->Add(table, 15, wxEXPAND, 0);

  parent->SetSizer(sizer);
}