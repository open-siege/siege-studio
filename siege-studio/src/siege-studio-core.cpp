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
#include <iostream>
#include <filesystem>
#undef NDEBUG
#include <cassert>

#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/win/capabilities.hpp>

#include "views/siege_main_window.hpp"
#include "views/preferences_view.hpp"
#include "views/about_view.hpp"
#include "views/default_view.hpp"
#include "views/stack_layout.hpp"


ATOM register_windows(win32::window_module_ref this_module)
{
  siege::views::stack_layout::register_class(this_module);
  auto main_atom = siege::views::siege_main_window::register_class(this_module);
  win32::window_meta_class<siege::views::default_view> def_info{};
  def_info.hCursor = LoadCursorW(this_module, IDC_ARROW);
  def_info.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  def_info.hIcon = (HICON)::LoadImageW(this_module, L"AppIcon", IMAGE_ICON, 0, 0, 0);
  this_module.RegisterClassExW(def_info);

  win32::window_meta_class<siege::views::preferences_view> pref_info{};
  pref_info.hCursor = LoadCursorW(this_module, IDC_ARROW);
  pref_info.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  pref_info.hIcon = (HICON)::LoadImageW(this_module, L"AppIcon", IMAGE_ICON, 0, 0, 0);
  this_module.RegisterClassExW(pref_info);
  return main_atom;
}

BOOL deregister_windows(win32::window_module_ref this_module)
{
  // TODO implement deregistration
  return TRUE;
}