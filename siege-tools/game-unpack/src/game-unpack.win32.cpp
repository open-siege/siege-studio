#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
#include <siege/platform/installation_module.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/resource/resource_maker.hpp>
#include <span>
#include <string_view>
#include <array>
#include <ranges>
#include <map>
#include <set>
#include <fstream>
#include <regex>
#include <filesystem>
#include <ranges>

#pragma comment(linker, \
  "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace fs = std::filesystem;
namespace stl = std::ranges;

struct installable_file : siege::platform::file_info
{
  std::function<void(fs::path, siege::fs_string_view)> install_to;
};

bool is_standalone_console()
{
  std::array<DWORD, 4> ids{};

  return GetConsoleProcessList(ids.data(), (DWORD)ids.size()) == 1;
}

void exec_on_thread(HANDLE thread_handle, std::move_only_function<void()> callback)
{
  struct handler
  {
    static void CALLBACK user_apc(ULONG_PTR arg)
    {
      std::unique_ptr<std::move_only_function<void()>> callback_context{ (std::move_only_function<void()>*)arg };

      callback_context->operator()();
    }
  };

  auto result = ::QueueUserAPC(handler::user_apc, thread_handle, (ULONG_PTR) new std::move_only_function<void()>{ std::move(callback) });
  assert(result != 0);
}

int main(int argc, const char* argv[])
{
  auto args = std::span(argv, argc) | std::views::transform([](char const* v) { return std::string_view(v); });

  if (args.size() == 1)
  {
    win32::com::init_com();
    if (is_standalone_console())
    {
      ::ShowWindow(::GetConsoleWindow(), SWP_HIDEWINDOW);
    }
    win32::init_common_controls_ex({ .dwSize{ sizeof(INITCOMMONCONTROLSEX) }, .dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES });


    class installation_state
    {
    public:
      TASKDIALOG_FLAGS get_flags() const
      {
        auto result = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS | TDF_CALLBACK_TIMER;

        if (search_in_progress)
        {
          result |= TDF_SHOW_MARQUEE_PROGRESS_BAR;
        }

        return result;
      }

      TASKDIALOG_COMMON_BUTTON_FLAGS get_common_buttons() const
      {
        TASKDIALOG_COMMON_BUTTON_FLAGS result = TDCBF_YES_BUTTON;

        if (!current_page)
        {
          result |= TDCBF_CANCEL_BUTTON;
        }

        return result;
      }

      std::span<const TASKDIALOG_BUTTON> get_buttons() const
      {
        if (!current_page)
        {
          return {};
        }

        return current_page->buttons;
      }

      void navigate_page(HWND window, std::optional<int> page = std::nullopt)
      {
        if (page)
        {
          current_page = &pages[*page];
        }
        else
        {
          current_page++;
        }

        refresh_page(window);
      };

      void refresh_page(HWND window)
      {
        TASKDIALOGCONFIG config{
          .cbSize = sizeof(TASKDIALOGCONFIG),
          .dwFlags = get_flags(),
          .dwCommonButtons = get_common_buttons(),
          .pszWindowTitle = title.c_str(),
          .cButtons = (UINT)get_buttons().size(),
          .pButtons = get_buttons().data()
        };
        ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);

        last_flags = get_flags();
      }

      bool should_refresh()
      {
        bool progress_visible = last_flags & TDF_SHOW_MARQUEE_PROGRESS_BAR;

        if (!search_in_progress && progress_visible)
        {
          return true;
        }

        if (search_in_progress && !progress_visible)
        {
          return true;
        }

        return false;
      }

      bool is_processing() const
      {
        return search_in_progress;
      }

      void start_progress()
      {
        search_in_progress = true;
      }

      void end_progress()
      {
        search_in_progress = false;
      }

      void disable_page()
      {
        for (auto& button : current_page->buttons)
        {
          disabled_buttons.emplace(button.nButtonID);
        }
      }

      void enable_page()
      {
        disabled_buttons.clear();
      }

      std::vector<int> get_enabled_buttons() const
      {
        std::vector<int> results;

        for (auto& button : current_page->buttons)
        {
          if (!disabled_buttons.contains(button.nButtonID))
          {
            results.emplace_back(button.nButtonID);
          }
        }

        return results;
      }

      std::vector<int> get_disabled_buttons() const
      {
        std::vector<int> results;
        results.reserve(disabled_buttons.size());

        for (auto& button : current_page->buttons)
        {
          if (disabled_buttons.contains(button.nButtonID))
          {
            results.emplace_back(button.nButtonID);
          }
        }

        return results;
      }

      std::wstring title = L"Siege Game Unpacker";

    private:
      bool search_in_progress = false;
      std::set<int> disabled_buttons;

      struct page
      {
        std::wstring content = L"";
        std::vector<TASKDIALOG_BUTTON> buttons;
      } default_page;

      std::vector<page> pages = {
        { .content = L"To install a game from a back, select an option below: ",
          .buttons = {
            TASKDIALOG_BUTTON{ .nButtonID = 10, .pszButtonText = L"Install Game From File" },
            TASKDIALOG_BUTTON{ .nButtonID = 11, .pszButtonText = L"Detect Game From Storage" } } }
      };

      page* current_page = &pages.front();

      TASKDIALOG_FLAGS last_flags;
    } state;


    auto result = win32::task_dialog_indirect({
                                                .cbSize = sizeof(TASKDIALOGCONFIG),
                                                .dwFlags = state.get_flags(),
                                                .dwCommonButtons = state.get_common_buttons(),
                                                .pszWindowTitle = state.title.c_str(),
                                                .cButtons = (UINT)state.get_buttons().size(),
                                                .pButtons = state.get_buttons().data(),

                                              },
      [&](auto window, auto message, auto wparam, auto lparam) -> LRESULT {
        if (message == TDN_BUTTON_CLICKED && wparam == 10)
        {
          auto dialog = win32::com::CreateFileOpenDialog();

          if (dialog)
          {
            auto open_dialog = *dialog;

            open_dialog.SetFolder(std::filesystem::current_path());

            auto result = open_dialog->Show(nullptr);

            if (result == S_OK)
            {
              auto selection = open_dialog.GetResult();

              if (selection)
              {
                using namespace siege::platform;
                auto path = selection.value().GetFileSysPath();
                state.start_progress();
                state.disable_page();
                state.refresh_page(window);

                auto ui_handle = ::GetCurrentThread();
                win32::queue_user_work_item([ui_handle, window, backup_path = *path, &state] {
                  std::ifstream game_backup(backup_path, std::ios::binary);

                  if (siege::resource::is_resource_reader(game_backup))
                  {
                    auto installation_modules = siege::platform::installation_module::load_modules(fs::current_path());

                    auto stem = backup_path.stem().wstring();

                    for (auto& module : installation_modules)
                    {
                      // TODO resolve from environment
                      auto resolved_variables = std::map<std::wstring, std::wstring>{
                        { L"systemDrive", L"C:" }
                      };

                      for (auto& variable : module.installation_variables)
                      {
                        // TODO ask the user to select the option
                        // Though the first option should be the most likely by default.
                        auto options = module.get_options_for_variable(variable.name);

                        if (!options.empty())
                        {
                          resolved_variables.emplace(variable.name, options[0].name);
                        }
                      }

                      std::set<std::wstring> names_to_check;

                      if (module.storage_properties)
                      {
                        for (auto* name : module.storage_properties->disc_names)
                        {
                          if (!name)
                          {
                            break;
                          }
                          names_to_check.emplace(name);
                        }
                      }

                      for (auto name : module.name_variations)
                      {
                        names_to_check.emplace(name);
                      }

                      if (stl::any_of(names_to_check, [&](auto& item) {
                            return item == stem;
                          }))
                      {
                        auto mappings = module.directory_mappings;

                        std::set<fs::path> verification_mappings;
                        std::multimap<fs::path, fs::path> child_verification_mappings;

                        bool requires_cab_tooling = false;

                        for (auto& mapping : mappings)
                        {
                          if (mapping.enforcement == mapping.optional)
                          {
                            continue;
                          }

                          auto full_path = std::wstring(mapping.source);
                          auto stem = fs::path(mapping.source).stem().wstring();

                          if (stem.starts_with(L"<") && stem.ends_with(L">"))
                          {
                            continue;
                          }

                          auto temp = fs::path(full_path);

                          if (temp.parent_path().has_extension() || stem.contains(L"*"))
                          {
                            verification_mappings.emplace(temp.parent_path());

                            if (stem != L"*")
                            {
                              child_verification_mappings.emplace(temp.parent_path(), temp);
                            }
                            if (!requires_cab_tooling && (temp.parent_path().extension() == ".cab" || temp.parent_path().extension() == ".CAB"))
                            {
                              requires_cab_tooling = true;
                            }

                            continue;
                          }

                          verification_mappings.emplace(temp);
                        }

                        // TODO ask user to download cab tooling

                        auto reader = std::shared_ptr(siege::resource::make_resource_reader(game_backup));

                        std::list<installable_file> backup_files;
                        std::any cache{};
                        std::any cab_cache{};

                        auto top_level_items = reader->get_content_listing(cache, game_backup, listing_query{ .archive_path = backup_path, .folder_path = backup_path });

                        bool should_continue = true;

                        std::function<void(decltype(top_level_items)&)> get_full_listing = [&](std::vector<resource_reader::content_info>& items) mutable {
                          for (resource_reader::content_info& info : items)
                          {
                            if (!should_continue)
                            {
                              break;
                            }
                            if (auto parent_info = std::get_if<folder_info>(&info); parent_info)
                            {
                              std::vector<resource_reader::content_info> children;
                              children = reader->get_content_listing(cache, game_backup, listing_query{ .archive_path = backup_path, .folder_path = parent_info->full_path });
                              get_full_listing(children);
                            }

                            if (auto leaf_info = std::get_if<file_info>(&info); leaf_info)
                            {
                              auto& new_item = backup_files.emplace_back(*leaf_info);
                              new_item.install_to = [new_item = &new_item, reader, cache, backup_path](auto install_path, auto install_segment) mutable {
                                std::ifstream game_backup(backup_path, std::ios::binary);
                                fs::create_directories(install_path / install_segment);
                                auto final_path = install_path / install_segment / new_item->filename;
                                std::ofstream temp_out_buffer(final_path, std::ios::binary | std::ios::trunc);
                                reader->extract_file_contents(cache, game_backup, *new_item, temp_out_buffer);
                              };
                            }
                          }
                        };

                        get_full_listing(top_level_items);

                        // first level verification
                        if (stl::all_of(verification_mappings, [&](auto& mapping) {
                              return stl::any_of(backup_files, [&](auto& item) {
                                return mapping == item.relative_path() || mapping == item.relative_path() / item.filename;
                              });
                            }))
                        {
                          std::list<resource_reader::file_info> exe_files;

                          stl::copy_if(backup_files, std::back_inserter(exe_files), [](auto& item) {
                            return item.filename.extension() == ".exe" || item.filename.extension() == ".EXE";
                          });

                          if (!module.associated_extensions.empty() && !exe_files.empty())
                          {
                            // TODO exe verification
                          }


                          auto staging_path = fs::temp_directory_path() / L"game-unpack" / fs::path(module.GetModuleFileName<wchar_t>()).stem();

                          fs::create_directories(staging_path);

                          bool is_verified = child_verification_mappings.empty();

                          if (!is_verified)
                          {
                            for (auto& backup_file : backup_files)
                            {
                              auto backup_path = backup_file.relative_path() / backup_file.filename;

                              auto should_extract = verification_mappings.contains(backup_path) || backup_file.filename.extension() == ".hdr" || backup_file.filename.extension() == ".HDR";

                              if (!should_extract)
                              {
                                continue;
                              }

                              fs::create_directories(staging_path / backup_file.relative_path());
                              auto final_path = staging_path / backup_path;
                              std::ofstream temp_out_buffer(final_path, std::ios::binary | std::ios::trunc);
                              reader->extract_file_contents(cache, game_backup, backup_file, temp_out_buffer);
                            }

                            for (auto& backup_file : backup_files)
                            {
                              auto backup_path = backup_file.relative_path() / backup_file.filename;

                              if (!verification_mappings.contains(backup_path))
                              {
                                continue;
                              }

                              auto final_path = staging_path / backup_path;

                              std::ifstream temp_in_buffer(final_path, std::ios::binary);

                              if (!siege::resource::is_resource_reader(temp_in_buffer))
                              {
                                continue;
                              }

                              auto inner_reader = std::shared_ptr(siege::resource::make_resource_reader(temp_in_buffer));
                              std::any inner_cache{};
                              // TODO more than the top level
                              auto top_level_children = inner_reader->get_content_listing(inner_cache, temp_in_buffer, listing_query{ .archive_path = final_path, .folder_path = final_path })
                                                        | std::views::filter([](auto& info) { return std::get_if<file_info>(&info) != nullptr; })
                                                        | std::views::transform([](auto& info) -> file_info& { return std::get<file_info>(info); });

                              auto [first, end] = child_verification_mappings.equal_range(backup_path);

                              auto all_valid = std::all_of(first, end, [&](auto& verification_name) { return stl::any_of(top_level_children, [&](file_info& child) {
                                                                                                        return verification_name.second == (fs::relative(child.folder_path, staging_path) / child.filename);
                                                                                                      }); });

                              is_verified = is_verified || all_valid;

                              for (auto& file : top_level_children)
                              {
                                auto& new_item = backup_files.emplace_back(file);
                                new_item.install_to = [new_item = &new_item, inner_reader, inner_cache, archive_path = final_path](auto install_path, auto install_segment) mutable {
                                  std::ifstream game_backup(archive_path, std::ios::binary);
                                  fs::create_directories(install_path / install_segment);
                                  auto final_path = install_path / install_segment / new_item->filename;
                                  std::ofstream temp_out_buffer(final_path, std::ios::binary | std::ios::trunc);
                                  inner_reader->extract_file_contents(inner_cache, game_backup, *new_item, temp_out_buffer);
                                };
                              }
                            }
                          }

                          if (!is_verified)
                          {
                            // TODO ask user if they want to do a generic extract
                            continue;
                          }

                          // TODO ask user for install path
                          auto install_path = std::wstring(module.storage_properties->default_install_path.data());
                          install_path = std::regex_replace(install_path, std::wregex(L"<systemDrive>"), resolved_variables.at(L"systemDrive"));

                          fs::create_directories(install_path);

                          for (auto& mapping : mappings)
                          {
                            std::wstring source(mapping.source);

                            if (source.contains(L"<") && source.contains(L">"))
                            {
                              for (auto& item : resolved_variables)
                              {
                                source = std::regex_replace(source, std::wregex(L"<" + item.first + L">"), item.second);
                              }
                            }

                            std::function<bool(file_info&)> is_supported = [source = fs::path(source)](file_info& item) {
                              auto source_filename_str = std::wstring_view(source.filename().c_str());
                              if (source_filename_str.contains(L"*"))
                              {
                                if (source.has_extension())
                                {
                                  return item.relative_path() == source.parent_path() && item.filename.extension() == source.extension();
                                }
                                return item.relative_path() == source.parent_path();
                              }

                              return source == item.relative_path() || source == (item.relative_path() / item.filename);
                            };

                            if (fs::path(source).parent_path().has_extension())
                            {
                              is_supported = [source = fs::path(source).make_preferred()](file_info& item) {
                                auto full_path = item.folder_path.make_preferred() / item.filename.make_preferred();

                                auto full_path_str = std::wstring_view(full_path.c_str());
                                auto source_str = std::wstring_view(source.c_str());

                                if (source_str.contains(L"*"))
                                {
                                  auto start_str = source_str.substr(0, source_str.find(L"*"));
                                  auto end_str = source_str.substr(start_str.size() + 1);

                                  return full_path_str.contains(start_str) && full_path_str.ends_with(end_str);
                                }
                                else
                                {
                                  return full_path_str.ends_with(source_str);
                                }
                              };
                            }

                            auto files_to_copy = backup_files | std::views::filter(is_supported);

                            for (auto& item : files_to_copy)
                            {
                              item.install_to(fs::path(install_path), mapping.destination);
                            }
                          }
                        }
                      }
                    }
                  }

                  ::Sleep(10000);
                  exec_on_thread(ui_handle, [window, &state]() {
                    state.enable_page();
                    state.end_progress();
                    // state.navigate_page(window);
                  });
                });

                // state.navigate_page(window);
              }
            }
          }
        }


        if (message == TDN_TIMER)
        {
          //          ::SleepEx(0, TRUE);
          if (state.should_refresh())
          {
            state.refresh_page(window);
          }
        }

        if (message == TDN_NAVIGATED)
        {
          for (auto button : state.get_enabled_buttons())
          {
            ::SendMessageW(window, TDM_ENABLE_BUTTON, button, 1);
          }

          for (auto button : state.get_disabled_buttons())
          {
            ::SendMessageW(window, TDM_ENABLE_BUTTON, button, 0);
          }
          ::SendMessageW(window, TDM_SET_PROGRESS_BAR_MARQUEE, state.is_processing() ? TRUE : FALSE, 0);
        }

        return S_FALSE;
      });
  }


  return 0;
}