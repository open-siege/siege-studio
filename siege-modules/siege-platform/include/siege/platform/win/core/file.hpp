#ifndef WIN_FILE_MODULE_HPP
#define WIN_FILE_MODULE_HPP

#include <siege/platform/win/auto_handle.hpp>
#include <filesystem>
#include <exception>
#include <expected>
#include <string>
#include <wtypes.h>
#include <WinDef.h>
#include <WinBase.h>
#include <psapi.h>

#undef CreateFileMapping

namespace win32
{
	struct handle_deleter
	{
		void operator()(HANDLE lib)
		{
			CloseHandle(lib);
		}
	};

	struct file_view_deleter
	{
		void operator()(void* view)
		{
			::UnmapViewOfFile(view);
		}
	};

	struct file_view : std::unique_ptr<void, file_view_deleter>
	{
		using base = std::unique_ptr<void, file_view_deleter>;
		using base::base;

		std::optional<std::wstring> GetMappedFilename()
		{
			std::wstring filename(255, L'\0');
		    auto size = ::GetMappedFileNameW(::GetCurrentProcess(), get(), filename.data(), filename.size());
			
			if (size == 0)
			{
				return std::nullopt;
			}

			std::wstring drive = L"A:";

            std::wstring buffer(32, L'\0');

            for (auto i = drive[0]; i <= L'Z'; ++i)
            {
                    drive[0] = i;

                    auto vol_size = ::QueryDosDeviceW(drive.c_str(), buffer.data(), buffer.size());

                    if (vol_size != 0)
                    {
                       buffer = buffer.c_str();

                       auto index = filename.find(buffer, 0);

                       if (index == 0)
                       {
                           filename = filename.replace(0, buffer.size(), drive);              
                           break;
                       }
                    }
            }

			filename.erase(filename.find(L'\0'));

			return filename;
		}
	};

	struct file_mapping : std::unique_ptr<void, handle_deleter>
	{
		using base = std::unique_ptr<void, handle_deleter>;
		using base::base;

		file_view MapViewOfFile(DWORD desired_access, std::size_t size)
		{
			return file_view(::MapViewOfFile(get(), desired_access, 0, 0, size));
		}
	};

	struct file : std::unique_ptr<void, handle_deleter>
	{
		using base = std::unique_ptr<void, handle_deleter>;
		using base::base;

		file(std::filesystem::path path, DWORD access, DWORD share_mode, std::optional<SECURITY_ATTRIBUTES> attributes, DWORD creation_disposition, DWORD flags)
			: base(::CreateFileW(path.c_str(), access, share_mode, attributes.has_value() ? &*attributes : nullptr, creation_disposition, flags, nullptr))
		{
			if (this->get() == INVALID_HANDLE_VALUE)
			{
				throw std::system_error(std::error_code(GetLastError(),  std::system_category()));
			}
		}

		std::expected<file_mapping, DWORD> CreateFileMapping(std::optional<SECURITY_ATTRIBUTES> attributes, DWORD protect, DWORD maxSzeHigh, DWORD maxSizeLow, std::wstring name)
		{
			auto mapping = ::CreateFileMappingW(get(), attributes.has_value() ? &*attributes : nullptr, protect, maxSzeHigh, maxSizeLow, name.empty() ? nullptr : name.c_str());

			if (mapping == nullptr)
			{
				return std::unexpected(::GetLastError());
			}

			return file_mapping(mapping);
		}
	};	
}

#endif