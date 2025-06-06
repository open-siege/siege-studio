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
#include <fstream>
#include <filesystem>

#pragma comment(linker, \
  "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace fs = std::filesystem;

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
                win32::queue_user_work_item([ui_handle, window, path = *path, &state] {
                  std::ifstream file(path, std::ios::binary);

                  if (siege::resource::is_resource_reader(file))
                  {
                    auto installation_modules = siege::platform::installation_module::load_modules(fs::current_path());

                    auto stem = path.stem().wstring();

                    for (auto& module : installation_modules)
                    {
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
                      
                      if (std::ranges::any_of(names_to_check, [&](auto& item) {
                            return item == stem;
                          }))
                      {
                        auto mappings = module.directory_mappings;


                        std::set<fs::path> verification_mappings;
                        
                        for (auto& mapping : mappings)
                        {
                          fs::path temp(mapping.source);

                          auto full_path = temp.wstring();
                          auto stem = temp.stem().wstring();

                          if (stem.starts_with(L"<") && stem.ends_with(L">"))
                          {
                            continue;
                          }

                          if (full_path.contains(L"*"))
                          {
                            continue;
                          }

                          verification_mappings.emplace(temp);
                        }

                        auto reader = siege::resource::make_resource_reader(file);

                        std::list<resource_reader::content_info> contents;
                        std::any cache{};

                        auto top_level_items = reader->get_content_listing(cache, file, listing_query{ .archive_path = path, .folder_path = path });

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
                              children = reader->get_content_listing(cache, file, listing_query{ .archive_path = path, .folder_path = parent_info->full_path });
                              get_full_listing(children);
                            }

                            if (auto leaf_info = std::get_if<file_info>(&info); leaf_info)
                            {
                              contents.emplace_back(info);
                            }
                          }
                        };

                        get_full_listing(top_level_items);

                        for (auto& item : contents)
                        {

                        }
                      }
                    }
                  }

                  ::Sleep(10000);
                  exec_on_thread(ui_handle, [window, &state]() {
                    state.enable_page();
                    state.end_progress();
                    //state.navigate_page(window);
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