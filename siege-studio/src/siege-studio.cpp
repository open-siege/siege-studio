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
#include <siege/platform/win/window_impl.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <imagehlp.h>

namespace fs = std::filesystem;
extern "C" __declspec(dllexport) std::uint32_t DisableSiegeExtensionModule = -1;
constexpr static std::wstring_view app_title = L"Siege Studio";

struct embedded_dll
{
  std::filesystem::path filename;
  HGLOBAL handle;
  WORD lang_id;
};

BOOL __stdcall enumerate_embedded_dlls(HMODULE module,
  LPCWSTR type,
  LPWSTR name,
  LONG_PTR lParam)
{
  std::vector<embedded_dll>* items = (std::vector<embedded_dll>*)lParam;

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

  if (path.extension() == ".dll" || path.extension() == ".DLL")
  {
    items->emplace_back(embedded_dll{ std::move(path), data });
  }

  return TRUE;
}

BOOL __stdcall enumerate_dll_languages(HMODULE hModule, const wchar_t* lpszType, const wchar_t* lpszName, WORD lang_id, LONG_PTR lParam)
{
  embedded_dll* dll = (embedded_dll*)lParam;

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

fs::path resolve_install_path(int major_version, int minor_version)
{
  fs::path install_path = fs::current_path();

  std::error_code last_error;
  win32::com::com_string known_folder;
  std::vector<fs::path> paths_to_try;

  if (auto hresult = ::SHGetKnownFolderPath(FOLDERID_UserProgramFiles, 0, nullptr, known_folder.put()); hresult == S_OK)
  {
    paths_to_try.emplace_back(fs::path(known_folder));
  }
  paths_to_try.emplace_back(fs::temp_directory_path());

  std::string version = [=] {
    std::string result;
    std::ostringstream version;
    version << std::setw(2) << std::setfill('0') << major_version;

    result = version.str();
    version.str("");

    version << minor_version;

    return result + "." + version.str();
  }();


  for (auto& path : paths_to_try)
  {
    auto temp = path / "The Siege Hub" / "Siege Studio" / version;

    fs::create_directories(temp, last_error);

    if (!last_error)
    {
      install_path = temp;
      break;
    }
  }

  return install_path;
}


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
  win32::com::init_com();

  win32::init_common_controls_ex({ .dwSize{ sizeof(INITCOMMONCONTROLSEX) },
    .dwICC = ICC_STANDARD_CLASSES | ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_DATE_CLASSES | ICC_BAR_CLASSES });

  auto wic_version = win32::get_wic_version();

  if (!wic_version)
  {
    ::MessageBoxW(nullptr, L"Windows Imaging Component has not been detected. Siege Studio will not function correctly without it. If you are using an older version of Windows, make sure to either install the latest updates or the Windows Imaging Component redistributable.", L"Windows Imaging Component is missing", MB_OK | MB_ICONERROR);
  }

  win32::window_module_ref this_module(hInstance);

  std::vector<embedded_dll> embedded_dlls;
  embedded_dlls.reserve(128);

  auto app_path = fs::path(win32::module_ref::current_application().GetModuleFileName());
  auto install_path = app_path.parent_path();

  ::EnumResourceNamesW(hInstance, RT_RCDATA, enumerate_embedded_dlls, (LONG_PTR)&embedded_dlls);

  for (auto& dll : embedded_dlls)
  {
    ::EnumResourceLanguagesW(hInstance, RT_RCDATA, dll.filename.c_str(), enumerate_dll_languages, (LONG_PTR)&dll);
  }

  if (!embedded_dlls.empty())
  {
    install_path = resolve_install_path(SIEGE_MAJOR_VERSION, SIEGE_MINOR_VERSION);
    std::for_each(embedded_dlls.begin(), embedded_dlls.end(), [=](embedded_dll& dll) {
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

    if (install_path != app_path.parent_path())
    {
      auto new_path = install_path / app_path.filename();

      std::error_code last_error;

      if (fs::exists(new_path, last_error))
      {
        goto launch_app;
      }

      fs::copy_file(app_path, new_path, last_error);

      if (last_error)
      {
        goto launch_app;
      }

      if (auto handle = ::BeginUpdateResourceW(new_path.c_str(), FALSE); handle)
      {
        for (auto& dll : embedded_dlls)
        {
          ::UpdateResourceW(handle, RT_RCDATA, dll.filename.c_str(), dll.lang_id, nullptr, 0);
        }

        ::EndUpdateResourceW(handle, FALSE);
      }
    launch_app:
      auto new_path_str = new_path.wstring();
      STARTUPINFO startup{};
      PROCESS_INFORMATION process{};

      if (::CreateProcessW(new_path.c_str(), new_path_str.data(), nullptr, nullptr, TRUE, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &startup, &process))
      {
        ::ExitProcess(0);
        return 0;
      }
    }
  }

  load_core_module(install_path);

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

    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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