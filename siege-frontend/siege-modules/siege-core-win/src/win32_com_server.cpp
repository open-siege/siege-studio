#include "win32_com_server.hpp"

namespace win32::com
{
    struct ComAllocatorAware : IUnknown
    {
        std::atomic_int refCount = 1;

        static std::set<void*>& ComAllocatorAware::GetHeapAllocations()
        {
            static std::set<void*> allocations;

            return allocations;
        }

        static bool ComAllocatorAware::IsHeapAllocated(void* object)
        {
            auto& allocations = GetHeapAllocations();

            auto item = allocations.find(object);

            return item != allocations.end();
        }

        static void* ComAllocatorAware::operator new(std::size_t count)
        {
            void* result = ::CoTaskMemAlloc(count);

            GetHeapAllocations().insert(result);

            return result;
        }

        static void ComAllocatorAware::operator delete(void* ptr, std::size_t sz)
        {
            if (IsHeapAllocated(ptr))
            {
                return;
            }

            return ::CoTaskMemFree(ptr);
        }

        ULONG __stdcall ComAllocatorAware::AddRef()
        {
            return ++refCount;
        }

        ULONG __stdcall ComAllocatorAware::Release()
        {
            if (refCount == 0)
            {
                return 0;
            }

            if (refCount == 1 && !IsHeapAllocated(this))
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
    };
}