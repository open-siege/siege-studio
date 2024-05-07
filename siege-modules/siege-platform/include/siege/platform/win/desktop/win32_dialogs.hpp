#ifndef WIN32_DIALOGS_HPP
#define WIN32_DIALOGS_HPP
#include <expected>
#include <memory>
#include <filesystem>
#include <shobjidl.h> 
#include <siege/platform/win/core/com.hpp>

namespace win32::com
{
	struct IShellItemEx : ::IShellItem
	{
		std::expected<std::filesystem::path, HRESULT> GetFileSysPath()
		{
			wchar_t* pszFilePath;
		    auto hr = this->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

			if (hr != S_OK)
			{
				return std::unexpected(hr);
			}

			std::filesystem::path temp(pszFilePath);
			CoTaskMemFree(pszFilePath);				
			return temp;
		}
	};

	struct IFileOpenDialogEx : ::IFileOpenDialog
	{
		std::expected<std::unique_ptr<IShellItemEx, com_deleter<IShellItemEx>>, HRESULT> GetResult()
		{
			com_ptr<IShellItem> result(nullptr);

			auto hr = static_cast<IFileOpenDialog*>(this)->GetResult(result.put());

			if (hr != S_OK)
			{
				return std::unexpected(hr);
			}

			return result.as<IShellItemEx>();
		}
	};

	std::expected<std::unique_ptr<IFileOpenDialogEx, com_deleter<IFileOpenDialogEx>>, HRESULT> CreateFileOpenDialog()
	{
		com_ptr<IFileOpenDialog> pFileOpen(nullptr);
					
		auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, __uuidof(::IFileOpenDialog), pFileOpen.put_void());

		if (hr != S_OK)
		{
			return std::unexpected(hr);
		}

		return pFileOpen.as<IFileOpenDialogEx>();
	}

	std::expected<std::unique_ptr<IFileSaveDialog, com_deleter<IFileSaveDialog>>, HRESULT> CreateFileSaveDialog()
	{
		com_ptr<IFileSaveDialog> pFileOpen;
					
		auto hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, __uuidof(::IFileSaveDialog), pFileOpen.put_void());

		if (hr != S_OK)
		{
			return std::unexpected(hr);
		}

		return pFileOpen;
	}
}

#endif // !WIN32_DIALOGS_HPP
