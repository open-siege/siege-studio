module;
#include <siege/platform/windows.hpp>
#undef TRUE
#undef FALSE
export module win32.utils;

import std;

export namespace win32
{
  using ::BOOL;
  using ::DWORD;
  using ::WORD;
  using ::BYTE;
  using ::UINT;
  using ::ULONG;
  using ::LONG;
  using ::LONG_PTR;
  using ::ULONG_PTR;
  using ::DWORD_PTR;
  using ::INT_PTR;
  using ::UINT_PTR;
  using ::HANDLE;
  using ::LPVOID;
  using ::PVOID;
  using ::HRESULT;
  using ::WCHAR;
  using ::LPWSTR;
  using ::LPCWSTR;
  using ::LPSTR;
  using ::LPCSTR;

  constexpr BOOL TRUE = 1;
  constexpr BOOL FALSE = 0;

  template<typename THandle, typename TDeleter>
  struct auto_handle : std::unique_ptr<std::decay_t<decltype(*std::declval<THandle>())>, TDeleter>
  {
    using base = std::unique_ptr<std::decay_t<decltype(*std::declval<THandle>())>, TDeleter>;
    using base::base;

    operator THandle()
    {
      return this->get();
    }

    operator THandle() const
    {
      return this->get();
    }
  };
}
