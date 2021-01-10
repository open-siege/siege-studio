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
#include "views/config.hpp"

namespace fs = std::filesystem;

studio::resource::file_stream create_null_stream()
{
  static studio::resource::null_buffer null_buffer;
  return std::make_pair(studio::resource::file_info{}, std::make_unique<std::basic_istream<std::byte>>(&null_buffer));
}

struct tree_item_file_info : public wxTreeItemData
{
  studio::resource::file_info info;

  explicit tree_item_file_info(studio::resource::file_info info) : info(std::move(info)) {}
};

struct tree_item_folder_info : public wxTreeItemData
{
  studio::resource::folder_info info;

  explicit tree_item_folder_info(studio::resource::folder_info info) : info(std::move(info)) {}
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

auto create_menu_bar()
{
  auto menuFile = std::make_unique<wxMenu>();
  menuFile->Append(wxID_OPEN);
  menuFile->Append(event_open_in_new_tab, "Open in New Tab...");
  menuFile->Append(event_open_folder_as_workspace, "Open Folder as Workspace");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  auto menuHelp = std::make_unique<wxMenu>();
  menuHelp->Append(wxID_ABOUT);

  auto menuBar = std::make_unique<wxMenuBar>();
  menuBar->Append(menuFile.release(), "&File");
  menuBar->Append(menuHelp.release(), "&Help");

  return menuBar;
}

void create_render_view(wxWindow& panel, studio::resource::file_stream file_stream, const view_factory& factory, const studio::resource::resource_explorer& archive)
{
  std::unique_ptr<studio_view> raw_view;

  try
  {
    raw_view = factory.create_view(file_stream.first, *file_stream.second, archive);
  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what() << '\n';
    raw_view = factory.create_default_view(file_stream.first, *file_stream.second, archive);
  }

  if (auto* view = dynamic_cast<normal_view*>(raw_view.get()); view)
  {
    auto content_panel = std::make_unique<wxPanel>(&panel, wxID_ANY);

    view->setup_view(*content_panel);
    content_panel->SetClientObject(raw_view.release());

    panel.GetSizer()->Add(content_panel.release(), 1, wxEXPAND | wxALL, 5);
  }
  else if (auto other_view = std::dynamic_pointer_cast<graphics_view>(std::shared_ptr<studio_view>(std::move(raw_view))); other_view)
  {
    auto graphics = std::shared_ptr<wxControl>(new wxControl(&panel, -1, wxDefaultPosition, wxDefaultSize, 0), default_wx_deleter);

    sf::ContextSettings context;
    context.depthBits = 24;
    auto window = std::make_shared<sf::RenderWindow>(get_handle(*graphics), context);
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

    graphics->Bind(wxEVT_ERASE_BACKGROUND, [](auto& event) {});

    graphics->Bind(wxEVT_SIZE, [=](auto& event) {
      other_view->setup_view(*graphics, *window, *gui_context);
    });

    graphics->Bind(wxEVT_IDLE, [=](auto& event) {
      graphics->Refresh();
    });

    graphics->Bind(wxEVT_PAINT, canvas_painter(graphics, window, *gui_context, other_view));

    graphics->Bind(wxEVT_DESTROY, [=](auto& event) mutable {
      window.reset();
      graphics.reset();
      if (gui_context != primary_gui_context)
      {
        ImGui::DestroyContext(gui_context);
        ImGui::SetCurrentContext(primary_gui_context);
      }
    });

    other_view->setup_view(*graphics, *window, *gui_context);
    panel.GetSizer()->Add(graphics.get(), 1, wxEXPAND | wxALL, 5);
  }
}

void populate_tree_view(const view_factory& view_factory,
  studio::resource::resource_explorer& archive,
  wxTreeCtrl& tree_view,
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
    tree_view.DeleteAllItems();
    is_root = true;
  }

  parent = parent.has_value() ? parent : tree_view.AddRoot(search_path.string());

  auto files_folders = archive.get_content_listing(search_path);

  std::sort(
    files_folders.begin(), files_folders.end(), [](const auto& a, const auto& b) {
      return std::visit([&](const auto& entry) {
        using AType = std::decay_t<decltype(entry)>;

        return std::visit([&](const auto& other) {
          using BType = std::decay_t<decltype(other)>;

          if constexpr (std::is_same_v<AType, studio::resource::folder_info> && std::is_same_v<BType, AType>)
          {
            return entry.full_path < other.full_path;
          }

          if constexpr (std::is_same_v<AType, studio::resource::file_info> && std::is_same_v<BType, AType>)
          {
            return entry.filename < other.filename;
          }

          if constexpr (std::is_same_v<AType, studio::resource::file_info> && std::is_same_v<BType, studio::resource::folder_info>)
          {
            return false;
          }

          if constexpr (std::is_same_v<AType, studio::resource::folder_info> && std::is_same_v<BType, studio::resource::file_info>)
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

      if constexpr (std::is_same_v<T, studio::resource::folder_info>)
      {
        auto new_parent = tree_view.AppendItem(parent.value(), folder.full_path.filename().string(), -1, -1, new tree_item_folder_info(folder));

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
      if constexpr (std::is_same_v<T, studio::resource::file_info>)
      {
        if (std::any_of(extensions.begin(), extensions.end(), [&file](const auto& ext) {
              return ends_with(to_lower(file.filename.string()), ext);
            }))
        {
          tree_view.AppendItem(parent.value(), file.filename.string(), -1, -1, new tree_item_file_info(file));
        }
      }
    },
      item);
  }

  if (is_root)
  {
    tree_view.Expand(parent.value());
  }
}

int main(int argc, char** argv)
{
  try
  {
    auto search_path = fs::current_path();
    auto archive = create_default_resource_explorer(search_path);
    auto view_factory = create_default_view_factory();

    wxApp::SetInitializerFunction(createApp);
    wxEntryStart(argc, argv);
    auto* app = wxApp::GetInstance();
    app->CallOnInit();
    wxInitAllImageHandlers();

    auto frame = std::shared_ptr<wxFrame>(new wxFrame(nullptr, wxID_ANY, "3Space Studio"), default_wx_deleter);

    frame->Bind(
      wxEVT_MENU, [](auto& event) {
        wxMessageBox("This is a tool to explore files using the 3Space or Darkstar engines. Currently only Starsiege, Starsiege Tribes, Trophy Bass 3D and Front Page Sports: Ski Racing are supported.",
          "About 3Space Studio",
          wxOK | wxICON_INFORMATION);
      },
      wxID_ABOUT);

    auto tree_panel = std::make_unique<wxPanel>(frame.get(), wxID_ANY);
    tree_panel->SetSizer(std::make_unique<wxBoxSizer>(wxVERTICAL).release());

    auto tree_search = std::shared_ptr<wxComboBox>(new wxComboBox(tree_panel.get(), wxID_ANY), default_wx_deleter);

    auto extensions = view_factory.get_extensions();

    tree_search->Append("All Supported Formats");
    tree_search->Append("All Palettes (.pal, .ppl, .ipl)");
    tree_search->Append("All Images (.bmp, .pba)");
    tree_search->Append("All 3D Models (.dts)");

    for (auto& extension : extensions)
    {
      tree_search->Append(std::string(extension));
    }

    tree_search->SetSelection(0);

    auto tree_view = std::shared_ptr<wxTreeCtrl>(new wxTreeCtrl(tree_panel.get()), default_wx_deleter);

    populate_tree_view(view_factory, archive, *tree_view, search_path, extensions);

    auto get_filter_selection = [tree_search, &view_factory]() {
      const auto selection = tree_search->GetSelection();

      std::vector<std::string_view> new_extensions;

      if (selection == 0)
      {
        return view_factory.get_extensions();
      }
      else if (selection == 1 || selection == 2 || selection == 3)
      {
        if (selection == 1)
        {
          new_extensions.reserve(3);
          new_extensions.emplace_back(".pal");
          new_extensions.emplace_back(".ppl");
          new_extensions.emplace_back(".ipl");
        }
        else if (selection == 2)
        {
          new_extensions.reserve(2);
          new_extensions.emplace_back(".bmp");
          new_extensions.emplace_back(".pba");
        }
        else
        {
          new_extensions.emplace_back(".dts");
        }
      }
      else
      {
        new_extensions.emplace_back(view_factory.get_extensions()[selection - 4]);
      }

      return new_extensions;
    };

    tree_search->Bind(wxEVT_COMBOBOX, [&view_factory, &archive, tree_view, search_path, get_filter_selection](wxCommandEvent& event) {
      populate_tree_view(view_factory, archive, *tree_view, search_path, get_filter_selection());
    });

    auto notebook = std::shared_ptr<wxAuiNotebook>(new wxAuiNotebook(frame.get(), wxID_ANY), default_wx_deleter);
    auto num_elements = notebook->GetPageCount();

    auto add_element_from_file = [notebook, frame, &num_elements, &view_factory, &archive](auto new_stream, bool replace_selection = false) {
      auto panel = std::make_unique<wxPanel>(notebook.get(), wxID_ANY);
      panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());

      auto new_path = new_stream.first;
      create_render_view(*panel, std::move(new_stream), view_factory, archive);

      if (replace_selection)
      {
        auto selection = notebook->GetSelection();
        notebook->InsertPage(selection, panel.release(), new_path.filename.string());

        num_elements = notebook->GetPageCount();

        if (num_elements > 2)
        {
          notebook->DeletePage(selection + 1);
        }

        notebook->ChangeSelection(selection);

        if (frame->IsMaximized())
        {
          frame->Maximize(false);
          frame->Maximize();
        }
      }
      else
      {
        notebook->InsertPage(notebook->GetPageCount() - 1, panel.release(), new_path.filename.string());
        num_elements = notebook->GetPageCount();
        notebook->ChangeSelection(notebook->GetPageCount() - 2);
      }
    };
    archive.add_action("open_new_tab", [&](auto& path) {
      add_element_from_file(archive.load_file(path));
    });

    auto add_new_element = [notebook, &num_elements, &view_factory, &archive]() {
      auto panel = std::make_unique<wxPanel>(notebook.get(), wxID_ANY);
      panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());
      create_render_view(*panel, create_null_stream(), view_factory, archive);
      notebook->InsertPage(notebook->GetPageCount() - 1, panel.release(), "New Tab");
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
            populate_tree_view(view_factory, archive, *tree_view, real_info->info.full_path, get_filter_selection(), child);
          }
        }

        do {
          child = tree_view->GetNextChild(item, cookie);

          if (cookie && !tree_view->HasChildren(child))
          {
            if (auto* real_info = dynamic_cast<tree_item_folder_info*>(tree_view->GetItemData(child)); real_info)
            {
              populate_tree_view(view_factory, archive, *tree_view, real_info->info.full_path, get_filter_selection(), child);
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
      else if (auto* folder_info = dynamic_cast<tree_item_folder_info*>(tree_view->GetItemData(item));
               folder_info && !std::filesystem::is_directory(folder_info->info.full_path))
      {
        studio::resource::file_info info{};
        info.filename = folder_info->info.full_path.filename();
        info.folder_path = folder_info->info.full_path.parent_path();
        add_element_from_file(archive.load_file(info), true);
      }
    });

    auto panel = std::make_unique<wxPanel>(notebook.get(), wxID_ANY);
    panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());
    create_render_view(*panel, create_null_stream(), view_factory, archive);
    notebook->AddPage(panel.release(), "New Tab");

    panel = std::make_unique<wxPanel>(notebook.get(), wxID_ANY);
    panel->SetName("+");
    notebook->AddPage(panel.release(), "+");

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
          populate_tree_view(view_factory, archive, *tree_view, search_path, get_filter_selection());
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
          populate_tree_view(view_factory, archive, *tree_view, search_path, get_filter_selection());
        }
      },
      event_open_folder_as_workspace);

    frame->Bind(
      wxEVT_MENU, [frame](auto& event) {
        frame->Close(true);
      },
      wxID_EXIT);

    auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    sizer->SetSizeHints(frame.get());

    tree_panel->GetSizer()->Add(tree_search.get(), 2, wxEXPAND, 0);
    tree_panel->GetSizer()->Add(tree_view.get(), 98, wxEXPAND, 0);
    sizer->Add(tree_panel.release(), 20, wxEXPAND, 0);
    sizer->Add(notebook.get(), 80, wxEXPAND, 0);
    frame->SetSizer(sizer.release());

    frame->SetMenuBar(create_menu_bar().release());
    frame->CreateStatusBar();
    frame->SetStatusText("3Space Studio");
    frame->Maximize();
    frame->Show(true);

    auto result = app->OnRun();

    ImGui::SFML::Shutdown();

    return result;
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }

  return 0;
}