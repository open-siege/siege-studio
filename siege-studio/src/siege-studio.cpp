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
};

BOOL __stdcall extract_embedded_dlls(HMODULE module,
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
  std::filesystem::path path(name);

  if (path.extension() == ".dll" || path.extension() == ".DLL")
  {
    items->emplace_back(embedded_dll{ std::move(path), data });
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

void load_core_module()
{
  if (!core_module)
  {
    try
    {
      auto exe_path = win32::module_ref::current_application().GetModuleFileName();
      auto dll_path = std::filesystem::path(exe_path).parent_path() / "siege-studio-core.dll";

      if (std::filesystem::exists(dll_path))
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


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
  win32::com::init_com();

  INITCOMMONCONTROLSEX settings{
    .dwSize{ sizeof(INITCOMMONCONTROLSEX) },
    .dwICC = ICC_STANDARD_CLASSES | ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_DATE_CLASSES | ICC_BAR_CLASSES
  };
  win32::init_common_controls_ex(&settings);

  auto wic_version = win32::get_wic_version();

  if (!wic_version)
  {
    ::MessageBoxW(nullptr, L"Windows Imaging Component has not been detected. Siege Studio will not function correctly without it. If you are using an older version of Windows, make sure to either install the latest updates or the Windows Imaging Component redistributable.", L"Windows Imaging Component is missing", MB_OK | MB_ICONERROR);
  }

  win32::window_module_ref this_module(hInstance);

  std::vector<embedded_dll> embedded_dlls;
  embedded_dlls.reserve(128);

  ::EnumResourceNamesW(hInstance, RT_RCDATA, extract_embedded_dlls, (LONG_PTR)&embedded_dlls);

  std::for_each(embedded_dlls.begin(), embedded_dlls.end(), [=](embedded_dll& dll) {
    auto entry = ::FindResourceW(hInstance, dll.filename.c_str(), RT_RCDATA);

    if (!entry)
    {
      return;
    }

    std::error_code code;
    if (fs::exists(siege::platform::to_lower(dll.filename.wstring()), code))
    {
      return;
    }

    auto size = ::SizeofResource(hInstance, entry);
    auto bytes = ::LockResource(dll.handle);


    {
      std::ofstream output(siege::platform::to_lower(dll.filename.wstring()), std::ios::trunc | std::ios::binary);
      output.write((const char*)bytes, size);
    }

    LOADED_IMAGE image{};
    if (::MapAndLoad(siege::platform::to_lower(dll.filename.string()).c_str(), nullptr, &image, TRUE, FALSE))
    {
      image.FileHeader->OptionalHeader.MajorImageVersion = SIEGE_MAJOR_VERSION;
      image.FileHeader->OptionalHeader.MinorImageVersion = SIEGE_MINOR_VERSION;
      ::UnMapAndLoad(&image);
    }
  });

  load_core_module();

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