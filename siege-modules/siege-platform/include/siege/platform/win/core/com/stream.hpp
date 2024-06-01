#ifndef WIN32_STREAM_HPP
#define WIN32_STREAM_HPP

#include <istream>
#include <memory>
#include <array>
#include <siege/platform/win/core/com/base.hpp>
#include <objidl.h>

namespace win32::com
{
	class StdStreamRef : public ComObject, public ::IStream
	{
		std::istream& stream;

	public:
		StdStreamRef(std::istream& stream) : stream(stream)
		{		
		}

		HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComQuery<IUnknown, ::IStream>(*this, riid, ppvObj)
                .or_else([&]() { return ComQuery<::ISequentialStream>(*this, riid, ppvObj); })
                .or_else([&]() { return ComQuery<::IStream>(*this, riid, ppvObj); })
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

		HRESULT __stdcall Read(void* data, ULONG size, ULONG* values_read) override
		{
			if (data == nullptr || values_read == nullptr)
			{
				return STG_E_INVALIDPOINTER;
			}

			if (size == 0)
			{
				*values_read = 0;
				return S_OK;
			}

			*values_read = stream.read((char*)data, size).gcount();

			return size == *values_read ? S_OK : S_FALSE;
		}

		HRESULT __stdcall Write(const void*, ULONG, ULONG*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall Clone(IStream**) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall SetSize(ULARGE_INTEGER) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall Stat(STATSTG*, DWORD) override
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

		HRESULT __stdcall LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override
		{
			return E_NOTIMPL;
		}
	};
}

#endif