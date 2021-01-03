#include <array>
#include <functional>
#include <iostream>
#include <deque>
#include <memory>
#include <optional>

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>

#include "utility.hpp"

#include "canvas_painter.hpp"
#include "views/view_factory.hpp"

#include "archives/darkstar_volume.hpp"
#include "archives/three_space_volume.hpp"
#include "archives/trophy_bass_volume.hpp"

namespace fs = std::filesystem;

studio::fs::file_stream create_null_stream()
{
  static studio::fs::null_buffer null_buffer;
  return std::make_pair(shared::archive::file_info{}, std::make_unique<std::basic_istream<std::byte>>(&null_buffer));
}

struct tree_item_file_info : public wxTreeItemData
{
  shared::archive::file_info info;

  explicit tree_item_file_info(shared::archive::file_info info) : info(std::move(info)) {}
};

struct tree_item_folder_info : public wxTreeItemData
{
  shared::archive::folder_info info;

  explicit tree_item_folder_info(shared::archive::folder_info info) : info(std::move(info)) {}
};

std::optional<std::filesystem::path> get_shape_path()
{
  constexpr static auto filetypes = "Darkstar DTS files|*.dts;*.DTS";
  auto dialog = std::make_unique<wxFileDialog>(nullptr, "Open a Darkstar DTS File", "", "", filetypes, wxFD_OPEN, wxDefaultPosition);

  if (dialog->ShowModal() == wxID_OK)
  {
    const auto buffer = dialog->GetPath().ToAscii();
    return std::string_view{ buffer.data(), buffer.length() };
  }

  return std::nullopt;
}

std::optional<std::filesystem::path> get_workspace_path()
{
  auto dialog = std::make_unique<wxDirDialog>(nullptr, "Open a folder to use as a workspace");

  if (dialog->ShowModal() == wxID_OK)
  {
    const auto buffer = dialog->GetPath().ToAscii();
    return std::string_view{ buffer.data(), buffer.length() };
  }

  return std::nullopt;
}

wxAppConsole* createApp()
{
  wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
    "3Space Studio");
  return new wxApp();
}

constexpr auto event_open_in_new_tab = 1;
constexpr auto event_open_folder_as_workspace = 2;

wxMenuBar* create_menu_bar()
{
  auto* menuFile = new wxMenu();
  menuFile->Append(wxID_OPEN);
  menuFile->Append(event_open_in_new_tab, "Open in New Tab...");
  menuFile->Append(event_open_folder_as_workspace, "Open Folder as Workspace");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  auto* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  auto* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");

  return menuBar;
}

void create_render_view(wxWindow* panel, studio::fs::file_stream file_stream, const view_factory& factory, const studio::fs::file_system_archive& archive)
{
  auto* graphics = new wxControl(panel, -1, wxDefaultPosition, wxDefaultSize, 0);

  panel->GetSizer()->Add(graphics, 1, wxEXPAND | wxALL, 5);

  sf::ContextSettings context;
  context.depthBits = 24;
  auto* window = new sf::RenderWindow(get_handle(graphics), context);
  static bool is_init = false;
  static ImGuiContext* primary_gui_context;

  ImGuiContext* gui_context;

  if (!is_init)
  {
    ImGui::SFML::Init(*window);

    primary_gui_context = gui_context = ImGui::GetCurrentContext();
    is_init = true;
  }
  else
  {
    gui_context = ImGui::CreateContext(ImGui::GetIO().Fonts);
  }

  graphics_view* handler = factory.create_view(file_stream.first, *file_stream.second, archive);

  graphics->SetClientObject(handler);

  graphics->Bind(wxEVT_ERASE_BACKGROUND, [](auto& event) {});

  graphics->Bind(wxEVT_SIZE, [=](auto& event) {
    handler->setup_gl(window, graphics, gui_context);
  });

  graphics->Bind(wxEVT_IDLE, [=](auto& event) {
    graphics->Refresh();
  });

  graphics->Bind(wxEVT_PAINT, canvas_painter(window, graphics, gui_context, handler));

  graphics->Bind(wxEVT_DESTROY, [=](auto& event) {
    delete window;
    if (gui_context != primary_gui_context)
    {
      ImGui::DestroyContext(gui_context);
      ImGui::SetCurrentContext(primary_gui_context);
    }
  });
}

void populate_tree_view(const view_factory& view_factory,
  studio::fs::file_system_archive& archive,
  wxTreeCtrl* tree_view,
  const fs::path& search_path,
  const std::vector<std::string_view>& extensions,
  std::optional<wxTreeItemId> parent = std::nullopt)
{
  using namespace std::literals;

  if (archive.is_regular_file(search_path))
  {
    return;
  }

  bool is_root = false;
  if (!parent.has_value())
  {
    tree_view->DeleteAllItems();
    is_root = true;
  }

  parent = parent.has_value() ? parent : tree_view->AddRoot(search_path.string());

  auto files_folders = archive.get_content_listing(search_path);

  std::sort(
    files_folders.begin(), files_folders.end(), [](const auto& a, const auto& b) {
      return std::visit([&](const auto& entry) {
        using AType = std::decay_t<decltype(entry)>;

        return std::visit([&](const auto& other) {
          using BType = std::decay_t<decltype(other)>;

          if constexpr (std::is_same_v<AType, shared::archive::folder_info> && std::is_same_v<BType, AType>)
          {
            return entry.full_path < other.full_path;
          }

          if constexpr (std::is_same_v<AType, shared::archive::file_info> && std::is_same_v<BType, AType>)
          {
            return entry.filename < other.filename;
          }

          if constexpr (std::is_same_v<AType, shared::archive::file_info> && std::is_same_v<BType, shared::archive::folder_info>)
          {
            return false;
          }

          if constexpr (std::is_same_v<AType, shared::archive::folder_info> && std::is_same_v<BType, shared::archive::file_info>)
          {
            return true;
          }

          return false;
        },
          b);
      },
        a);
    });

  for (auto& item : files_folders)
  {
    std::visit([&](auto&& folder) {
      using T = std::decay_t<decltype(folder)>;

      if constexpr (std::is_same_v<T, shared::archive::folder_info>)
      {
        auto new_parent = tree_view->AppendItem(parent.value(), folder.full_path.filename().string(), -1, -1, new tree_item_folder_info(folder));

        if (is_root)
        {
          populate_tree_view(view_factory, archive, tree_view, folder.full_path, extensions, new_parent);
        }
      }
    },
      item);
  }

  for (auto& item : files_folders)
  {
    std::visit([&](auto&& file) {
      using T = std::decay_t<decltype(file)>;
      if constexpr (std::is_same_v<T, shared::archive::file_info>)
      {
        if (std::any_of(extensions.begin(), extensions.end(), [&file](const auto& ext) {
              return ends_with(to_lower(file.filename.string()), ext);
            }))
        {
          tree_view->AppendItem(parent.value(), file.filename.string(), -1, -1, new tree_item_file_info(file));
        }
      }
    },
      item);
  }

  if (is_root)
  {
    tree_view->Expand(parent.value());
  }
}

int main(int argc, char** argv)
{
  try
  {
    auto search_path = fs::current_path();
    studio::fs::file_system_archive archive(search_path);

    archive.add_archive_type(".tbv", std::make_unique<trophy_bass::vol::tbv_file_archive>());
    archive.add_archive_type(".rbx", std::make_unique<trophy_bass::vol::rbx_file_archive>());
    archive.add_archive_type(".rmf", std::make_unique<three_space::vol::rmf_file_archive>());
    archive.add_archive_type(".map", std::make_unique<three_space::vol::rmf_file_archive>());
    archive.add_archive_type(".vga", std::make_unique<three_space::vol::rmf_file_archive>());
    archive.add_archive_type(".dyn", std::make_unique<three_space::vol::dyn_file_archive>());
    archive.add_archive_type(".vol", std::make_unique<three_space::vol::vol_file_archive>());
    archive.add_archive_type(".vol", std::make_unique<darkstar::vol::vol_file_archive>());

    view_factory view_factory = create_default_view_factory();

    wxApp::SetInitializerFunction(createApp);
    wxEntryStart(argc, argv);
    auto* app = wxApp::GetInstance();
    app->CallOnInit();

    auto* frame = new wxFrame(nullptr, wxID_ANY, "3Space Studio");

    frame->Bind(
      wxEVT_MENU, [](auto& event) {
        wxMessageBox("This is a tool to explore files using the 3Space or Darkstar engines. Currently only Starsiege, Starsiege Tribes, Trophy Bass 3D and Front Page Sports: Ski Racing are supported.",
          "About 3Space Studio",
          wxOK | wxICON_INFORMATION);
      },
      wxID_ABOUT);

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);

    auto* tree_panel = new wxPanel(frame, wxID_ANY);
    sizer->Add(tree_panel, 20, wxEXPAND, 0);

    auto* tree_sizer = new wxBoxSizer(wxVERTICAL);
    tree_panel->SetSizer(tree_sizer);

    auto* tree_search = new wxComboBox(tree_panel, wxID_ANY);
    tree_sizer->Add(tree_search, 2, wxEXPAND, 0);

    auto extensions = view_factory.get_extensions();

    tree_search->Append("All Supported Formats");
    tree_search->Append("All Palettes (.pal, .ppl, .ipl)");
    tree_search->Append("All Images (.bmp, .pba)");

    for (auto& extension : extensions)
    {
      tree_search->Append(std::string(extension));
    }

    tree_search->SetSelection(0);

    auto* tree_view = new wxTreeCtrl(tree_panel);
    tree_sizer->Add(tree_view, 98, wxEXPAND, 0);

    populate_tree_view(view_factory, archive, tree_view, search_path, extensions);

    auto& get_filter_selection = [&tree_search, &extensions]() {
      const auto selection = tree_search->GetSelection();

      static std::vector<std::string_view> new_extensions;

      if (selection == 0)
      {
        return extensions;
      }
      else if (selection == 1 || selection == 2)
      {
        new_extensions.clear();

        if (selection == 1)
        {
          new_extensions.emplace_back(".pal");
          new_extensions.emplace_back(".ppl");
          new_extensions.emplace_back(".ipl");
        }
        else if (selection == 2)
        {
          new_extensions.emplace_back(".bmp");
          new_extensions.emplace_back(".pba");
        }
      }
      else
      {
        new_extensions.clear();
        new_extensions.emplace_back(extensions[selection - 3]);
      }

      return new_extensions;
    };

    tree_search->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
      populate_tree_view(view_factory, archive, tree_view, search_path, get_filter_selection());
    });

    auto* notebook = new wxAuiNotebook(frame, wxID_ANY);
    auto num_elements = notebook->GetPageCount();

    sizer->Add(notebook, 80, wxEXPAND, 0);

    auto add_element_from_file = [notebook, &num_elements, &view_factory, &archive](auto new_stream, bool replace_selection = false) {
      auto* panel = new wxPanel(notebook, wxID_ANY);
      panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));

      auto new_path = new_stream.first;
      create_render_view(panel, std::move(new_stream), view_factory, archive);

      if (replace_selection)
      {
        auto selection = notebook->GetSelection();
        notebook->InsertPage(selection, panel, new_path.filename.string());
        num_elements = notebook->GetPageCount();

        if (num_elements > 2)
        {
          notebook->DeletePage(selection + 1);
        }

        notebook->ChangeSelection(selection);
      }
      else
      {
        notebook->InsertPage(notebook->GetPageCount() - 1, panel, new_path.filename.string());
        num_elements = notebook->GetPageCount();
        notebook->ChangeSelection(notebook->GetPageCount() - 2);
      }
    };

    auto add_new_element = [notebook, &num_elements, &view_factory, &archive]() {
      auto* panel = new wxPanel(notebook, wxID_ANY);
      panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
      create_render_view(panel, create_null_stream(), view_factory, archive);
      notebook->InsertPage(notebook->GetPageCount() - 1, panel, "New Tab");
      notebook->ChangeSelection(notebook->GetPageCount() - 2);
      num_elements = notebook->GetPageCount();
    };

    tree_view->Bind(wxEVT_TREE_ITEM_EXPANDING, [&view_factory, &archive, &get_filter_selection, tree_view](wxTreeEvent& event) {
      auto item = event.GetItem();

      if (item == tree_view->GetRootItem())
      {
        return;
      }

      if (tree_view->HasChildren(item))
      {
        wxTreeItemIdValue cookie = nullptr;

        auto child = tree_view->GetFirstChild(item, cookie);

        if (cookie && !tree_view->HasChildren(child))
        {
          if (auto* real_info = dynamic_cast<tree_item_folder_info*>(tree_view->GetItemData(child)); real_info)
          {
            populate_tree_view(view_factory, archive, tree_view, real_info->info.full_path, get_filter_selection(), child);
          }
        }

        do {
          child = tree_view->GetNextChild(item, cookie);

          if (cookie && !tree_view->HasChildren(child))
          {
            if (auto* real_info = dynamic_cast<tree_item_folder_info*>(tree_view->GetItemData(child)); real_info)
            {
              populate_tree_view(view_factory, archive, tree_view, real_info->info.full_path, get_filter_selection(), child);
            }
          }
        } while (cookie != nullptr);
      }
    });

    tree_view->Bind(wxEVT_TREE_ITEM_ACTIVATED, [&archive, tree_view, &add_element_from_file](wxTreeEvent& event) {
      static bool had_first_activation = false;
      auto item = event.GetItem();

      if (item == tree_view->GetRootItem())
      {
        return;
      }

      if (auto* real_info = dynamic_cast<tree_item_file_info*>(tree_view->GetItemData(item)); real_info)
      {
        add_element_from_file(archive.load_file(real_info->info), had_first_activation == false);
        had_first_activation = true;
      }
    });

    auto* panel = new wxPanel(notebook, wxID_ANY);
    panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    create_render_view(panel, create_null_stream(), view_factory, archive);
    notebook->AddPage(panel, "New Tab");

    panel = new wxPanel(notebook, wxID_ANY);
    panel->SetName("+");
    notebook->AddPage(panel, "+");

    notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED,
      [notebook, &add_new_element](wxAuiNotebookEvent& event) {
        auto* tab = notebook->GetPage(event.GetSelection());

        if (tab->GetName() == "+")
        {
          if (notebook->GetPageCount() == 1)
          {
            add_new_element();
          }
          else
          {
            notebook->ChangeSelection(event.GetSelection() - 1);
          }
        }
      });

    notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING,
      [notebook, &num_elements, &add_new_element](wxAuiNotebookEvent& event) mutable {
        auto* tab = notebook->GetPage(event.GetSelection());

        if (tab->GetName() == "+")
        {
          if (num_elements > notebook->GetPageCount())
          {
            num_elements = notebook->GetPageCount();
            return;
          }

          add_new_element();
        }
        else
        {
          event.Skip();
        }
      });

    frame->Bind(
      wxEVT_MENU, [&](auto& event) {
        const auto new_path = get_shape_path();

        if (new_path.has_value())
        {
          add_element_from_file(archive.load_file(new_path.value()), true);

          search_path = new_path.value().parent_path();
          populate_tree_view(view_factory, archive, tree_view, search_path, get_filter_selection());
        }
      },
      wxID_OPEN);

    frame->Bind(
      wxEVT_MENU, [&](auto& event) {
        const auto new_path = get_shape_path();

        if (new_path.has_value())
        {
          add_element_from_file(archive.load_file(new_path.value()));
        }
      },
      event_open_in_new_tab);

    frame->Bind(
      wxEVT_MENU, [&](auto& event) {
        const auto new_path = get_workspace_path();

        if (new_path.has_value())
        {
          search_path = new_path.value();
          populate_tree_view(view_factory, archive, tree_view, search_path, get_filter_selection());
        }
      },
      event_open_folder_as_workspace);

    frame->Bind(
      wxEVT_MENU, [frame](auto& event) {
        frame->Close(true);
      },
      wxID_EXIT);

    sizer->SetSizeHints(frame);
    frame->SetSizer(sizer);

    frame->SetMenuBar(create_menu_bar());
    frame->CreateStatusBar();
    frame->SetStatusText("3Space Studio");
    frame->Maximize();
    frame->Show(true);

    app->OnRun();

    ImGui::SFML::Shutdown();
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }

  return 0;
}