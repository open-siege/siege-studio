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

#include <siege/platform/win/desktop/shell.hpp>
#include <siege/platform/win/desktop/window_impl.hpp>
#include <siege/platform/win/core/com/client.hpp>
#include <siege/platform/shared.hpp>
#include <commctrl.h>

#include "views/siege_main_window.hpp"
#include "views/preferences_view.hpp"
#include "views/about_view.hpp"
#include "views/default_view.hpp"

extern "C" __declspec(dllexport) std::uint32_t DisableSiegeExtensionModule = -1;
constexpr static std::wstring_view app_title = L"Siege Studio";

BOOL __stdcall extract_embedded_dlls(HMODULE module,
  LPCWSTR type,
  LPWSTR name,
  LONG_PTR lParam)
{
  std::vector<std::filesystem::path>* items = (std::vector<std::filesystem::path>*)lParam;

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
    auto size = ::SizeofResource(module, entry);
    auto bytes = ::LockResource(data);

    std::ofstream output(siege::platform::to_lower(path.c_str()), std::ios::trunc | std::ios::binary);
    output.write((const char*)bytes, size);
  }

  return TRUE;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
  win32::com::init_com();

  INITCOMMONCONTROLSEX settings{
    .dwSize{ sizeof(INITCOMMONCONTROLSEX) }
  };
  ::InitCommonControlsEx(&settings);

  win32::window_module_ref this_module(hInstance);

  ::EnumResourceNamesW(hInstance, RT_RCDATA, extract_embedded_dlls, 0);

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