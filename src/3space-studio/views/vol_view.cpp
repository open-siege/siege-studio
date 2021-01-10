#include <utility>
#include <execution>
#include <atomic>
#include <wx/treelist.h>
#include <wx/filepicker.h>

#include "vol_view.hpp"
#include "3space-studio/utility.hpp"

vol_view::vol_view(const studio::resource::file_info& info, const studio::resource::resource_explorer& archive)
  : archive(archive)
{
  archive_path = info.folder_path / info.filename;
  files = archive.find_files(archive_path, { "ALL" });
}

void vol_view::setup_view(wxWindow& parent)
{
  const static std::map<studio::resource::compression_type, const char*> type_names{
    { studio::resource::compression_type::none, "None" },
    { studio::resource::compression_type::lz, "Lempel-Ziv" },
    { studio::resource::compression_type::lzh, "Lempel-Ziv w/ Huffman coding" },
    { studio::resource::compression_type::rle, "Run-Length Encoding" }
  };

  std::set<std::filesystem::path> folders;

  for (auto& file : files)
  {
    folders.emplace(file.folder_path);
  }

  auto table = std::shared_ptr<wxTreeListCtrl>(new wxTreeListCtrl(&parent, wxID_ANY), default_wx_deleter);
  table->AppendColumn("Filename");

  if (folders.size() > 1)
  {
    table->AppendColumn("Path");
  }
  table->AppendColumn("Size (in bytes)");
  table->AppendColumn("Compression Method");

  table->Bind(
    wxEVT_TREELIST_ITEM_ACTIVATED, [this, folders, table](const wxTreeListEvent& event) {
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

  auto panel = std::make_unique<wxPanel>(&parent);

  auto folder_picker = std::shared_ptr<wxDirPickerCtrl>(new wxDirPickerCtrl(panel.get(), wxID_ANY, (archive.get_search_path() / "extracted").string()), default_wx_deleter);

  auto export_button = std::make_unique<wxButton>(panel.get(), wxID_ANY, "Extract Volume Contents");

  export_button->Bind(wxEVT_BUTTON, [parent = &parent, this, folder_picker](wxCommandEvent& event) {
    should_cancel = false;
    auto dialog = std::make_unique<wxDialog>(parent, wxID_ANY, "Extracting Volume Contents");

    auto dialog_sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto gauge = std::shared_ptr<wxGauge>(new wxGauge(dialog.get(), wxID_ANY, files.size()), default_wx_deleter);
    gauge->SetWindowStyle(wxGA_HORIZONTAL);

    auto text1 = std::shared_ptr<wxStaticText>(new wxStaticText(dialog.get(), wxID_ANY, ""), default_wx_deleter);
    text1->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);

    auto text2 = std::shared_ptr<wxStaticText>(new wxStaticText(dialog.get(), wxID_ANY, ""), default_wx_deleter);
    text2->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);
    dialog_sizer->AddStretchSpacer(2);
    dialog_sizer->Add(text1.get(), 2, wxEXPAND, 0);
    dialog_sizer->Add(gauge.get(), 2, wxEXPAND, 0);
    dialog_sizer->Add(text2.get(), 2, wxEXPAND, 0);
    dialog_sizer->AddStretchSpacer(2);
    dialog->SetSizer(dialog_sizer.release());


    pending_save = std::async(std::launch::async, [this, dialog = dialog.release(), folder_picker, text1, text2, gauge] {
      auto dest = std::filesystem::path(folder_picker->GetPath().c_str().AsChar());
      std::filesystem::create_directory(dest);
      auto scoped_dialog = std::unique_ptr<wxDialog>(dialog);
      scoped_dialog->SetSize(scoped_dialog->GetSize().x + dest.string().size(), scoped_dialog->GetSize().y);
      scoped_dialog->CenterOnScreen();
      scoped_dialog->Show();
      text1->SetLabel("Extracting to\n" + (dest / archive_path.stem()).string());

      std::basic_ifstream<std::byte> archive_file(archive_path, std::ios::binary);

      for (auto& file : files)
      {
        if (should_cancel)
        {
          return true;
        }

        text2->SetLabel((std::filesystem::relative(file.folder_path, archive_path) / file.filename).string());

        archive.extract_file_contents(archive_file, dest, file);
        gauge->SetValue(gauge->GetValue() + 1);
      }

      if (!opened_folder)
      {
        wxLaunchDefaultApplication(dest.string());
        opened_folder = true;
      }

      return true;
    });
    event.Skip();
  });

  auto export_all_button = std::make_unique<wxButton>(panel.get(), wxID_ANY, "Extract All Volumes");

  export_all_button->Bind(wxEVT_BUTTON, [parent = &parent, this, folder_picker](wxCommandEvent& event) {
    should_cancel = false;
    auto dialog = std::make_unique<wxDialog>(parent, wxID_ANY, "Extracting All Volumes");

    auto dialog_sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto gauge = std::shared_ptr<wxGauge>(new wxGauge(dialog.get(), wxID_ANY, 0), default_wx_deleter);
    gauge->SetWindowStyle(wxGA_HORIZONTAL);

    auto text1 = std::shared_ptr<wxStaticText>(new wxStaticText(dialog.get(), wxID_ANY, ""), default_wx_deleter);
    text1->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);

    auto text2 = std::shared_ptr<wxStaticText>(new wxStaticText(dialog.get(), wxID_ANY, ""), default_wx_deleter);
    text2->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);
    dialog_sizer->AddStretchSpacer(2);
    dialog_sizer->Add(text1.get(), 2, wxEXPAND, 0);
    dialog_sizer->Add(gauge.get(), 2, wxEXPAND, 0);
    dialog_sizer->Add(text2.get(), 2, wxEXPAND, 0);
    dialog_sizer->AddStretchSpacer(2);
    dialog->SetSizer(dialog_sizer.release());

    dialog->Bind(wxEVT_CLOSE_WINDOW, [=](auto&) {
      should_cancel = true;
    });

    pending_save = std::async(std::launch::async, [this, dialog = dialog.release(), folder_picker, text1, text2, gauge] {
      auto dest = std::filesystem::path(folder_picker->GetPath().c_str().AsChar());
      std::filesystem::create_directory(dest);
      auto scoped_dialog = std::unique_ptr<wxDialog>(dialog);
      scoped_dialog->SetSize(scoped_dialog->GetSize().x + dest.string().size(), scoped_dialog->GetSize().y);
      scoped_dialog->CenterOnScreen();
      scoped_dialog->Show();
      text1->SetLabel("Extracting to\n" + dest.string());

      auto all_files = archive.find_files({ ".vol" });

      std::for_each(std::execution::par_unseq, all_files.begin(), all_files.end(), [=](const auto& volume_file) {
        static std::mutex label_mutex;
        static std::mutex gauge_mutex;

        auto file_archive_path = volume_file.folder_path / volume_file.filename;

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
            return;
          }

          {
            std::lock_guard<std::mutex> lock(label_mutex);
            text2->SetLabel((std::filesystem::relative(file.folder_path, archive.get_search_path()) / file.filename).string());
          }

          archive.extract_file_contents(archive_file, dest, file);

          {
            std::lock_guard<std::mutex> lock(gauge_mutex);
            gauge->SetValue(gauge->GetValue() + 1);
          }
        }
      });

      if (!opened_folder)
      {
        wxLaunchDefaultApplication(dest.string());
        opened_folder = true;
      }

      return true;
    });
    event.Skip();
  });

  panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());

  panel->GetSizer()->Add(folder_picker.get(), 4, wxEXPAND, 0);
  panel->GetSizer()->AddStretchSpacer(1);
  panel->GetSizer()->Add(export_button.release(), 2, wxEXPAND, 0);
  panel->GetSizer()->AddStretchSpacer(1);
  panel->GetSizer()->Add(export_all_button.release(), 2, wxEXPAND, 0);
  panel->GetSizer()->AddStretchSpacer(12);

  auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
  sizer->Add(panel.release(), 1, wxEXPAND, 0);
  sizer->Add(table.get(), 15, wxEXPAND, 0);

  parent.SetSizer(sizer.release());
}