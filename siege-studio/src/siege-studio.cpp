#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
#include <array>
#include <optional>
#include <algorithm>
#include <vector>
#include <set>
#include <bit>
#include <variant>
#include <functional>
#include <fstream>
#undef NDEBUG
#include <cassert>

#include <siege/platform/win/shell.hpp>

#include <siege/platform/shared.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <imagehlp.h>

namespace fs = std::filesystem;
extern "C" __declspec(dllexport) std::uint32_t DisableSiegeExtensionModule = -1;
constexpr static std::wstring_view app_title = L"Siege Studio";
void load_core_module(fs::path load_path);
int register_and_create_main_window(DWORD window_thread_id, int nCmdShow);

struct discovery_info
{
  fs::path current_app_exe_path;
  fs::path launch_exe_path;
  std::optional<fs::path> matching_installed_exe_path;
  std::optional<fs::path> latest_installed_exe_path;
  std::optional<fs::path> extraction_target_path;
};

discovery_info discover_installation_info(int major_version, int minor_version, std::wstring channel);

struct embedded_module
{
  std::filesystem::path filename;
  HGLOBAL handle;
  WORD lang_id;
};

BOOL __stdcall enumerate_embedded_modules(HMODULE module,
  LPCWSTR type,
  LPWSTR name,
  LONG_PTR lParam)
{
  std::vector<embedded_module>* items = (std::vector<embedded_module>*)lParam;

  auto entry = ::FindResourceW(module, name, RT_RCDATA);

  if (!entry)
  {
    return TRUE;
  }

  auto data = ::LoadResource(module, entry);

  if (!data)
  {
    return TRUE;
  }

  fs::path path(name);

  if (path.extension() == ".dll" || path.extension() == ".DLL" || path.extension() == ".exe" || path.extension() == ".EXE")
  {
    items->emplace_back(embedded_module{ std::move(path), data });
  }

  return TRUE;
}

BOOL __stdcall enumerate_dll_languages(HMODULE hModule, const wchar_t* lpszType, const wchar_t* lpszName, WORD lang_id, LONG_PTR lParam)
{
  embedded_module* dll = (embedded_module*)lParam;

  if (!dll->lang_id)
  {
    dll->lang_id = lang_id;
    return FALSE;
  }
  return TRUE;
}

extern "C" ATOM register_windows(HMODULE this_module);
extern "C" BOOL deregister_windows(HMODULE this_module);

static std::optional<win32::window_module> core_module;
static auto* register_windows_ptr = &register_windows;
static auto* deregister_windows_ptr = &deregister_windows;
static ATOM main_atom = 0;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
  std::array<char, 256> data{};

  GetLogicalDriveStringsA(data.size(), data.data());

  win32::com::init_com();

  win32::init_common_controls_ex({ .dwSize{ sizeof(INITCOMMONCONTROLSEX) },
    .dwICC = ICC_STANDARD_CLASSES | ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_DATE_CLASSES | ICC_BAR_CLASSES });

  auto wic_version = win32::get_wic_version();

  if (!wic_version)
  {
    ::MessageBoxW(nullptr, L"Windows Imaging Component has not been detected. Siege Studio will not function correctly without it. If you are using an older version of Windows, make sure to either install the latest updates or the Windows Imaging Component redistributable.", L"Windows Imaging Component is missing", MB_OK | MB_ICONERROR);
  }

  win32::window_module_ref this_module(hInstance);

  std::vector<embedded_module> embedded_modules;
  embedded_modules.reserve(128);

  ::EnumResourceNamesW(hInstance, RT_RCDATA, enumerate_embedded_modules, (LONG_PTR)&embedded_modules);

  for (auto& dll : embedded_modules)
  {
    ::EnumResourceLanguagesW(hInstance, RT_RCDATA, dll.filename.c_str(), enumerate_dll_languages, (LONG_PTR)&dll);
  }

  // TODO get the preferred channel from the registry
  auto preferred_channel = L"any";

  auto install_info = discover_installation_info(SIEGE_MAJOR_VERSION, SIEGE_MINOR_VERSION, preferred_channel);

  std::error_code last_error;

  if (!embedded_modules.empty() && install_info.extraction_target_path)
  {
    auto install_path = *install_info.extraction_target_path;
    std::for_each(embedded_modules.begin(), embedded_modules.end(), [=](embedded_module& dll) {
      auto entry = ::FindResourceW(hInstance, dll.filename.c_str(), RT_RCDATA);

      if (!entry)
      {
        return;
      }

      std::error_code code;
      if (fs::exists(install_path / siege::platform::to_lower(dll.filename.wstring()), code))
      {
        return;
      }

      auto size = ::SizeofResource(hInstance, entry);
      auto bytes = ::LockResource(dll.handle);

      std::ofstream output(install_path / siege::platform::to_lower(dll.filename.wstring()), std::ios::trunc | std::ios::binary);
      output.write((const char*)bytes, size);
    });

    auto new_path = *install_info.extraction_target_path / install_info.current_app_exe_path.filename();
    fs::copy_file(install_info.current_app_exe_path, new_path, last_error);

    if (!last_error)
    {
      if (auto handle = ::BeginUpdateResourceW(new_path.c_str(), FALSE); handle)
      {
        for (auto& dll : embedded_modules)
        {
          ::UpdateResourceW(handle, RT_RCDATA, dll.filename.c_str(), dll.lang_id, nullptr, 0);
        }

        ::EndUpdateResourceW(handle, FALSE);
      }

      auto new_path_str = new_path.wstring();
      STARTUPINFO startup{};
      PROCESS_INFORMATION process{};

      auto result = (INT_PTR)::ShellExecuteW(NULL, L"open", new_path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
      if (result > SE_ERR_DLLNOTFOUND)
      {
        ::ExitProcess(0);
        return 0;
      }
    }
  }

  win32::add_dll_directory(install_info.launch_exe_path.parent_path().c_str());

  if (fs::is_directory(install_info.launch_exe_path.parent_path() / "external", last_error))
  {
    auto external = install_info.launch_exe_path.parent_path() / "external";
    win32::add_dll_directory(external.c_str());
  }

  load_core_module(install_info.launch_exe_path.parent_path());

  if (auto result = register_and_create_main_window(::GetCurrentThreadId(), nCmdShow); result != 0)
  {
    return result;
  }

  auto deferred = std::shared_ptr<void>(nullptr, [hInstance](...) { deregister_windows_ptr(hInstance); });

  MSG msg;

  while (true)
  {
    DWORD wait_result = ::MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);

    if (wait_result == WAIT_FAILED)
    {
      return -1;
    }

    while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
      {
        return (int)msg.wParam;
      }

      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
    }
  }

  return (int)msg.wParam;
}
void exec_on_thread(DWORD window_thread_id, std::move_only_function<void()> callback)
{
  struct handler
  {
    static void CALLBACK GetMsgProc(ULONG_PTR arg)
    {
      std::unique_ptr<std::move_only_function<void()>> callback_context{ (std::move_only_function<void()>*)arg };

      callback_context->operator()();
    }
  };
  auto handle = ::OpenThread(THREAD_SET_CONTEXT, FALSE, window_thread_id);

  assert(handle != nullptr);
  if (!handle)
  {
    return;
  }
  auto result = ::QueueUserAPC(handler::GetMsgProc, handle, (ULONG_PTR) new std::move_only_function<void()>{ std::move(callback) });
  assert(result != 0);
}

void load_core_module(fs::path load_path)
{
  if (!core_module)
  {
    try
    {
      auto dll_path = load_path / "siege-studio-core.dll";

      if (fs::exists(dll_path))
      {
        core_module.emplace(dll_path);

        auto* register_ptr = core_module->GetProcAddress<decltype(register_windows_ptr)>("register_windows");
        auto* deregister_ptr = core_module->GetProcAddress<decltype(deregister_windows_ptr)>("deregister_windows");

        if (register_ptr && deregister_ptr)
        {
          register_windows_ptr = register_ptr;
          deregister_windows_ptr = deregister_ptr;
        }
      }
    }
    catch (...)
    {
    }
  }
}

void unload_core_module(HWND window)
{
  if (core_module)
  {
    auto window_thread_id = ::GetWindowThreadProcessId(window, nullptr);

    if (::GetCurrentThreadId() != window_thread_id)
    {
      exec_on_thread(window_thread_id, [window]() {
        ::DestroyWindow(window);
      });
    }
    else
    {
      ::DestroyWindow(window);
    }


    while (::IsWindow(window))
    {
      ::Sleep(10);
    }

    deregister_windows_ptr(win32::module_ref::current_application());
    register_windows_ptr = &register_windows;
    deregister_windows_ptr = &deregister_windows;


    core_module->reset();
    core_module = std::nullopt;
  }
}

int register_and_create_main_window(DWORD window_thread_id, int nCmdShow)
{
  auto this_module = win32::module_ref::current_application();
  main_atom = register_windows_ptr(this_module);
  if (!main_atom)
  {
    return -1;
  }

  if (::GetCurrentThreadId() != window_thread_id)
  {
    exec_on_thread(window_thread_id, [nCmdShow]() {
      auto this_module = win32::module_ref::current_application();
      auto temp_window = win32::window_module_ref(this_module.get()).CreateWindowExW(CREATESTRUCTW{ .cx = CW_USEDEFAULT, .x = CW_USEDEFAULT, .style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, .lpszName = app_title.data(), .lpszClass = (LPCWSTR)main_atom });

      if (temp_window)
      {
        ::ShowWindow(*temp_window, nCmdShow);
        ::UpdateWindow(temp_window->release());
      }
    });
  }
  else
  {
    auto temp_window = win32::window_module_ref(this_module.get()).CreateWindowExW(CREATESTRUCTW{ .cx = CW_USEDEFAULT, .x = CW_USEDEFAULT, .style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, .lpszName = app_title.data(), .lpszClass = (LPCWSTR)main_atom });

    if (!temp_window)
    {
      return temp_window.error();
    }

    ::ShowWindow(*temp_window, nCmdShow);
    ::UpdateWindow(temp_window->release());
  }
  return 0;
}

discovery_info discover_installation_info(int major_version, int minor_version, std::wstring channel)
{
  discovery_info info{};

  std::vector<std::wstring> channels = {};

  if (channel.empty() || siege::platform::to_lower(channel) == L"any")
  {
    channels.emplace_back(L"stable");
    channels.emplace_back(L"development");
  }
  else
  {
    channels.emplace_back(channel);
  }

  std::string current_version = [=] {
    std::string result;
    std::ostringstream version;
    version << std::setw(2) << std::setfill('0') << major_version;

    result = version.str();
    version.str("");

    version << minor_version;

    return result + "." + version.str();
  }();

  info.current_app_exe_path = fs::path(win32::module_ref::current_application().GetModuleFileName());

  static bool has_embedded_modules = [] {
    std::vector<embedded_module> embedded_modules;
    embedded_modules.reserve(128);
    ::EnumResourceNamesW(win32::module_ref::current_application(), RT_RCDATA, enumerate_embedded_modules, (LONG_PTR)&embedded_modules);
    return !embedded_modules.empty();
  }();

  if (!has_embedded_modules)
  {
    info.launch_exe_path = info.current_app_exe_path;
    return info;
  }

  win32::com::com_string known_folder;
  std::error_code last_error;

  if (auto hresult = ::SHGetKnownFolderPath(FOLDERID_UserProgramFiles, 0, nullptr, known_folder.put()); hresult == S_OK)
  {
    auto root = fs::path(known_folder) / "The Siege Hub" / "Siege Studio";

    fs::create_directories(root, last_error);

    if (last_error)
    {
      goto discover_temp;
    }

    for (auto& channel : channels)
    {
      auto temp = root / channel / current_version;

      if (!fs::is_directory(temp, last_error))
      {
        continue;
      }

      std::optional<fs::path> app_path;
      int dll_count = 0;
      for (auto const& dir_entry : fs::directory_iterator{ temp })
      {
        if (dir_entry.path().filename() == info.current_app_exe_path.filename())
        {
          app_path = dir_entry.path();
        }

        if (dir_entry.path().extension() == ".dll" || dir_entry.path().extension() == ".DLL")
        {
          dll_count++;
        }

        if (app_path && dll_count >= 2)
        {
          info.matching_installed_exe_path = *app_path;
          break;
        }
      }

      if (info.matching_installed_exe_path)
      {
        break;
      }
    }


    std::set<fs::path, std::greater<fs::path>> other_paths;

    for (auto const& dir_entry : fs::recursive_directory_iterator{ root })
    {
      if (dir_entry.is_directory() && dir_entry.path().filename() != current_version && fs::exists(dir_entry.path() / info.current_app_exe_path.filename()) && dir_entry.path().filename().wstring().contains(L".") && dir_entry.path().filename().string() > current_version)
      {
        other_paths.emplace(dir_entry.path() / info.current_app_exe_path.filename());
      }
    }

    if (!other_paths.empty())
    {
      info.latest_installed_exe_path = *other_paths.begin();
    }

    if (!info.matching_installed_exe_path && !info.latest_installed_exe_path)
    {
      auto temp = root / SIEGE_CHANNEL_TYPE / current_version;
      fs::create_directories(temp, last_error);

      if (!last_error)
      {
        info.extraction_target_path = temp;
      }
    }
  }
discover_temp:

  if (info.latest_installed_exe_path)
  {
    info.launch_exe_path = *info.latest_installed_exe_path;
  }
  else if (info.matching_installed_exe_path)
  {
    info.launch_exe_path = *info.matching_installed_exe_path;
  }
  else
  {
    info.launch_exe_path = info.current_app_exe_path;
  }

  return info;
}