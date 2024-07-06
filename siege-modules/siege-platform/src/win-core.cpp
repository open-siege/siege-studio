#include <siege/platform/win/core/com/client.hpp>
#include <siege/platform/win/core/com/variant.hpp>
#include <siege/platform/win/core/com/property_variant.hpp>
#include <siege/platform/win/core/com/base.hpp>
#include <siege/platform/win/core/module.hpp>
#include <cassert>
#include <set>
#include <comcat.h>

namespace win32::com
{
	static_assert(sizeof(std::uint32_t) == sizeof(ULONG));
	static_assert(sizeof(std::uint32_t) == sizeof(LCID));
	static_assert(sizeof(char16_t) == sizeof(wchar_t));

	HRESULT init_com(COINIT apartment_model)
	{
		thread_local HRESULT result = CoInitializeEx(nullptr, apartment_model);
		thread_local auto com_handle = std::unique_ptr<HRESULT, void(*)(HRESULT*)>(&result, [](auto*) { CoUninitialize(); });

		return result;
	}

	std::set<void*>& GetHeapAllocations()
	{
		static std::set<void*> allocations;

		return allocations;
	}

	auto FindHeapAllocation(void* object, std::size_t size)
	{
		auto& allocations = GetHeapAllocations();

		auto objectValue = reinterpret_cast<std::size_t>(object);
		auto item = std::find_if(allocations.begin(), allocations.end(), [&](auto start) {

			auto startRange = reinterpret_cast<std::size_t>(start);
			auto endRange = startRange + size;

			return objectValue >= startRange && objectValue < endRange;
			});

		return item;
	}

	bool ComObject::IsHeapAllocated(void* object, std::size_t size)
	{
		auto& allocations = GetHeapAllocations();

		return FindHeapAllocation(object, size) != allocations.end();
	}

	void* ComObject::operator new(std::size_t count)
	{
		void* result = ::CoTaskMemAlloc(count);
		assert(result);
		GetHeapAllocations().insert(result);

		return result;
	}

	void ComObject::operator delete(void* ptr, std::size_t sz)
	{
		auto iter = FindHeapAllocation(ptr, sz);

		auto& allocations = GetHeapAllocations();

		if (iter == allocations.end())
		{
			return;
		}

		::CoTaskMemFree(*iter);
		allocations.erase(iter); 
	}

	ULONG __stdcall ComObject::AddRef() noexcept
	{
		return ++refCount;
	}

	ULONG __stdcall ComObject::Release() noexcept
	{
		if (refCount == 0)
		{
			return 0;
		}

		if (refCount == 1 && !IsHeapAllocated(this, sizeof(*this)))
		{
			return 1;
		}

		--refCount;

		if (refCount == 0)
		{
			delete this;
			return 0;
		}

		return refCount;
	}
}
