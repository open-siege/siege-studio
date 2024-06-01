#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/platform/win/core/com/base.hpp>
#include <siege/platform/win/core/com/stream_buf.hpp>

struct test_class : win32::com::ComObject
{
	HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
	{
		return ComQuery<IUnknown, IUnknown>(*this, riid, ppvObj)
			.value_or(E_NOINTERFACE);
	}
};


TEST_CASE("When ComObject is stack allocated, the COM heap is not used.", "[win.core.com.base]")
{
	test_class test_value{};

	REQUIRE(win32::com::ComObject::IsHeapAllocated(&test_value, sizeof(test_class)) == false);
}

TEST_CASE("When ComObject is allocated with the new operator, the object is allocated on the COM heap.", "[win.core.com.base]")
{
	auto* test_value = new test_class{};

	REQUIRE(win32::com::ComObject::IsHeapAllocated(test_value, sizeof(test_class)) == true);

	delete test_value;
}

TEST_CASE("When ComObject is deleted with the delete operator, the object is removed from the COM heap.", "[win.core.com.base]")
{
	auto* test_value = new test_class{};

	auto* raw_value = test_value;

	delete test_value;
	REQUIRE(win32::com::ComObject::IsHeapAllocated(raw_value, sizeof(test_class)) == false);
}

TEST_CASE("When ComObject is allocated with std::make_unique, the object is allocated on the COM heap.", "[win.core.com.base]")
{
	auto test_value = std::make_unique<test_class>();

	REQUIRE(win32::com::ComObject::IsHeapAllocated(test_value.get(), sizeof(test_class)) == true);
}

TEST_CASE("When unique_ptr with com object is reset, the object is removed from the COM heap.", "[win.core.com.base]")
{
	auto test_value = std::make_unique<test_class>();

	auto* raw_value = test_value.get();
	test_value.reset();

	REQUIRE(win32::com::ComObject::IsHeapAllocated(raw_value, sizeof(test_class)) == false);
}
