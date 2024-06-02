#ifndef WIN32_STREAM_HPP
#define WIN32_STREAM_HPP

#include <istream>
#include <memory>
#include <array>
#include <siege/platform/win/core/com/base.hpp>
#include <objidl.h>

namespace win32::com
{
	class StdStreamRefBase : public ComObject, public ::IStream
	{
	public:
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

		HRESULT __stdcall Read(void* data, ULONG size, ULONG* data_read) override
		{
			return STG_E_ACCESSDENIED;
		}

		HRESULT __stdcall Write(const void* data, ULONG size, ULONG* data_written) override
		{
			return STG_E_ACCESSDENIED;
		}

		HRESULT __stdcall Seek(LARGE_INTEGER offset, DWORD origin, ULARGE_INTEGER* new_position) override
		{
			return STG_E_INVALIDFUNCTION;
		}

		HRESULT __stdcall CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall Stat(STATSTG*, DWORD) override
		{
			return STG_E_ACCESSDENIED;
		}

		HRESULT __stdcall Clone(IStream**) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall SetSize(ULARGE_INTEGER) override
		{
			return STG_E_INVALIDFUNCTION;
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
			return STG_E_INVALIDFUNCTION;
		}

		HRESULT __stdcall UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override
		{
			return STG_E_INVALIDFUNCTION;
		}
	};

	class StdIStreamRef : public virtual StdStreamRefBase
	{
		std::istream& stream;

	public:
		StdIStreamRef(std::iostream& stream) : stream(stream)
		{
		}

		HRESULT __stdcall Read(void* data, ULONG size, ULONG* data_read) override
		{
			if (data == nullptr || data_read == nullptr)
			{
				return STG_E_INVALIDPOINTER;
			}

			if (size == 0)
			{
				*data_read = 0;
				return S_OK;
			}

			*data_read = stream.read((char*)data, size).gcount();

			return size == *data_read ? S_OK : S_FALSE;
		}

		HRESULT __stdcall CopyTo(IStream* target, ULARGE_INTEGER size, ULARGE_INTEGER* data_read, ULARGE_INTEGER* data_written) override
		{
			return E_NOTIMPL;
		}

		HRESULT __stdcall Seek(LARGE_INTEGER offset, DWORD origin, ULARGE_INTEGER* new_position) override
		{
			if (!(origin == STREAM_SEEK_SET || origin == STREAM_SEEK_CUR || origin == STREAM_SEEK_END))
			{
				return STG_E_INVALIDFUNCTION;
			}

			if (origin == STREAM_SEEK_SET)
			{
				ULARGE_INTEGER temp;
				std::memcpy(&temp, &offset, sizeof(ULARGE_INTEGER));
				stream.seekg(static_cast<std::streamoff>(temp.QuadPart), std::ios::beg);
				return S_OK;
			}

			auto direction = origin == STREAM_SEEK_CUR ? std::ios::cur : std::ios::end;

			stream.seekg(static_cast<std::streamoff>(offset.QuadPart), direction);

			return S_OK;
		}

		HRESULT __stdcall Stat(STATSTG*, DWORD) override
		{
			return STG_E_ACCESSDENIED;
		}
	};

	class StdOStreamRef : public virtual StdStreamRefBase
	{
		std::ostream& stream;

	public:
		StdOStreamRef(std::ostream& stream) : stream(stream)
		{
		}

		HRESULT __stdcall Write(const void* data, ULONG size, ULONG* data_written) override
		{
			if (data == nullptr)
			{
				return STG_E_INVALIDPOINTER;
			}

			if (size == 0)
			{
				if (data_written)
				{
					*data_written = 0;
				}
				return S_OK;
			}

			stream.write((const char*)data, size);

			if (stream.bad())
			{
				return STG_E_WRITEFAULT;
			}

			if (data_written)
			{
				*data_written = size;
			}

			return S_OK;
		}

		HRESULT __stdcall SetSize(ULARGE_INTEGER) override
		{
			return STG_E_INVALIDFUNCTION;
		}

		HRESULT __stdcall Seek(LARGE_INTEGER offset, DWORD origin, ULARGE_INTEGER* new_position) override
		{
			if (!(origin == STREAM_SEEK_SET || origin == STREAM_SEEK_CUR || origin == STREAM_SEEK_END))
			{
				return STG_E_INVALIDFUNCTION;
			}

			if (origin == STREAM_SEEK_SET)
			{
				ULARGE_INTEGER temp;
				std::memcpy(&temp, &offset, sizeof(ULARGE_INTEGER));
				stream.seekp(static_cast<std::streamoff>(temp.QuadPart), std::ios::beg);
				return S_OK;
			}

			auto direction = origin == STREAM_SEEK_CUR ? std::ios::cur : std::ios::end;

			stream.seekp(static_cast<std::streamoff>(offset.QuadPart), direction);

			return S_OK;
		}

		HRESULT __stdcall Stat(STATSTG*, DWORD) override
		{
			return STG_E_ACCESSDENIED;
		}

		HRESULT __stdcall Commit(DWORD) override
		{
			stream.flush();
			return S_OK;
		}
	};

	class StdIOStreamRef : public StdIStreamRef, public StdOStreamRef
	{
	public:
		StdIOStreamRef(std::iostream& stream) : StdIStreamRef(stream), StdOStreamRef(stream)
		{
		}

		HRESULT __stdcall Stat(STATSTG* info, DWORD flags) override
		{
			auto first_result = StdIStreamRef::Stat(info, flags);

			if (first_result == S_OK)
			{
				return StdOStreamRef::Stat(info, flags);
			}

			return first_result;
		}

		HRESULT __stdcall Seek(LARGE_INTEGER offset, DWORD origin, ULARGE_INTEGER* new_position) override
		{
			auto first_result = StdIStreamRef::Seek(offset, origin, new_position);

			if (first_result == S_OK)
			{
				return StdOStreamRef::Seek(offset, origin, new_position);
			}

			return first_result;
		}
	};
}

#endif