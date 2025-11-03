#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
#undef NDEBUG
#include <cassert>

#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/win/capabilities.hpp>

namespace siege::views
{
  ATOM register_stack_layout(HINSTANCE module);
  ATOM register_main_window(HINSTANCE module);
  ATOM register_default_view(HINSTANCE module);
  ATOM register_preferences_view(HINSTANCE module);
}// namespace siege::views

extern "C" {
static std::set<ATOM> atoms_to_unregister{};

ATOM register_windows(HMODULE module)
{
  atoms_to_unregister.emplace(siege::views::register_stack_layout(module));
  auto main_atom = siege::views::register_main_window(module);
  atoms_to_unregister.emplace(main_atom);
  atoms_to_unregister.emplace(siege::views::register_default_view(module));
  atoms_to_unregister.emplace(siege::views::register_preferences_view(module));
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