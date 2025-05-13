#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
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

extern "C" {
static std::set<ATOM> atoms_to_unregister{};

ATOM register_windows(HMODULE module)
{
  win32::window_module_ref this_module(module);
  atoms_to_unregister.emplace(siege::views::stack_layout::register_class(this_module));
  auto main_atom = siege::views::siege_main_window::register_class(this_module);
  atoms_to_unregister.emplace(main_atom);
  win32::window_meta_class<siege::views::default_view> def_info{};
  def_info.hCursor = LoadCursorW(this_module, IDC_ARROW);
  def_info.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  def_info.hIcon = (HICON)::LoadImageW(this_module, L"AppIcon", IMAGE_ICON, 0, 0, 0);
  atoms_to_unregister.emplace(this_module.RegisterClassExW(def_info));

  win32::window_meta_class<siege::views::preferences_view> pref_info{};
  pref_info.hCursor = LoadCursorW(this_module, IDC_ARROW);
  pref_info.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  pref_info.hIcon = (HICON)::LoadImageW(this_module, L"AppIcon", IMAGE_ICON, 0, 0, 0);
  atoms_to_unregister.emplace(this_module.RegisterClassExW(pref_info));
  return main_atom;
}

BOOL deregister_windows(HMODULE module)
{
  auto temp = atoms_to_unregister;
  for (auto atom : temp)
  {
    if (::UnregisterClassW((wchar_t*)atom, module))
    {
      atoms_to_unregister.erase(atom);
    }
  }

  return TRUE;
}
}