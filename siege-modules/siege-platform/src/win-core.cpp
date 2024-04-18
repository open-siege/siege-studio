#include <siege/platform/win/core/com_client.hpp>
#include <siege/platform/win/core/com_variant.hpp>
#include <siege/platform/win/core/com_property_variant.hpp>
#include <siege/platform/win/core/com_stream_buf.hpp>
#include <siege/platform/win/core/com_xml.hpp>
#include <siege/platform/win/core/com.hpp>
#include <set>

namespace win32::com
{
	HRESULT init_com()
	{
		thread_local HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		thread_local auto com_handle = as_unique<HRESULT>(&result, [](auto*){ CoUninitialize(); });
		return result;
	}

	std::set<void*>& GetHeapAllocations()
        {
            static std::set<void*> allocations;

            return allocations;
        }

        bool ComObject::IsHeapAllocated(void* object, std::size_t size)
        {
            auto& allocations = GetHeapAllocations();

            auto objectValue = reinterpret_cast<std::size_t>(object);
            auto item = std::find_if(allocations.begin(), allocations.end(), [&](auto start) {
                    
                    auto startRange = reinterpret_cast<std::size_t>(start);
                    auto endRange = startRange + size;

                    return objectValue >= startRange && objectValue < endRange;
            });
            return item != allocations.end();
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
            if (IsHeapAllocated(ptr, sz))
            {
                return;
            }

            return ::CoTaskMemFree(ptr);
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