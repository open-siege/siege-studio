#include <array>
#include <functional>
#include <iostream>
#include <deque>
#include <memory>
#include <optional>

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/srchctrl.h>

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>

#include "utility.hpp"

#include "canvas_painter.hpp"
#include "views/config.hpp"

namespace fs = std::filesystem;

namespace studio
{
  studio::resources::file_stream create_null_stream()
  {
    static studio::resources::null_buffer null_buffer;
    return std::make_pair(studio::resources::file_info{}, std::make_unique<std::istream>(&null_buffer));
  }

  struct tree_item_file_info : public wxTreeItemData
  {
    studio::resources::file_info info;

    explicit tree_item_file_info(studio::resources::file_info info) : info(std::move(info)) {}
  };

  struct tree_item_folder_info : public wxTreeItemData
  {
    studio::resources::folder_info info;

    explicit tree_item_folder_info(studio::resources::folder_info info) : info(std::move(info)) {}
  };

  std::optional<std::filesystem::path> get_shape_path(std::vector<std::string_view>&& file_types)
  {
    std::stringstream final_types;

    for (auto i = 0; i < file_types.size(); ++i)
    {
      final_types << file_types[i] << " (*" << file_types[i] << ")|*" << file_types[i];

      if (i != file_types.size() - 1)
      {
        final_types << "|";
      }
    }
    auto dialog = std::make_unique<wxFileDialog>(nullptr, "Open a supported file", "", "", final_types.str(), wxFD_OPEN, wxDefaultPosition);

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
      "Siege Studio");
    return new wxApp();
  }

  constexpr auto event_open_in_new_tab = 1;
  constexpr auto event_open_folder_as_workspace = 2;
  constexpr auto event_close_current_tab = 3;
  constexpr auto event_close_other_tabs = 4;
  constexpr auto event_close_left_tabs = 5;
  constexpr auto event_close_right_tabs = 6;
  static bool sfml_initialised = false;

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

  template<typename T>
  struct view_data final : wxClientData
  {
    view_data(T view) : view(std::move(view)) {}
    T view;
  };

  void create_render_view(wxWindow& panel, studio::resources::file_stream file_stream, const views::view_factory& factory, const studio::resources::resource_explorer& archive)
  {
    views::studio_view raw_view;

    try
    {
      raw_view = factory.create_view(file_stream.first, *file_stream.second, archive);
    }
    catch (const std::exception& ex)
    {
      std::cerr << ex.what() << '\n';
      raw_view = factory.create_default_view(file_stream.first, *file_stream.second, archive);
    }

    std::visit([&](auto&& view) {
      using view_type = std::decay_t<decltype(view)>;

      if constexpr (std::is_same_v<view_type, views::normal_view>)
      {
        auto content_panel = std::make_unique<wxPanel>(&panel, wxID_ANY);

        view.setup_view(*content_panel);
        content_panel->SetClientObject(new view_data<views::normal_view>{ std::move(view) });

        panel.GetSizer()->Add(content_panel.release(), 1, wxEXPAND | wxALL, 5);
      }
      else if constexpr (std::is_same_v<view_type, views::graphics_view>)
      {
        auto shared_view = std::make_shared<views::graphics_view>(std::move(view));
        auto graphics = std::shared_ptr<wxControl>(new wxControl(&panel, -1, wxDefaultPosition, wxDefaultSize, 0), default_wx_deleter);

        sf::ContextSettings context;
        context.depthBits = 24;
// TODO It's much more complicated than anticipated to make everything work correctly.
// There are plans to overhaul everything in the future in any case.
// It makes more sense to get a compilable (and partially viable) solution going,
// then come back to this in the future. The main goal is to have a better separation
// in place for the ImGui-based UIs, such that they aren't aware of wxControls or anything like that.
// This can be addressed as part of the planned reworking.
#ifdef __WXGTK__
        static auto main_window = std::make_shared<sf::RenderWindow>(sf::VideoMode(640, 480), "Graphics Window", sf::Style::Default, context);

        if (!main_window->isOpen())
        {
          main_window = std::make_shared<sf::RenderWindow>(sf::VideoMode(640, 480), "Graphics Window", sf::Style::Default, context);
        }
        auto window = main_window;
        window->setTitle(file_stream.first.filename.string());
#else
        auto window = std::make_shared<sf::RenderWindow>(get_handle(*graphics), context);
#endif
        static std::unordered_map<std::shared_ptr<sf::RenderWindow>, ImGuiContext*> window_contexts;

        auto existing_context = window_contexts.find(window);

        if (existing_context == window_contexts.end())
        {
          // Preserving the context as it may still be in use and
          // replacing it while ImGui is busy could cause a crash.
          auto current_context = ImGui::GetCurrentContext();
          ImGui::SFML::Init(*window);
          studio::sfml_initialised = true;

          auto result = window_contexts.emplace(window, ImGui::GetCurrentContext());
          existing_context = result.first;
          ImGui::SetCurrentContext(current_context);
        }

        graphics->Bind(wxEVT_ERASE_BACKGROUND, [](auto& event) {});

        graphics->Bind(wxEVT_SIZE, [=](auto& event) {
          shared_view->setup_view(*graphics, *window, *existing_context->second);
        });

        graphics->Bind(wxEVT_IDLE, [=](auto& event) {
          graphics->Refresh();
        });

        graphics->Bind(wxEVT_PAINT, canvas_painter(graphics, window, *existing_context->second, shared_view));

#ifndef __WXGTK__
        // direct access of window in the below lambda causes an ICE in VS 2019.
        auto reset_imgui = [window]()
        {
          window_contexts.erase(window);
          if (window_contexts.empty())
          {
            studio::sfml_initialised = false;
            ImGui::SFML::Shutdown();
          }
          else
          {
            ImGui::SetCurrentContext(window_contexts.begin()->second);
            ImGui::SFML::Shutdown(*window);
          }
        };

        graphics->Bind(wxEVT_DESTROY, [=](auto& event) mutable {
          window.reset();
          graphics.reset();
          reset_imgui();
        });
#endif

        shared_view->setup_view(*graphics, *window, *existing_context->second);
        panel.GetSizer()->Add(graphics.get(), 1, wxEXPAND | wxALL, 5);
        window->requestFocus();
      }
    },
      std::move(raw_view));
  }

  void populate_tree_view(const views::view_factory& view_factory,
    studio::resources::resource_explorer& archive,
    wxTreeCtrl& tree_view,
    const fs::path& search_path,
    const std::vector<std::string_view>& extensions,
    std::optional<wxTreeItemId> parent = std::nullopt,
    std::optional<std::string_view> file_filter = std::nullopt)
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

            if constexpr (std::is_same_v<AType, studio::resources::folder_info> && std::is_same_v<BType, AType>)
            {
              return entry.full_path < other.full_path;
            }

            if constexpr (std::is_same_v<AType, studio::resources::file_info> && std::is_same_v<BType, AType>)
            {
              return entry.filename < other.filename;
            }

            if constexpr (std::is_same_v<AType, studio::resources::file_info> && std::is_same_v<BType, studio::resources::folder_info>)
            {
              return false;
            }

            if constexpr (std::is_same_v<AType, studio::resources::folder_info> && std::is_same_v<BType, studio::resources::file_info>)
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

        if constexpr (std::is_same_v<T, studio::resources::folder_info>)
        {
          auto new_parent = tree_view.AppendItem(parent.value(), folder.full_path.filename().string(), -1, -1, new tree_item_folder_info(folder));

          if (is_root)
          {
            populate_tree_view(view_factory, archive, tree_view, folder.full_path, extensions, new_parent, file_filter);
          }
        }
      },
        item);
    }

    for (auto& item : files_folders)
    {
      std::visit([&](auto&& file) {
        using T = std::decay_t<decltype(file)>;
        if constexpr (std::is_same_v<T, studio::resources::file_info>)
        {
          if (std::any_of(extensions.begin(), extensions.end(), [&file](const auto& ext) {
                return shared::ends_with(shared::to_lower(file.filename.string()), ext);
              }))
          {
            if (file_filter.has_value() && !file_filter.value().empty())
            {
              auto filename = shared::to_lower(file.filename.string());
              auto lower_search = shared::to_lower(file_filter.value());

              if (filename.find(lower_search) == std::string::npos)
              {
                return;
              }
            }
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

  auto create_navigation_panel(std::shared_ptr<wxFrame> frame,
    const views::view_factory& view_factory,
    studio::resources::resource_explorer& archive,
    std::function<void(studio::resources::file_stream, bool)> add_element_from_file)
  {
    auto tree_panel = std::make_unique<wxPanel>(frame.get(), wxID_ANY);
    tree_panel->SetSizer(std::make_unique<wxBoxSizer>(wxVERTICAL).release());

    auto tree_search = std::shared_ptr<wxComboBox>(new wxComboBox(tree_panel.get(), wxID_ANY), studio::default_wx_deleter);

    auto categories = view_factory.get_extension_categories();

    auto extensions = view_factory.get_extensions();

    for (auto& category : categories)
    {
      tree_search->Append(std::string(category));
    }

    for (auto& extension : extensions)
    {
      tree_search->Append(std::string(extension));
    }

    tree_search->SetSelection(0);

    auto tree_view = std::shared_ptr<wxTreeCtrl>(new wxTreeCtrl(tree_panel.get()), studio::default_wx_deleter);

    studio::populate_tree_view(view_factory, archive, *tree_view, fs::current_path(), extensions);

    auto get_filter_selection = [tree_search, &view_factory, categories, extensions]() {
      const auto selection = tree_search->GetSelection();

      if (selection < categories.size())
      {
        return view_factory.get_extensions_by_category(categories[selection]);
      }

      std::vector<std::string_view> new_extensions;
      new_extensions.emplace_back(extensions[selection - categories.size()]);

      return new_extensions;
    };

    auto search = std::shared_ptr<wxSearchCtrl>(new wxSearchCtrl(tree_panel.get(), wxID_ANY), studio::default_wx_deleter);
    search->ShowSearchButton(true);
    search->ShowCancelButton(true);

    auto repopulate = [&view_factory, &archive, tree_view, search, get_filter_selection]() {
      studio::populate_tree_view(view_factory, archive, *tree_view, fs::current_path(), get_filter_selection(), std::nullopt, search->GetValue().utf8_string());
    };

    search->Bind(wxEVT_SEARCH, [repopulate](wxCommandEvent& event) {
      repopulate();
    });

    search->Bind(wxEVT_SEARCH_CANCEL, [search, repopulate](wxCommandEvent& event) {
      search->Clear();
      repopulate();
    });

    tree_search->Bind(wxEVT_COMBOBOX, [repopulate](wxCommandEvent& event) {
      repopulate();
    });

    tree_view->Bind(wxEVT_TREE_ITEM_EXPANDING, [&view_factory, &archive, tree_view, get_filter_selection](wxTreeEvent& event) {
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
          if (auto* real_info = dynamic_cast<studio::tree_item_folder_info*>(tree_view->GetItemData(child)); real_info)
          {
            studio::populate_tree_view(view_factory, archive, *tree_view, real_info->info.full_path, get_filter_selection(), child);
          }
        }

        do {
          child = tree_view->GetNextChild(item, cookie);

          if (cookie && !tree_view->HasChildren(child))
          {
            if (auto* real_info = dynamic_cast<studio::tree_item_folder_info*>(tree_view->GetItemData(child)); real_info)
            {
              studio::populate_tree_view(view_factory, archive, *tree_view, real_info->info.full_path, get_filter_selection(), child);
            }
          }
        } while (cookie != nullptr);
      }
    });

    tree_view->Bind(wxEVT_TREE_ITEM_ACTIVATED, [&archive, tree_view, add_element_from_file](wxTreeEvent& event) {
      static bool had_first_activation = false;
      auto item = event.GetItem();

      if (item == tree_view->GetRootItem())
      {
        return;
      }

      if (auto* real_info = dynamic_cast<studio::tree_item_file_info*>(tree_view->GetItemData(item)); real_info)
      {
        add_element_from_file(archive.load_file(real_info->info), had_first_activation == false);
        had_first_activation = true;
      }
      else if (auto* folder_info = dynamic_cast<studio::tree_item_folder_info*>(tree_view->GetItemData(item));
               folder_info && !std::filesystem::is_directory(folder_info->info.full_path))
      {
        studio::resources::file_info info{};
        info.filename = folder_info->info.full_path.filename();
        info.folder_path = folder_info->info.full_path.parent_path();
        add_element_from_file(archive.load_file(info), true);
      }
    });

    tree_panel->GetSizer()->Add(tree_search.get(), 2, wxEXPAND, 0);
    tree_panel->GetSizer()->Add(search.get(), 2, wxEXPAND, 0);
    tree_panel->GetSizer()->Add(tree_view.get(), 96, wxEXPAND, 0);

    return std::make_pair(std::move(tree_panel), repopulate);
  }
}// namespace studio

int main(int argc, char** argv)
{
  try
  {
    auto archive = studio::views::create_default_resource_explorer();
    auto view_factory = studio::views::create_default_view_factory(archive);

    view_factory.get_actions().get_extensions_by_category = [&](auto path) {
      return view_factory.get_extensions_by_category(path);
    };

    wxApp::SetInitializerFunction(studio::createApp);
    wxEntryStart(argc, argv);
    auto* app = wxApp::GetInstance();
    app->CallOnInit();
    wxInitAllImageHandlers();

    auto frame = std::shared_ptr<wxFrame>(new wxFrame(nullptr, wxID_ANY, "Siege Studio"), studio::default_wx_deleter);

#if _WIN32 || _WIN64
    frame->SetIcon(wxICON(MAINICON));
#endif
    frame->Bind(
      wxEVT_MENU, [](auto& event) {
        wxMessageBox("This is a tool to explore files using the 3Space or Darkstar engines. Currently only Starsiege, Starsiege Tribes, Trophy Bass 3D and Front Page Sports: Ski Racing are supported.",
          "About Siege Studio",
          wxOK | wxICON_INFORMATION);
      },
      wxID_ABOUT);

    auto notebook = std::shared_ptr<wxAuiNotebook>(new wxAuiNotebook(frame.get(), wxID_ANY), studio::default_wx_deleter);
    auto num_elements = notebook->GetPageCount();

    auto add_element_from_file = [notebook, frame, &num_elements, &view_factory, &archive](auto new_stream, bool replace_selection = false) {
      auto panel = std::make_unique<wxPanel>(notebook.get(), wxID_ANY);
      panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());

      auto new_path = new_stream.first;
      studio::create_render_view(*panel, std::move(new_stream), view_factory, archive);

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
      }
      else
      {
        notebook->InsertPage(notebook->GetPageCount() - 1, panel.release(), new_path.filename.string());
        num_elements = notebook->GetPageCount();
        notebook->ChangeSelection(notebook->GetPageCount() - 2);
      }

      if (frame->IsMaximized())
      {
        frame->Maximize(false);
        frame->Maximize();
      }
    };

    view_factory.get_actions().open_new_tab = [&](auto& path) {
      add_element_from_file(archive.load_file(path));
    };

    auto add_new_element = [notebook, &num_elements, &view_factory, &archive]() {
      auto new_tab_count = 0;

      for (auto i = 0u; i < notebook->GetPageCount(); ++i)
      {
        if (notebook->GetPageText(i) == "New Tab")
        {
          new_tab_count++;
        }
      }

      if (new_tab_count >= 5)
      {
        return;
      }

      auto panel = std::make_unique<wxPanel>(notebook.get(), wxID_ANY);
      panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());
      studio::create_render_view(*panel, studio::create_null_stream(), view_factory, archive);

      notebook->InsertPage(notebook->GetPageCount() - 1, panel.release(), "New Tab");
      notebook->ChangeSelection(notebook->GetPageCount() - 2);
      num_elements = notebook->GetPageCount();
    };


    auto panel = std::make_unique<wxPanel>(notebook.get(), wxID_ANY);
    panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());
    studio::create_render_view(*panel, studio::create_null_stream(), view_factory, archive);
    notebook->AddPage(panel.release(), "New Tab");

    panel = std::make_unique<wxPanel>(notebook.get(), wxID_ANY);
    panel->SetName("+");
    notebook->AddPage(panel.release(), "+");

    auto menuTab = std::make_unique<wxMenu>();
    menuTab->Append(studio::event_close_current_tab, "Close");
    menuTab->Append(studio::event_close_other_tabs, "Close Other Tabs");
    menuTab->Append(studio::event_close_left_tabs, "Close Tabs to the Left");
    menuTab->Append(studio::event_close_right_tabs, "Close Tabs to the Right");

    auto selection = notebook->GetSelection();

    notebook->Bind(wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, [notebook, &selection, menuTab = menuTab.get()](wxAuiNotebookEvent& event) {
      selection = event.GetSelection();
      notebook->PopupMenu(menuTab);
    });

    notebook->Bind(
      wxEVT_MENU, [notebook, &selection](auto& event) {
        if (notebook->GetPageCount() > 2 && notebook->GetPageText(selection) != "+")
        {
          notebook->DeletePage(selection);
        }
      },
      studio::event_close_current_tab);

    auto close_right_tabs = [notebook, &selection](auto& event) {
      for (auto i = notebook->GetPageCount(); i > selection; --i)
      {
        if (notebook->GetPageText(i) != "+")
        {
          notebook->DeletePage(i);
        }
      }
      selection = notebook->GetSelection();
    };

    auto close_left_tabs = [notebook, &selection](auto& event) {
      for (auto i = selection - 1; i >= 0; --i)
      {
        notebook->DeletePage(i);
      }
      selection = notebook->GetSelection();
    };

    notebook->Bind(
      wxEVT_MENU, [notebook, &close_left_tabs, &close_right_tabs](auto& event) {
        close_left_tabs(event);
        close_right_tabs(event);
      },
      studio::event_close_other_tabs);

    notebook->Bind(wxEVT_MENU, close_left_tabs,studio::event_close_left_tabs);
    notebook->Bind(wxEVT_MENU, close_right_tabs,studio::event_close_right_tabs);

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

    auto navigation_panel_data = studio::create_navigation_panel(frame, view_factory, archive, add_element_from_file);
    auto tree_view = std::move(navigation_panel_data.first);
    auto repopulate = navigation_panel_data.second;

    frame->Bind(
      wxEVT_MENU, [&](auto& event) {
        const auto new_path = studio::get_shape_path(view_factory.get_extensions());

        if (new_path.has_value())
        {
          add_element_from_file(archive.load_file(new_path.value()), true);

          repopulate();
        }
      },
      wxID_OPEN);

    frame->Bind(
      wxEVT_MENU, [&](auto& event) {
        const auto new_path = studio::get_shape_path(view_factory.get_extensions());

        if (new_path.has_value())
        {
          add_element_from_file(archive.load_file(new_path.value()));
        }
      },
      studio::event_open_in_new_tab);

    frame->Bind(
      wxEVT_MENU, [&](auto& event) {
        const auto new_path = studio::get_workspace_path();

        if (new_path.has_value())
        {
          fs::current_path(new_path.value());
          repopulate();
        }
      },
      studio::event_open_folder_as_workspace);

    frame->Bind(
      wxEVT_MENU, [frame](auto& event) {
        frame->Close(true);
      },
      wxID_EXIT);

    auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    sizer->SetSizeHints(frame.get());


    sizer->Add(tree_view.release(), 20, wxEXPAND, 0);
    sizer->Add(notebook.get(), 80, wxEXPAND, 0);
    frame->SetSizer(sizer.release());

    frame->SetMenuBar(studio::create_menu_bar().release());
    frame->CreateStatusBar();
    frame->SetStatusText("Siege Studio");
    frame->Maximize();
    frame->Show(true);

    auto result = app->OnRun();

    if (studio::sfml_initialised)
    {
      ImGui::SFML::Shutdown();
    }

    return result;
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }

  return 0;
}
