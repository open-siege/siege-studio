#include <array>
#include <functional>
#include <iostream>
#include <map>
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
#include "archives/file_system_archive.hpp"

namespace fs = std::filesystem;

using optional_istream = std::optional<std::reference_wrapper<std::basic_istream<std::byte>>>;

static studio::fs::null_buffer null_buffer;
static std::basic_istream<std::byte> null_stream (&null_buffer);

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

std::optional<std::filesystem::path> get_shape_path(int argc, char** argv)
{
  if (argc > 1)
  {
    return argv[1];
  }

  return get_shape_path();
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

void create_render_view(wxWindow* panel, std::basic_istream<std::byte>& file_stream, const view_factory& factory)
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

  graphics_view* handler = factory.create_view(file_stream);

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

void populate_tree_view(studio::fs::file_system_archive& archive,
  wxTreeCtrl* tree_view,
  const fs::path& search_path,
  std::optional<wxTreeItemId> parent = std::nullopt)
{
  using namespace std::literals;
  constexpr std::array extensions = { ".dts"sv, ".DTS"sv };

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

  for (auto& item : files_folders)
  {
    std::visit([&](auto&& folder) {
      using T = std::decay_t<decltype(folder)>;

      if constexpr (std::is_same_v<T, shared::archive::folder_info>)
      {
        auto new_parent = tree_view->AppendItem(parent.value(), folder.full_path.filename().string(), -1, -1, new tree_item_folder_info(folder));

        if (is_root)
        {
          populate_tree_view(archive, tree_view, folder.full_path, new_parent);
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
              return ends_with(file.filename, ext);
            }))
        {
          tree_view->AppendItem(parent.value(), file.filename, -1, -1, new tree_item_file_info(file));
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
  studio::fs::file_system_archive archive;

  archive.add_archive_type(".tbv", std::make_unique<trophy_bass::vol::tbv_file_archive>());
  archive.add_archive_type(".rbx", std::make_unique<trophy_bass::vol::rbx_file_archive>());
  archive.add_archive_type(".rmf", std::make_unique<three_space::vol::rmf_file_archive>());
  archive.add_archive_type(".map", std::make_unique<three_space::vol::rmf_file_archive>());
  archive.add_archive_type(".vga", std::make_unique<three_space::vol::rmf_file_archive>());
  archive.add_archive_type(".dyn", std::make_unique<three_space::vol::dyn_file_archive>());
  archive.add_archive_type(".vol", std::make_unique<three_space::vol::vol_file_archive>());
  archive.add_archive_type(".vol", std::make_unique<darkstar::vol::vol_file_archive>());

  view_factory view_factory = create_default_view_factory();

  auto search_path = fs::current_path();

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

  auto* tree_view = new wxTreeCtrl(frame);
  sizer->Add(tree_view, 20, wxEXPAND, 0);

  populate_tree_view(archive, tree_view, search_path);

  auto* notebook = new wxAuiNotebook(frame, wxID_ANY);
  auto num_elements = notebook->GetPageCount();

  sizer->Add(notebook, 80, wxEXPAND, 0);

  auto add_element_from_file = [notebook, &num_elements, &view_factory](auto new_path, auto new_stream, bool replace_selection = false) {
    auto* panel = new wxPanel(notebook, wxID_ANY);
    panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    create_render_view(panel, new_stream, view_factory);

    if (replace_selection)
    {
      auto selection = notebook->GetSelection();
      notebook->InsertPage(selection, panel, new_path.filename().string());
      num_elements = notebook->GetPageCount();

      if (num_elements > 2)
      {
        notebook->DeletePage(selection + 1);
      }

      notebook->ChangeSelection(selection);
    }
    else
    {
      notebook->InsertPage(notebook->GetPageCount() - 1, panel, new_path.filename().string());
      num_elements = notebook->GetPageCount();
      notebook->ChangeSelection(notebook->GetPageCount() - 2);
    }
  };

  auto add_new_element = [notebook, &num_elements, &view_factory]() {
    auto* panel = new wxPanel(notebook, wxID_ANY);
    panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    create_render_view(panel, null_stream, view_factory);
    notebook->InsertPage(notebook->GetPageCount() - 1, panel, "New Tab");
    notebook->ChangeSelection(notebook->GetPageCount() - 2);
    num_elements = notebook->GetPageCount();
  };

  tree_view->Bind(wxEVT_TREE_ITEM_EXPANDING, [&archive, tree_view](wxTreeEvent& event) {
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
          populate_tree_view(archive, tree_view, real_info->info.full_path, child);
        }
      }

      do {
        child = tree_view->GetNextChild(item, cookie);

        if (cookie && !tree_view->HasChildren(child))
        {
          if (auto* real_info = dynamic_cast<tree_item_folder_info*>(tree_view->GetItemData(child)); real_info)
          {
            populate_tree_view(archive, tree_view, real_info->info.full_path, child);
          }
        }
      } while (cookie != nullptr);
    }
  });

  tree_view->Bind(wxEVT_TREE_ITEM_ACTIVATED, [&archive, tree_view, &add_element_from_file](wxTreeEvent& event) {
    auto item = event.GetItem();

    if (item == tree_view->GetRootItem())
    {
      return;
    }

    if (auto* real_info = dynamic_cast<tree_item_file_info*>(tree_view->GetItemData(item)); real_info)
    {
      auto& info = real_info->info;

      // TODO put this into the archive class
      // TODO needs to support compression too
      if (info.compression_type == shared::archive::compression_type::none)
      {
        auto full_path = info.folder_path / info.filename;

        if (std::filesystem::is_directory(info.folder_path))
        {
          auto file_stream = std::basic_ifstream<std::byte>(full_path, std::ios::binary);

          add_element_from_file(full_path, std::ref(file_stream), true);
        }
        else
        {
          auto archive_path = archive.get_archive_path(info.folder_path);
          auto file_stream = std::basic_ifstream<std::byte>(archive_path, std::ios::binary);

          archive.set_stream_position(archive_path, file_stream, info);

          add_element_from_file(full_path, std::ref(file_stream), true);
        }
      }
    }
  });

  auto* panel = new wxPanel(notebook, wxID_ANY);
  panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
  create_render_view(panel, null_stream, view_factory);
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
        //TODO fix this
        //add_element_from_file(new_path.value(), true);

        search_path = new_path.value().parent_path();
        populate_tree_view(archive, tree_view, search_path);
      }
    },
    wxID_OPEN);

  frame->Bind(
    wxEVT_MENU, [&](auto& event) {
      const auto new_path = get_shape_path();

      // TODO fix this
      if (new_path.has_value())
      {
        //add_element_from_file(new_path.value());
      }
    },
    event_open_in_new_tab);

  frame->Bind(
    wxEVT_MENU, [&](auto& event) {
      const auto new_path = get_workspace_path();

      if (new_path.has_value())
      {
        search_path = new_path.value();
        populate_tree_view(archive, tree_view, search_path);
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

  return 0;
}