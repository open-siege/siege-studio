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
#include <filesystem>
#undef NDEBUG
#include <cassert>

#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <commctrl.h>
#include <imagehlp.h>


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
  embedded_dlls.reserve(32);

  ::EnumResourceNamesW(hInstance, RT_RCDATA, extract_embedded_dlls, (LONG_PTR)&embedded_dlls);

  std::for_each(embedded_dlls.begin(), embedded_dlls.end(), [=](embedded_dll& dll) {
    auto entry = ::FindResourceW(hInstance, dll.filename.c_str(), RT_RCDATA);

    if (!entry)
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

  auto* register_windows_ptr = &register_windows;
  auto* deregister_windows_ptr = &deregister_windows;

  try
  {
    auto exe_path = this_module.GetModuleFileName();

    auto dll_path = std::filesystem::path(exe_path).parent_path() / "siege-studio-core.dll";

    if (std::filesystem::exists(dll_path)) {
      static win32::window_module core_module{ dll_path };

      auto* register_ptr = core_module.GetProcAddress<decltype(register_windows_ptr)>("register_windows");
      auto* deregister_ptr = core_module.GetProcAddress<decltype(deregister_windows_ptr)>("deregister_windows");

      if (register_ptr && deregister_ptr) {
        register_windows_ptr = register_ptr;
        deregister_ptr = deregister_windows_ptr;
      }
    }
  }
  catch (...)
  {

  }

  auto main_atom = register_windows_ptr(hInstance);
  if (!main_atom)
  {
    return -1;
  }

  auto main_window = this_module.CreateWindowExW(CREATESTRUCTW{
    .cx = CW_USEDEFAULT,
    .x = CW_USEDEFAULT,
    .style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
    .lpszName = app_title.data(),
    .lpszClass = (LPCWSTR)main_atom });

  if (!main_window)
  {
    return main_window.error();
  }

  ::ShowWindow(*main_window, nCmdShow);
  ::UpdateWindow(*main_window);

  MSG msg;

  while (::GetMessageW(&msg, nullptr, 0, 0))
  {
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
  }

  deregister_windows_ptr(hInstance);

  return (int)msg.wParam;
}