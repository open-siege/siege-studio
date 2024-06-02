#ifndef SIEGE_PLATFORM_RESOURCE_STORAGE_HPP
#define SIEGE_PLATFORM_RESOURCE_STORAGE_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/win/core/com/storage.hpp>

namespace siege::platform
{
	class StorageReaderRef : virtual win32::com::StorageBase
	{
		std::istream& stream;
		resource_reader& reader;
	
		StorageReaderRef(std::istream& stream, resource_reader& reader) : stream(stream), reader(reader)
		{
		}
	
		HRESULT __stdcall OpenStream(const OLECHAR* name, void*, DWORD, DWORD, IStream**) override
		{
			return E_NOTIMPL;			
		}

		HRESULT __stdcall OpenStorage(const OLECHAR*, IStorage*, DWORD, SNB, DWORD, IStorage**) override
		{
			return E_NOTIMPL;						
		}

		HRESULT __stdcall EnumElements(DWORD, void*, DWORD, IEnumSTATSTG**) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall CopyTo(DWORD, const IID*, SNB, IStorage*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall Stat(STATSTG*, DWORD) override
		{
			return E_NOTIMPL;
		}
	};
}

#endif