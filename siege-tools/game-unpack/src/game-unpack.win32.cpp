#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
#include <siege/platform/installation_module.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/resource/resource_maker.hpp>
#include <siege/platform/http.hpp>
#include <span>
#include <string_view>
#include <array>
#include <ranges>
#include <map>
#include <set>
#include <fstream>
#include <iostream>
#include <regex>
#include <future>
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

struct transformed_path_rule : siege::platform::path_rule
{
  std::wstring resolved_source;
};

struct discovery_context
{
  siege::platform::installation_module module;
  std::vector<transformed_path_rule> transformed_rules;
};

struct command_line_args
{
  fs::path app_path;
  std::optional<fs::path> backup_path;
  std::optional<fs::path> output_path;
  bool should_use_ui;
  HWND hwnd_to_contact = HWND_BROADCAST;
};

command_line_args parse_command_line(std::span<std::string_view> args);

void exec_on_thread(DWORD thread_id, std::move_only_function<void()> callback)
{
  struct handler
  {
    static void CALLBACK user_apc(ULONG_PTR arg)
    {
      std::unique_ptr<std::move_only_function<void()>> callback_context{ (std::move_only_function<void()>*)arg };

      callback_context->operator()();
    }
  };

  auto handle = ::OpenThread(THREAD_SET_CONTEXT, FALSE, thread_id);

  assert(handle != nullptr);
  if (!handle)
  {
    return;
  }

  auto result = ::QueueUserAPC(handler::user_apc, handle, (ULONG_PTR) new std::move_only_function<void()>{ std::move(callback) });
  assert(result != 0);
}

using installation_variable = siege::platform::installation_variable;
using installation_option = siege::platform::installation_option;

struct user_interaction
{
  std::function<installation_option(installation_variable, std::span<installation_option>)> ask_for_variable;
  std::function<std::optional<fs::path>(std::vector<fs::path>)> ask_for_install_path;

  // TODO implement these correctly
  std::function<bool()> ask_to_download_cab_tooling;
  std::function<bool()> ask_to_do_generic_extract;
};

enum struct unpacking_status
{
  failed,
  cancelled,
  succeeded
};

unpacking_status do_unpacking(user_interaction ui, std::vector<fs::path> backup_paths, std::atomic_bool&);

int main(int argc, const char* argv[])
{
  static auto unpack_done_id = ::RegisterWindowMessageW(L"SIEGE_UNPACK_DONE");

  auto args = [=] {
    std::vector<std::string_view> temp(argv, argv + argc);
    return parse_command_line(temp);
  }();

  static std::atomic_bool should_cancel = false;

  if (args.should_use_ui)
  {
    win32::set_process_dpi_awareness(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    win32::com::init_com();
    win32::init_common_controls_ex({ .dwSize{ sizeof(INITCOMMONCONTROLSEX) }, .dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES });
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);

    std::vector<TASKDIALOG_BUTTON> default_buttons = {
      TASKDIALOG_BUTTON{ .nButtonID = 10, .pszButtonText = L"Install Game From File" }
    };

    std::vector<fs::path> roots;
    std::wstring drive = L"A://";
    for (auto i = L'A'; i <= L'Z'; ++i)
    {
      drive[0] = i;

      if (auto type = ::GetDriveTypeW(drive.c_str()); type == DRIVE_REMOVABLE || type == DRIVE_CDROM)
      {
        roots.emplace_back(drive);
      }
    }

    if (!roots.empty())
    {
      default_buttons.emplace_back(TASKDIALOG_BUTTON{ .nButtonID = 11, .pszButtonText = L"Detect Game From Storage" });
    }

    std::vector<fs::path> backup_paths = roots;

    constexpr static auto window_title = L"Siege Game Unpacker";
    std::function<HRESULT(int)> button_callback;
    static std::atomic_bool is_done = true;

    auto result = win32::task_dialog_indirect({
                                                .cbSize = sizeof(TASKDIALOGCONFIG),
                                                .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS,
                                                .dwCommonButtons = TDCBF_CLOSE_BUTTON,
                                                .pszWindowTitle = window_title,
                                                .pszMainInstruction = L"To install a game from a back-up, select an option below: ",
                                                .cButtons = (UINT)default_buttons.size(),
                                                .pButtons = default_buttons.data(),

                                              },
      [&](auto window, auto message, auto wparam, auto lparam) -> LRESULT {
        if (message == TDN_BUTTON_CLICKED && (wparam == IDCANCEL || wparam == IDCLOSE))
        {
          should_cancel = true;
          return S_OK;
        }

        if (message == TDN_BUTTON_CLICKED && button_callback)
        {
          return button_callback((int)wparam);
        }

        if (message == TDN_BUTTON_CLICKED && wparam == 11)
        {
          backup_paths = roots;
        }

        if (message == TDN_BUTTON_CLICKED && wparam == 10)
        {
          auto new_path = win32::get_path_via_file_dialog({});

          if (!new_path)
          {
            return S_FALSE;
          }

          backup_paths.clear();
          backup_paths.emplace_back(std::move(*new_path));
        }

        if (message == TDN_BUTTON_CLICKED && (wparam == 10 || wparam == 11))
        {
          using namespace siege::platform;

          auto ui_thread_id = ::GetCurrentThreadId();
          win32::queue_user_work_item([ui_thread_id, window, backup_paths, &button_callback, &args] {
            user_interaction ui{
              .ask_for_variable = [ui_thread_id, window, &button_callback](auto variable, auto options) {
                    std::promise<installation_option> result{};
                          exec_on_thread(ui_thread_id, [window, variable, options, &result, &button_callback]() {
                      std::vector<TASKDIALOG_BUTTON> new_buttons;

                      int index = 31;
                      for (auto& option : options)
                      {
                        new_buttons.emplace_back(TASKDIALOG_BUTTON{ .nButtonID = index++, .pszButtonText = option.label });
                      }
                      
                      std::wstring instruction = L"Select an option for " + std::wstring(variable.label) + L":";
                      TASKDIALOGCONFIG config{
                              .cbSize = sizeof(TASKDIALOGCONFIG),
                              .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS,
                              .dwCommonButtons = TDCBF_CLOSE_BUTTON,
                              .pszWindowTitle = window_title,
                              .pszMainInstruction = instruction.c_str(),
                              .cButtons = (UINT)new_buttons.size(),
                              .pButtons = new_buttons.data(),
                            };
                            ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);

                      button_callback = [options, window, &result, &button_callback](int button_id) -> HRESULT {
                        if (button_id > 30)
                        {
                          try
                          {
                            result.set_value(options[button_id - 30 - 1]);
                            TASKDIALOGCONFIG config{
                              .cbSize = sizeof(TASKDIALOGCONFIG),
                              .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_ALLOW_DIALOG_CANCELLATION | TDF_SHOW_MARQUEE_PROGRESS_BAR | TDF_CALLBACK_TIMER,
                              .dwCommonButtons = TDCBF_CLOSE_BUTTON,
                              .pszWindowTitle = window_title,
                              .pszMainInstruction = L"Continuing with installation"
                            };
                            ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);
                          }
                          catch (...)
                          {
                            result.set_value(options[0]);
                          }
                        }
                        button_callback = nullptr;

                        return S_FALSE;
                      };

                          });

                    return result.get_future().get(); },
              .ask_for_install_path = [ui_thread_id, window, &button_callback](auto hints) {
                      std::promise<std::optional<fs::path>> result{};
                      exec_on_thread(ui_thread_id, [window, hints, &result, &button_callback]() {
                        std::vector<TASKDIALOG_BUTTON> new_buttons;
                        int index = 21;
                        for (auto& hint : hints)
                        {
                          new_buttons.emplace_back(TASKDIALOG_BUTTON{ .nButtonID = index++, .pszButtonText = hint.c_str() });
                        }

                        new_buttons.emplace_back(TASKDIALOG_BUTTON{ .nButtonID = 20, .pszButtonText = L"Select a custom directory..." });

                         TASKDIALOGCONFIG config{
                          .cbSize = sizeof(TASKDIALOGCONFIG),
                          .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS,
                          .dwCommonButtons = TDCBF_CLOSE_BUTTON,
                          .pszWindowTitle = window_title,
                          .pszMainInstruction = L"Select your preferred install path below:",
                          .cButtons = (UINT)new_buttons.size(),
                          .pButtons = new_buttons.data(),
                        };
                        ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);

                        button_callback = [hints, window, &result, &button_callback](int button_id) mutable -> HRESULT {
                          if (button_id == 20)
                          {
                            auto new_path = win32::get_path_via_file_dialog({
                              .Flags = FOS_PICKFOLDERS,
                            });

                            if (new_path)
                            {
                              hints.emplace_back(std::move(*new_path));
                              button_id = 20 + hints.size();
                              // fallthrough to the next if statement
                            }
                          }

                          if (button_id > 20)
                          {
                            try
                            {
                              result.set_value(hints.at(button_id - 20 - 1));
                              TASKDIALOGCONFIG config{
                                .cbSize = sizeof(TASKDIALOGCONFIG),
                                .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_SHOW_MARQUEE_PROGRESS_BAR | TDF_CALLBACK_TIMER,
                                .dwCommonButtons = TDCBF_CLOSE_BUTTON,
                                .pszWindowTitle = window_title,
                                .pszMainInstruction = L"Installing game files to path"
                              };
                              ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);
                            }
                            catch (...)
                            {
                              result.set_value(std::nullopt);
                            }
                            button_callback = nullptr;
                          }

                          return S_FALSE;
                        };
                      });

                      return result.get_future().get(); },
              .ask_to_download_cab_tooling = []() { return true; }
            };
            std::shared_ptr<void> deferred{ nullptr, [](...) { is_done = true; } };
            is_done = false;
            auto result = do_unpacking(ui, backup_paths, should_cancel);
            if (result == unpacking_status::succeeded)
            {
              ::SendNotifyMessageW(args.hwnd_to_contact, unpack_done_id, 0, 0);
              exec_on_thread(ui_thread_id, [window]() {
                TASKDIALOGCONFIG config{
                  .cbSize = sizeof(TASKDIALOGCONFIG),
                  .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION,
                  .dwCommonButtons = TDCBF_CLOSE_BUTTON,
                  .pszWindowTitle = window_title,
                  .pszMainInstruction = L"Installed game succesfully"
                };
                ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);
              });
            }
            else
            {
              exec_on_thread(ui_thread_id, [window]() {
                TASKDIALOGCONFIG config{
                  .cbSize = sizeof(TASKDIALOGCONFIG),
                  .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION,
                  .dwCommonButtons = TDCBF_CLOSE_BUTTON,
                  .pszWindowTitle = window_title,
                  .pszMainInstruction = L"Could not install game."
                };
                ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);
              });
            }
          });

          TASKDIALOGCONFIG config{
            .cbSize = sizeof(TASKDIALOGCONFIG),
            .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_SHOW_MARQUEE_PROGRESS_BAR | TDF_CALLBACK_TIMER,
            .dwCommonButtons = TDCBF_CLOSE_BUTTON,
            .pszWindowTitle = window_title,
            .pszMainInstruction = L"Detecting game to install, please wait"
          };
          ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);
        }

        if (message == TDN_TIMER)
        {
          ::SleepEx(0, TRUE);
        }

        if (message == TDN_NAVIGATED)
        {
          ::SendMessageW(window, TDM_SET_PROGRESS_BAR_MARQUEE, TRUE, 0);
        }

        return S_FALSE;
      });

    for (auto i = 0; i < 1000; ++i)
    {
      if (is_done)
      {
        break;
      }
      ::Sleep(10);
    }
  }
  else if (args.backup_path)
  {
    user_interaction console_ui{
      .ask_for_variable = [](auto variable, auto options) { 
        std::wcout << L"Setting " << variable.label << L" to " << options[0].label << std::endl;
            
        return options[0]; },
      .ask_for_install_path = [](auto hints) -> std::optional<fs::path> {
        if (!hints.empty())
        {
          std::cout << "Installing to " << hints[0] << std::endl;
          return hints[0];
        }
        return std::nullopt;
      },
      .ask_to_download_cab_tooling = []() { return true; }
    };

    std::cout << "Detecting game for " << *args.backup_path << std::endl;
    auto status = do_unpacking(console_ui, { *args.backup_path }, should_cancel);

    if (status == unpacking_status::succeeded)
    {
      ::SendNotifyMessageW(args.hwnd_to_contact, unpack_done_id, 0, 0);
      std::cout << "Unpacked game successfully" << std::endl;
    }
    else
    {
      std::cout << "Could not unpack game" << std::endl;
    }
  }
  else if (!args.should_use_ui)
  {
    std::cout << "Please specify at least one argument for the file or folder which contains a game backup." << std::endl;
  }

  return 0;
}

unpacking_status do_unpacking(user_interaction ui, std::vector<fs::path> backup_paths, std::atomic_bool& should_cancel)
{
  using file_info = siege::platform::file_info;
  using listing_query = siege::platform::listing_query;
  using resource_reader = siege::platform::resource_reader;

  if (stl::all_of(backup_paths, [](auto& item) { return !fs::is_directory(item); }) && !stl::all_of(backup_paths, [](auto& item) { 
          std::ifstream temp(item, std::ios::binary);
          return siege::resource::is_resource_readable(temp); }))
  {
    return unpacking_status::failed;
  }

  auto installation_modules = siege::platform::installation_module::load_modules(fs::current_path());

  std::list<installable_file> backup_files;
  std::map<std::wstring, std::wstring> resolved_variables;
  std::optional<discovery_context> discovered_info;

  std::array<wchar_t, 32> system_drive{ L"C:" };
  ::GetEnvironmentVariableW(L"SYSTEMDRIVE", system_drive.data(), (DWORD)system_drive.size());

  for (auto& module : installation_modules)
  {
    if (should_cancel)
    {
      break;
    }
    backup_files.clear();

    resolved_variables = std::map<std::wstring, std::wstring>{
      { L"systemDrive", system_drive.data() }
    };

    std::set<std::wstring> names_to_check;

    bool check_name_variations = true;

    int disk_count = 0;

    if (module.storage_properties)
    {
      for (auto* name : module.storage_properties->disc_names)
      {
        if (!name)
        {
          break;
        }

        disk_count++;
        names_to_check.emplace(name);
      }

      check_name_variations = disk_count == 0 || disk_count == 1;
    }

    if (check_name_variations)
    {
      for (auto name : module.name_variations)
      {
        names_to_check.emplace(name);
      }
    }

    auto has_valid_names = stl::all_of(backup_paths, [&names_to_check](fs::path& path) {
      if (path.root_path() == path)
      {
        std::array<wchar_t, 256> volume_name{};
        if (::GetVolumeInformationW(path.c_str(),
              volume_name.data(),
              (DWORD)volume_name.size(),
              nullptr,
              nullptr,
              nullptr,
              nullptr,
              0))
        {
          return stl::any_of(names_to_check, [&](auto& item) {
            return item == volume_name.data() || siege::platform::to_lower(item) == siege::platform::to_lower(volume_name.data());
          });
        }
        return false;
      }

      auto stem = siege::platform::to_lower(path.stem().wstring());
      return stl::any_of(names_to_check, [&](auto& item) {
        return siege::platform::to_lower(item) == stem;
      });
    });

    if (!has_valid_names)
    {
      continue;
    }

    std::vector<transformed_path_rule> transformed_rules;
    transformed_rules.reserve(module.directory_mappings.size());

    for (auto& mapping : module.directory_mappings)
    {
      auto& new_rule = transformed_rules.emplace_back(mapping);
      new_rule.resolved_source = new_rule.source;
    }

    if (disk_count > 1 && backup_paths.size() == 1)
    {
      std::set<std::wstring_view> disc_names;

      for (auto* name : module.storage_properties->disc_names)
      {
        if (!name)
        {
          break;
        }
        disc_names.emplace(name);
      }

      auto parent_folder = backup_paths.front().parent_path();
      auto stem = backup_paths.front().stem();

      std::multimap<std::wstring, fs::path> additional_paths;

      for (auto& other : std::filesystem::directory_iterator{ parent_folder })
      {
        if (other.path().stem().wstring() == stem)
        {
          continue;
        }

        if (other.is_regular_file() && disc_names.contains(other.path().stem().wstring()))
        {
          std::ifstream temp(other.path(), std::ios::binary);

          if (!siege::resource::is_resource_readable(temp))
          {
            continue;
          }
          additional_paths.emplace(other.path().stem().wstring(), other.path());
        }
      }

      if (additional_paths.empty())
      {
        continue;
      }

      for (auto& name : disc_names)
      {
        if (name == stem)
        {
          auto matching_rules = std::views::filter(transformed_rules, [&](transformed_path_rule& rule) {
            return rule.resolved_source.starts_with(name);
          });

          for (auto& rule : matching_rules)
          {
            rule.resolved_source = std::regex_replace(rule.resolved_source, std::wregex(std::wstring(name)), backup_paths.front().filename().wstring());
          }
          continue;
        }
        auto found_items = additional_paths.equal_range(std::wstring(name));

        // TODO try the first file with the same extension, then try other extensions
        auto first_alt = found_items.first;

        auto matching_rules = std::views::filter(transformed_rules, [&](transformed_path_rule& rule) {
          return rule.resolved_source.starts_with(name);
        });

        backup_paths.emplace_back(first_alt->second);
        for (auto& rule : matching_rules)
        {
          rule.resolved_source = std::regex_replace(rule.resolved_source, std::wregex(std::wstring(name)), first_alt->second.filename().wstring());
        }
      }
    }
    else if (disk_count > 1 && backup_paths.size() > 1)
    {
      for (auto& path : backup_paths)
      {
        std::array<wchar_t, 256> volume_name{};
        if (::GetVolumeInformationW(path.c_str(),
              volume_name.data(),
              (DWORD)volume_name.size(),
              nullptr,
              nullptr,
              nullptr,
              nullptr,
              0))
        {
          // TODO rework the logic to make name checking uniform.
          // At the moment it's lower case for filenames, upper case for volume names
          // and exact case for multi volume checks with multiple files.
          auto matching_rules = std::views::filter(transformed_rules, [&](transformed_path_rule& rule) {
            return rule.resolved_source.starts_with(siege::platform::to_upper(volume_name.data()));
          });
          for (auto& rule : matching_rules)
          {
            rule.resolved_source = std::regex_replace(rule.resolved_source, std::wregex(siege::platform::to_upper(volume_name.data())), path.c_str());
          }
        }
      }
    }

    std::set<fs::path> verification_mappings;
    std::multimap<fs::path, fs::path> child_verification_mappings;

    bool requires_cab_tooling = false;

    for (auto& rule : transformed_rules)
    {
      if (should_cancel)
      {
        return unpacking_status::cancelled;
      }

      if (rule.enforcement == rule.optional)
      {
        continue;
      }

      auto full_path = rule.resolved_source;
      auto stem = fs::path(rule.resolved_source).stem().wstring();

      if (stem.starts_with(L"<") && stem.ends_with(L">"))
      {
        continue;
      }

      auto temp = fs::path(full_path);

      if (temp.parent_path().has_extension() || stem.contains(L"*"))
      {
        verification_mappings.emplace(temp.parent_path());

        child_verification_mappings.emplace(temp.parent_path(), temp);
        if (!requires_cab_tooling && (temp.parent_path().extension() == ".cab" || temp.parent_path().extension() == ".CAB"))
        {
          requires_cab_tooling = true;
        }

        continue;
      }

      verification_mappings.emplace(temp);
    }

    auto external_path = fs::path(win32::module_ref::current_application().GetModuleFileName()).parent_path() / "external";
    std::error_code last_error;

    struct external_app
    {
      const wchar_t* exe_name;
      const wchar_t* remote_name;
    };
    std::array<external_app, 3> external_apps{ { { L"I5comp.exe", L"external/i5comp21.zip" },
      { L"i6comp.exe", L"external/i6cmp13b.zip" },
      { L"7zr.exe", L"external/7zr.zip" } } };

    auto has_cab_extractors =
      stl::all_of(external_apps, [&](auto& app) {
        return fs::exists(external_path / fs::path(app.remote_name).stem() / app.exe_name, last_error);
      });

    if (has_cab_extractors)
    {
      for (auto& app : external_apps)
      {
        auto temp = external_path / fs::path(app.remote_name).stem();
        win32::add_dll_directory(temp.c_str());
      }
    }

    if (!has_cab_extractors && requires_cab_tooling && ui.ask_to_download_cab_tooling())
    {
      for (auto domain : { L"updates.thesiegehub.com" })
      {
        siege::platform::http_client_context context;

        for (auto& app : external_apps)
        {
          std::stringstream content;
          auto downloaded = siege::platform::download_http_data(context, domain, app.remote_name, content);

          if (!downloaded)
          {
            continue;
          }

          if (!siege::resource::is_resource_readable(content))
          {
            continue;
          }

          std::any cache;
          auto reader = siege::resource::make_resource_reader(content);

          auto files = reader.get_all_files_for_query(cache, content, {});

          auto new_path = external_path / fs::path(app.remote_name).stem();
          fs::create_directories(new_path, last_error);
          for (const auto& file : files)
          {
            std::ofstream output(new_path / file.filename, std::ios::binary);
            reader.extract_file_contents(cache, content, file, output);
          }
        }
      }
    }

    std::any cab_cache{};

    for (auto& backup_path : backup_paths)
    {
      if (should_cancel)
      {
        return unpacking_status::cancelled;
      }

      if (fs::is_directory(backup_path))
      {
        for (const fs::directory_entry& dir_entry :
          fs::recursive_directory_iterator(backup_path))
        {
          if (dir_entry.is_directory())
          {
            continue;
          }
          auto& new_item = backup_files.emplace_back();
          new_item.filename = dir_entry.path().filename();
          new_item.folder_path = dir_entry.path().parent_path();
          new_item.archive_path = backup_path;
          new_item.compression_type = siege::platform::compression_type::none;
          new_item.size = dir_entry.file_size();
          new_item.install_to = [new_item = &new_item](auto install_path, auto install_segment) mutable {
            fs::create_directories(install_path / install_segment);
            auto final_path = install_path / install_segment / new_item->filename;
            fs::copy(new_item->folder_path / new_item->filename, final_path, fs::copy_options::overwrite_existing);
          };
        }
      }
      else
      {
        auto game_backup = std::make_shared<std::ifstream>(backup_path, std::ios::binary);

        auto reader = siege::resource::make_resource_reader(*game_backup);
        std::any cache{};

        for (auto& file : reader.get_all_files_for_query(cache, *game_backup, listing_query{ .archive_path = backup_path, .folder_path = backup_path }))
        {
          auto& new_item = backup_files.emplace_back(file);
          new_item.install_to = [new_item = &new_item, reader, cache, game_backup](auto install_path, auto install_segment) mutable {
            fs::create_directories(install_path / install_segment);
            auto final_path = install_path / install_segment / new_item->filename;
            std::ofstream temp_out_buffer(final_path, std::ios::binary | std::ios::trunc);
            reader.extract_file_contents(cache, *game_backup, *new_item, temp_out_buffer);
          };
        }
      }
    }

    // first level verification
    if (stl::all_of(verification_mappings, [&](auto& mapping) {
          return stl::any_of(backup_files, [&](auto& item) {
            if (item.archive_path.has_filename() && mapping.wstring().starts_with(item.archive_path.filename().wstring()))
            {
              auto relative_path = item.archive_path.filename() / item.relative_path();
              return mapping == relative_path || mapping == relative_path / item.filename;
            }

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

      std::list<installable_file> inner_backup_files;

      if (!is_verified)
      {
        for (auto& backup_file : backup_files)
        {
          if (should_cancel)
          {
            return unpacking_status::cancelled;
          }
          auto should_extract = stl::all_of(backup_paths, [&](auto& path) { return verification_mappings.contains(path); }) || backup_file.filename.extension() == ".cab" || backup_file.filename.extension() == ".CAB" || backup_file.filename.extension() == ".hdr" || backup_file.filename.extension() == ".HDR";

          if (!should_extract)
          {
            continue;
          }

          backup_file.install_to(staging_path, backup_file.relative_path().c_str());
        }

        for (auto& backup_file : backup_files)
        {
          if (should_cancel)
          {
            return unpacking_status::cancelled;
          }
          auto backup_path = backup_file.relative_path() / backup_file.filename;
          auto backup_path_with_archive = backup_file.archive_path.filename() / backup_file.relative_path() / backup_file.filename;

          if (!(verification_mappings.contains(backup_path) || verification_mappings.contains(backup_path_with_archive)))
          {
            continue;
          }

          auto final_path = staging_path / backup_path;

          auto inner_backup = std::make_shared<std::ifstream>(final_path, std::ios::binary);

          if (!siege::resource::is_resource_readable(*inner_backup))
          {
            continue;
          }

          auto inner_reader = siege::resource::make_resource_reader(*inner_backup);
          auto inner_cache = std::make_shared<std::any>();

          auto inner_files = inner_reader.get_all_files_for_query(*inner_cache, *inner_backup, listing_query{ .archive_path = final_path, .folder_path = final_path });

          auto [first, end] = child_verification_mappings.equal_range(backup_path);

          auto all_valid = std::all_of(first, end, [&](auto& verification_name) { return stl::any_of(inner_files, [&](file_info& child) {
                                                                                    if (std::wstring_view(verification_name.second.c_str()).contains(L"*"))
                                                                                    {
                                                                                      return verification_name.first == fs::relative(child.folder_path, staging_path);
                                                                                    }
                                                                                    return verification_name.second == (fs::relative(child.folder_path, staging_path) / child.filename);
                                                                                  }); });

          is_verified = is_verified || all_valid;

          if (is_verified)
          {
            for (auto& file : inner_files)
            {
              auto& new_item = inner_backup_files.emplace_back(file);
              new_item.install_to = [new_item = &new_item, inner_reader, inner_cache, inner_backup](auto install_path, auto install_segment) mutable {
                fs::create_directories(install_path / install_segment / new_item->relative_path());
                auto final_path = install_path / install_segment / new_item->relative_path() / new_item->filename;
                std::ofstream temp_out_buffer(final_path, std::ios::binary | std::ios::trunc);
                inner_reader.extract_file_contents(*inner_cache, *inner_backup, *new_item, temp_out_buffer);
              };
            }
          }
        }
      }

      backup_files.splice(backup_files.end(), std::move(inner_backup_files));

      if (!backup_files.empty())
      {
        discovered_info.emplace(std::move(module), std::move(transformed_rules));
        break;
      }
    }
  }

  if (should_cancel)
  {
    return unpacking_status::cancelled;
  }

  if (!discovered_info)
  {
    // TODO ask user if they want to do a generic extract
    return unpacking_status::failed;
  }

  std::vector<fs::path> install_path_hints{};

  if (discovered_info->module.storage_properties && discovered_info->module.storage_properties->default_install_path[0] != '\0')
  {
    auto install_path = std::wstring(discovered_info->module.storage_properties->default_install_path.data());

    install_path_hints.emplace_back(std::regex_replace(install_path, std::wregex(L"<systemDrive>"), resolved_variables.at(L"systemDrive")));

    for (auto i = 'A'; i <= 'Z'; ++i)
    {
      if (resolved_variables.at(L"systemDrive").at(0) == (wchar_t)i)
      {
        continue;
      }

      std::wstring drive;
      drive.append(1, (wchar_t)i);
      drive.append(1, L':');
      drive.append(1, L'\\');

      if (auto type = ::GetDriveTypeW(drive.c_str()); type == DRIVE_REMOVABLE || type == DRIVE_FIXED)
      {
        drive.pop_back();
        install_path_hints.emplace_back(std::regex_replace(install_path, std::wregex(L"<systemDrive>"), drive));
      }
    }
  }

  if (should_cancel)
  {
    return unpacking_status::cancelled;
  }
  auto install_path = ui.ask_for_install_path(install_path_hints);

  if (!install_path)
  {
    return unpacking_status::failed;
  }

  fs::create_directories(*install_path);

  for (auto& variable : discovered_info->module.installation_variables)
  {
    if (should_cancel)
    {
      return unpacking_status::cancelled;
    }
    // TODO ask the user to select the option
    // Though the first option should be the most likely by default.
    auto options = discovered_info->module.get_options_for_variable(variable.name);

    if (!options.empty())
    {
      auto selected_option = ui.ask_for_variable(variable, options);
      resolved_variables.emplace(variable.name, selected_option.name);
    }
  }


  for (auto& mapping : discovered_info->transformed_rules)
  {
    std::wstring source(mapping.resolved_source);

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

        if (source.begin()->has_extension() && !full_path_str.starts_with(source.begin()->filename().wstring()))
        {
          source_str = source_str.substr(source.begin()->filename().wstring().size());
        }

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
    using namespace std::literals;
    for (auto& item : files_to_copy)
    {
      if (should_cancel)
      {
        return unpacking_status::cancelled;
      }

      if (mapping.destination == L"="sv)
      {
        item.install_to(*install_path, item.relative_path().wstring());
      }
      else
      {
        item.install_to(*install_path, mapping.destination);
      }
    }
  }

  if (discovered_info->module.apply_post_install_steps_proc)
  {
    fs::current_path(*install_path);

    // TODO report the correct status. Partially installed or failed depending on the severity
    discovered_info->module.apply_post_install_steps_proc();
  }

  return unpacking_status::succeeded;
}

command_line_args parse_command_line(std::span<std::string_view> args)
{
  auto app_arg = fs::path(win32::module_ref().GetModuleFileName<wchar_t>());
  std::optional<fs::path> file_arg;
  std::optional<fs::path> output_path;
  std::string window;

  std::array<DWORD, 4> ids{};

  bool use_ui = ::GetConsoleProcessList(ids.data(), (DWORD)ids.size()) == 1;

  if (!args.empty())
  {
    if (!use_ui)
    {
      use_ui = std::any_of(args.begin(), args.end(), [](auto& arg) { return arg == "--force-ui"; });
    }

    auto temp = fs::path(args[0]);

    auto next_arg = 0;

    if (temp.has_extension() && (temp.extension() == ".exe" || temp.extension() == ".EXE"))
    {
      app_arg = temp;
      next_arg = 1;
    }

    // in the very rare case you launch a process with the first argument not being the exe path
    // next_arg will be 0
    if (next_arg + 1 <= args.size() && fs::is_regular_file(args[next_arg]))
    {
      file_arg = args[next_arg];
    }

    auto parse_arg = [args](auto condition, auto& value) -> std::optional<std::string_view> {
      auto output = stl::find_if(args, condition);
      if (output != args.end())
      {
        if (output->contains("="))
        {
          value = output->substr(output->find("=") + 1);
        }
        else
        {
          auto next = output;
          std::advance(next, 1);

          if (next != args.end())
          {
            value = *next;
          }
        }
      }
      return std::nullopt;
    };

    parse_arg([](auto& arg) {
      return arg.starts_with("--output") || arg.starts_with("-o");
    },
      output_path);


    parse_arg([](auto& arg) {
      return arg.starts_with("--broadcast-hwnd");
    },
      window);
  }

  if (!output_path && file_arg)
  {
    output_path = file_arg->parent_path() / file_arg->stem();
  }

  HWND real_window = HWND_BROADCAST;

  try
  {
    real_window = (HWND)std::stoul(window);
  }
  catch (...)
  {
  }

  return { app_arg, file_arg, output_path, use_ui, real_window };
}
