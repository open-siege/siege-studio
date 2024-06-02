#ifndef WIN32_STORAGE_HPP
#define WIN32_STORAGE_HPP

#include <istream>
#include <memory>
#include <array>
#include <siege/platform/win/core/com/base.hpp>
#include <objidl.h>

namespace win32::com
{
	class StorageBase : public ComObject, public ::IStorage
	{
	public:
		HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
		{
			return ComQuery<IUnknown, ::IStorage>(*this, riid, ppvObj)
				.or_else([&]() { return ComQuery<::IStorage>(*this, riid, ppvObj); })
				.value_or(E_NOINTERFACE);
		}

		[[maybe_unused]] ULONG __stdcall AddRef() noexcept override
		{
			return ComObject::AddRef();
		}

		[[maybe_unused]] ULONG __stdcall Release() noexcept override
		{
			return ComObject::Release();
		}

		HRESULT __stdcall CreateStream(const OLECHAR* name, DWORD, DWORD, DWORD, IStream**) override
		{
			return E_NOTIMPL;			
		}

		HRESULT __stdcall OpenStream(const OLECHAR* name, void*, DWORD, DWORD, IStream**) override
		{
			return E_NOTIMPL;			
		}

		HRESULT __stdcall OpenStorage(const OLECHAR*, IStorage*, DWORD, SNB, DWORD, IStorage**) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall CopyTo(DWORD, const IID*, SNB, IStorage*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall MoveElementTo(const OLECHAR* name, IStorage* dest, const OLECHAR* new_name, DWORD) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall Commit(DWORD) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall Revert() override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall EnumElements(DWORD, void*, DWORD, IEnumSTATSTG**) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall DestroyElement(const OLECHAR*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall RenameElement(const OLECHAR*, const OLECHAR*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall SetElementTimes(const OLECHAR*, const FILETIME*, const FILETIME*, const FILETIME*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall SetClass(REFCLSID) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall SetStateBits(DWORD, DWORD) override
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