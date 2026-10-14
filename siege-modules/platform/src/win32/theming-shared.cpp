module;
#include <siege/platform/windows.hpp>

module win32.theming;

import std;

import win32.capabilities;
import win32.common_controls;
import win32.window;

namespace win32
{
  bool is_parent_from_system(HWND window)
  {
    if (!window)
    {
      return false;
    }

    if (window == HWND_MESSAGE)
    {
      return true;
    }

    auto module = (HMODULE)::GetClassLongPtrW(window, GCLP_HMODULE);

    if (!module)
    {
      return false;
    }

    if (::GetPropW(window, L"SiegeForceThemed") == reinterpret_cast<HANDLE>(1))
    {
      return false;
    }

    if (module == ::GetModuleHandleW(L"comdlg32") || module == ::GetModuleHandleW(L"shell32") || module == ::GetModuleHandleW(L"user32") || module == ::GetModuleHandleW(L"ExplorerFrame") || module == ::GetModuleHandleW(L"shlwapi"))
    {
      return true;
    }

    return false;
  }

  void superclass_handle::init(const wchar_t* class_name, LONG_PTR sub_class)
  {
    dummy = ::CreateWindowExW(0, class_name, L"Dummy", 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
    control_proc = (decltype(control_proc))::SetClassLongPtrW(dummy, GCLP_WNDPROC, sub_class);
  }

  void superclass_handle::dispose(std::unordered_set<HWND>& controls, LONG_PTR sub_class)
  {
    if (!dummy)
    {
      return;
    }

    for (auto control : controls)
    {
      auto existing_proc = ::GetWindowLongPtrW(control, GWLP_WNDPROC);

      if (existing_proc == sub_class)
      {
        ::SetWindowLongW(control, GWLP_WNDPROC, (LONG_PTR)control_proc);
      }
    }

    ::SetClassLongPtrW(dummy, GCLP_WNDPROC, (LONG_PTR)control_proc);
    ::DestroyWindow(dummy);
    dummy = nullptr;
  }

  superclass_handle::~superclass_handle()
  {
    if (dummy)
    {
      ::SetClassLongPtrW(dummy, GCLP_WNDPROC, (LONG_PTR)control_proc);
      ::DestroyWindow(dummy);
      dummy = nullptr;
    }
  }

  LRESULT __stdcall root_window_handler::handle_root_message(HWND root, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
  {
    if (dwRefData && message == WM_SETTINGCHANGE && lParam && std::wstring_view((wchar_t*)lParam) == L"ImmersiveColorSet")
    {
      auto result = def_subclass_proc(root, message, wParam, lParam);
      auto& controls = *(std::unordered_set<HWND>*)dwRefData;

      for (auto& control : controls)
      {
        SendMessage(control, WM_SETTINGCHANGE, lParam, wParam);
        SendMessage(control, WM_THEMECHANGED, 0, 0);
      }

      return result;
    }

    if (dwRefData && message == WM_DPICHANGED)
    {
      auto& controls = *(std::unordered_set<HWND>*)dwRefData;

      if (auto best_font = get_best_system_font())
      {
        for (auto& control : controls)
        {
          auto font = win32::load_font(LOGFONTW{ .lfPitchAndFamily = VARIABLE_PITCH }, *best_font);
          ::SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), TRUE);
        }
      }

      return def_subclass_proc(root, message, wParam, lParam);
    }

    if (message == WM_NCDESTROY)
    {
      remove_window_subclass(root, handle_root_message, 0);
    }

    return def_subclass_proc(root, message, wParam, lParam);
  }
}// namespace win32
