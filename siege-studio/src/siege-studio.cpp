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
#include <thread>
#include <iostream>
#include <filesystem>
#undef NDEBUG
#include <cassert>

#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <siege/platform/shared.hpp>
#include <commctrl.h>

#include "views/siege_main_window.hpp"
#include "views/preferences_view.hpp"
#include "views/about_view.hpp"
#include "views/default_view.hpp"
#include "views/stack_layout.hpp"

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

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
  win32::com::init_com();

  INITCOMMONCONTROLSEX settings{
    .dwSize{ sizeof(INITCOMMONCONTROLSEX) },
    .dwICC = ICC_STANDARD_CLASSES | 
             ICC_INTERNET_CLASSES | 
             ICC_LISTVIEW_CLASSES | 
             ICC_TAB_CLASSES | 
             ICC_TREEVIEW_CLASSES | 
             ICC_DATE_CLASSES | 
             ICC_BAR_CLASSES
  };
  ::InitCommonControlsEx(&settings);

  win32::window_module_ref this_module(hInstance);

  std::vector<embedded_dll> embedded_dlls;
  embedded_dlls.reserve(32);

  ::EnumResourceNamesW(hInstance, RT_RCDATA, extract_embedded_dlls, (LONG_PTR)&embedded_dlls);

  std::for_each(std::execution::par_unseq, embedded_dlls.begin(), embedded_dlls.end(), [=](embedded_dll& dll) {
    auto entry = ::FindResourceW(hInstance, dll.filename.c_str(), RT_RCDATA);

    if (!entry)
    {
      return;
    }

    auto size = ::SizeofResource(hInstance, entry);
    auto bytes = ::LockResource(dll.handle);

    std::ofstream output(siege::platform::to_lower(dll.filename.c_str()), std::ios::trunc | std::ios::binary);
    output.write((const char*)bytes, size);
  });

  siege::views::stack_layout::register_class(this_module);

  win32::window_meta_class<siege::views::siege_main_window> info{};
  info.hCursor = LoadCursorW(hInstance, IDC_ARROW);
  info.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  info.hIcon = (HICON)::LoadImageW(hInstance, L"AppIcon", IMAGE_ICON, 0, 0, 0);
  this_module.RegisterClassExW(info);

  win32::window_meta_class<siege::views::default_view> def_info{};
  def_info.hCursor = LoadCursorW(hInstance, IDC_ARROW);
  def_info.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  def_info.hIcon = info.hIcon;
  this_module.RegisterClassExW(def_info);

  win32::window_meta_class<siege::views::preferences_view> pref_info{};
  pref_info.hCursor = LoadCursorW(hInstance, IDC_ARROW);
  pref_info.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  pref_info.hIcon = info.hIcon;
  this_module.RegisterClassExW(pref_info);

  auto main_window = this_module.CreateWindowExW(CREATESTRUCTW{
    .cx = CW_USEDEFAULT,
    .x = CW_USEDEFAULT,
    .style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
    .lpszName = app_title.data(),
    .lpszClass = win32::type_name<siege::views::siege_main_window>().c_str() });

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

  return (int)msg.wParam;
}