#include <catch2/catch_test_macros.hpp>
#include <array>
#include <string>
#include <siege/platform/win/core/com/enumerable.hpp>

TEST_CASE("When RangeEnumerator<wstring>.QueryInterface has IUnknown, return code is okay.")
{
  std::array<std::wstring, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<std::wstring>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IUnknown> temp;

  REQUIRE(test->QueryInterface(IID_IUnknown, temp.put_void()) == S_OK);
}

TEST_CASE("When RangeEnumerator<wstring>.QueryInterface has IUnknown, pointer is valid.")
{
  std::array<std::wstring, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<std::wstring>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IUnknown> temp;

  REQUIRE(temp.get() == nullptr);
  test->QueryInterface(IID_IUnknown, temp.put_void());
  REQUIRE(temp.get() != nullptr);
}

TEST_CASE("When RangeEnumerator<wstring>.QueryInterface has IEnumString, return code is okay.")
{
  std::array<std::wstring, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<std::wstring>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IEnumString> temp;

  REQUIRE(test->QueryInterface(IID_IEnumString, temp.put_void()) == S_OK);
}

TEST_CASE("When RangeEnumerator<wstring>.QueryInterface has IEnumString, pointer is valid.")
{
  std::array<std::wstring, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<std::wstring>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IEnumString> temp;

  REQUIRE(temp.get() == nullptr);
  test->QueryInterface(IID_IEnumString, temp.put_void());
  REQUIRE(temp.get() != nullptr);
}

TEST_CASE("When RangeEnumerator<wstring>.QueryInterface has IEnumVARIANT, return code is no interface.")
{
  std::array<std::wstring, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<std::wstring>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IEnumVARIANT> temp;

  REQUIRE(test->QueryInterface(IID_IEnumVARIANT, temp.put_void()) == E_NOINTERFACE);
}

TEST_CASE("When RangeEnumerator<Variant>.QueryInterface has IEnumVARIANT, return code is okay.")
{
  std::array<win32::com::Variant, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<win32::com::Variant>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IEnumVARIANT> temp;

  REQUIRE(test->QueryInterface(IID_IEnumVARIANT, temp.put_void()) == S_OK);
}

TEST_CASE("When RangeEnumerator<Variant>.QueryInterface has IEnumString, return code is no interface.")
{
  std::array<win32::com::Variant, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<win32::com::Variant>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IEnumString> temp;

  REQUIRE(test->QueryInterface(IID_IEnumString, temp.put_void()) == E_NOINTERFACE);
}

TEST_CASE("When RangeEnumerator<std::wstring_view>.QueryInterface has IEnumString, return code is okay.")
{
  std::array<std::wstring_view, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<std::wstring_view>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IEnumString> temp;

  REQUIRE(test->QueryInterface(IID_IEnumString, temp.put_void()) == S_OK);
}

TEST_CASE("When RangeEnumerator<std::wstring_view>.QueryInterface has IEnumVARIANT, return code is no interface.")
{
  std::array<std::wstring_view, 1> storage{};
  auto test = win32::com::make_unique_range_enumerator<std::wstring_view>(storage.begin(), storage.begin(), storage.end());

  win32::com::com_ptr<IEnumVARIANT> temp;

  REQUIRE(test->QueryInterface(IID_IEnumVARIANT, temp.put_void()) == E_NOINTERFACE);
}