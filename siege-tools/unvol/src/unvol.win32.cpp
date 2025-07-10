#include <SDKDDKVer.h>
#include <filesystem>
#include <utility>
#include <span>
#include <optional>
#include <string_view>
#include <ranges>
#include <set>
#include <string>
#include <functional>
#include <spanstream>
#include <siege/platform/win/module.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/platform/win/common_controls.hpp>

#pragma comment(linker, \
  "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace fs = std::filesystem;
namespace stl = std::ranges;

struct command_line_args
{
  fs::path app_path;
  std::optional<fs::path> vol_path;
  std::optional<fs::path> output_path;
  bool should_use_ui;
};

command_line_args parse_command_line(std::span<std::string_view> args);
bool has_embedded_output_path();
fs::path get_embedded_output_path();
bool has_embedded_file();

int start_ui_modal(command_line_args args, std::function<void(command_line_args)> action, std::atomic_bool& should_cancel)
{
  win32::set_process_dpi_awareness(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
  win32::com::init_com();
  win32::init_common_controls_ex({ .dwSize{ sizeof(INITCOMMONCONTROLSEX) }, .dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES });

  thread_local std::map<int, std::wstring> string_table;
  std::vector<TASKDIALOG_BUTTON> buttons = {};

  for (auto i = 'A'; args.output_path && i <= 'Z'; ++i)
  {
    std::wstring drive;
    drive.append(1, (wchar_t)i);
    drive.append(1, L':');
    drive.append(1, L'\\');

    if (auto type = ::GetDriveTypeW(drive.c_str()); type == DRIVE_REMOVABLE || type == DRIVE_FIXED)
    {
      auto new_path = drive / args.output_path->relative_path();
      auto new_string = string_table.emplace(20 + buttons.size(), new_path.wstring());
      auto& new_button = buttons.emplace_back();
      new_button.pszButtonText = new_string.first->second.c_str();
      new_button.nButtonID = new_string.first->first;
    }
  }

  auto& new_button = buttons.emplace_back();
  new_button.pszButtonText = L"Custom path...";

  const static auto instruction = has_embedded_file() ? L"Pick a destination to extract the files in this self-extracting archive"
                                                      : L"Pick a destination to extract the files in the selected archive";

  std::atomic_bool is_done = true;

  auto result = win32::task_dialog_indirect({
                                              .cbSize = sizeof(TASKDIALOGCONFIG),
                                              .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS,
                                              .dwCommonButtons = TDCBF_CANCEL_BUTTON,
                                              .pszWindowTitle = L"Extract contents",
                                              .pszMainInstruction = instruction,
                                              .cButtons = (UINT)buttons.size(),
                                              .pButtons = buttons.data(),

                                            },
    [&](auto window, auto message, auto wparam, auto lparam) -> LRESULT {
      if (message == TDN_BUTTON_CLICKED && (wparam == IDCANCEL || wparam == IDCLOSE))
      {
        should_cancel = true;
        return S_OK;
      }

      if (message == TDN_BUTTON_CLICKED && string_table.contains(wparam))
      {
        args.output_path = string_table[wparam];
        win32::queue_user_work_item([window, action = std::move(action), args, &should_cancel, &is_done]() {
          std::shared_ptr<void> deferred(nullptr, [&](...) { is_done = true; });
          try
          {
            is_done = false;
            action(args);

            if (should_cancel)
            {
              return;
            }
            auto content = L"Extracted files to " + args.output_path->wstring();

            std::vector<TASKDIALOG_BUTTON> buttons = {};
            buttons.emplace_back(TASKDIALOG_BUTTON{
              .nButtonID = 30,
              .pszButtonText = L"Open output folder",
            });

            auto config = TASKDIALOGCONFIG{
              .cbSize = sizeof(TASKDIALOGCONFIG),
              .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION,
              .dwCommonButtons = TDCBF_CLOSE_BUTTON,
              .pszWindowTitle = L"Extracted file contents",
              .pszMainInstruction = instruction,
              .pszContent = content.c_str(),
              .cButtons = (UINT)buttons.size(),
              .pButtons = buttons.data()
            };

            ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);
          }
          catch (...)
          {
            auto config = TASKDIALOGCONFIG{
              .cbSize = sizeof(TASKDIALOGCONFIG),
              .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION,
              .dwCommonButtons = TDCBF_RETRY_BUTTON | TDCBF_CLOSE_BUTTON,
              .pszWindowTitle = L"Could not extract file contents",
              .pszMainInstruction = instruction,
              .pszContent = L""
            };
            ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);
          }
        });

        auto config = TASKDIALOGCONFIG{
          .cbSize = sizeof(TASKDIALOGCONFIG),
          .dwFlags = TDF_SHOW_MARQUEE_PROGRESS_BAR,
          .dwCommonButtons = TDCBF_CANCEL_BUTTON,
          .pszWindowTitle = L"Extracting contents",
          .pszMainInstruction = instruction,
          .pszContent = L"Extracting file contents"
        };

        ::SendMessageW(window, TDM_NAVIGATE_PAGE, 0, (LPARAM)&config);
      }

      if (message == TDN_BUTTON_CLICKED && wparam == 30)
      {
        ::ShellExecuteW(NULL, L"open", args.output_path->c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return S_OK;
      }

      if (message == TDN_NAVIGATED)
      {
        ::SendMessageW(window, TDM_SET_MARQUEE_PROGRESS_BAR, TRUE, 0);
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

  return 0;
}

command_line_args parse_command_line(std::span<std::string_view> args)
{
  auto app_arg = fs::path(win32::module_ref().GetModuleFileName<wchar_t>());
  std::optional<fs::path> file_arg;
  std::optional<fs::path> output_path;

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

    auto output = stl::find_if(args, [](auto& arg) {
      return arg.starts_with("--output") || arg.starts_with("-o");
    });

    if (output != args.end())
    {
      if (output->contains("="))
      {
        output_path = output->substr(output->find("=") + 1);
      }
      else
      {
        auto next = output;
        std::advance(next, 1);

        if (next != args.end())
        {
          output_path = *next;
        }
      }
    }
  }

  if (!output_path && has_embedded_output_path())
  {
    output_path = get_embedded_output_path();
  }

  if (!output_path && file_arg)
  {
    output_path = file_arg->parent_path() / file_arg->stem();
  }

  if (!output_path && use_ui && has_embedded_file())
  {
    auto exe_path = fs::path(win32::module_ref().GetModuleFileName<wchar_t>());
    output_path = has_embedded_file() ? exe_path.parent_path() / exe_path.stem() : exe_path.parent_path();
  }

  return { app_arg, file_arg, output_path, use_ui };
}

bool has_embedded_file()
{
  if (auto resource = ::FindResourceW(nullptr, L"embedded", RT_RCDATA); resource != nullptr)
  {
    return ::SizeofResource(nullptr, resource) > 0;
  }

  return false;
}

std::unique_ptr<std::istream> create_stream_for_embedded_file()
{
  auto resource = ::FindResourceW(nullptr, L"embedded", RT_RCDATA);

  if (!resource)
  {
    return nullptr;
  }

  auto handle = ::LoadResource(nullptr, resource);

  if (!handle)
  {
    return nullptr;
  }

  auto ptr = ::LockResource(handle);
  auto size = ::SizeofResource(nullptr, resource);

  auto span = std::span<char>((char*)ptr, size);

  return std::unique_ptr<std::istream>(new std::ispanstream(span));
}

bool has_embedded_output_path()
{
  if (auto resource = ::FindResourceW(nullptr, L"output_path", RT_RCDATA); resource != nullptr)
  {
    return ::SizeofResource(nullptr, resource) > 0;
  }

  return false;
}

fs::path get_embedded_output_path()
{
  auto resource = ::FindResourceW(nullptr, L"output_path", RT_RCDATA);

  if (!resource)
  {
    return fs::path{};
  }

  auto handle = ::LoadResource(nullptr, resource);

  if (!handle)
  {
    return fs::path{};
  }

  auto ptr = ::LockResource(handle);
  auto size = ::SizeofResource(nullptr, resource);

  return std::string_view((char*)ptr, size);
}

bool has_embedded_post_extract_commands()
{
  if (auto resource = ::FindResourceW(nullptr, L"post_extract", RT_RCDATA); resource != nullptr)
  {
    return ::SizeofResource(nullptr, resource) > 0;
  }

  return false;
}

std::vector<std::string> get_embedded_post_extract_commands()
{
  auto resource = ::FindResourceW(nullptr, L"post_extract", RT_RCDATA);

  if (!resource)
  {
    return {};
  }

  auto handle = ::LoadResource(nullptr, resource);

  if (!handle)
  {
    return {};
  }

  auto ptr = ::LockResource(handle);
  auto size = ::SizeofResource(nullptr, resource);

  std::string_view temp((char*)ptr, size);

  std::vector<std::string> results;

  std::size_t offset = 0;

  do
  {
    auto next = temp.find("\r\n");

    if (auto result = temp.substr(0, next); !result.empty())
    {
      results.emplace_back(result);
    }

    temp = next == std::string_view::npos ? std::string_view() : temp.substr(next + 2);
  } while (!temp.empty());

  return results;
}