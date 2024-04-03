#include <set>
#include "win32_com_server.hpp"
#include "win32_property_variant.hpp"

namespace win32::com
{
        std::set<void*>& GetHeapAllocations()
        {
            static std::set<void*> allocations;

            return allocations;
        }

        bool ComAllocatorAware::IsHeapAllocated(void* object, std::size_t size)
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

        void* ComAllocatorAware::operator new(std::size_t count)
        {
            void* result = ::CoTaskMemAlloc(count);
            assert(result);
            GetHeapAllocations().insert(result);

            return result;
        }

        void ComAllocatorAware::operator delete(void* ptr, std::size_t sz)
        {
            if (IsHeapAllocated(ptr, sz))
            {
                return;
            }

            return ::CoTaskMemFree(ptr);
        }

        ULONG __stdcall ComAllocatorAware::AddRef() noexcept
        {
            return ++refCount;
        }

        ULONG __stdcall ComAllocatorAware::Release() noexcept
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