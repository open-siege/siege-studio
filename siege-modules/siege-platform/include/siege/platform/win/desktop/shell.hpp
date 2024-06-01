#ifndef WIN32_DIALOGS_HPP
#define WIN32_DIALOGS_HPP
#include <expected>
#include <memory>
#include <filesystem>
#include <shobjidl.h>
#include <commoncontrols.h>
#include <siege/platform/win/core/com/base.hpp>

namespace win32::com
{
	struct ShellItemEx : com_ptr<::IShellItem>
	{
		using com_ptr<::IShellItem>::com_ptr;

		std::expected<std::filesystem::path, HRESULT> GetFileSysPath()
		{
			wchar_t* pszFilePath;
		    auto hr = get()->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

			if (hr != S_OK)
			{
				return std::unexpected(hr);
			}

			std::filesystem::path temp(pszFilePath);
			CoTaskMemFree(pszFilePath);				
			return temp;
		}
	};

	struct FileOpenDialogEx : com_ptr<::IFileOpenDialog>
	{
		using com_ptr<::IFileOpenDialog>::com_ptr;

		std::expected<ShellItemEx, HRESULT> GetResult()
		{
			ShellItemEx result(nullptr);

			auto hr = get()->GetResult(result.put());

			if (hr != S_OK)
			{
				return std::unexpected(hr);
			}

			return result;
		}
	};

	std::expected<FileOpenDialogEx, HRESULT> CreateFileOpenDialog()
	{
		FileOpenDialogEx pFileOpen(nullptr);
					
		auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, __uuidof(::IFileOpenDialog), pFileOpen.put_void());

		if (hr != S_OK)
		{
			return std::unexpected(hr);
		}

		return pFileOpen;
	}

	std::expected<com_ptr<IFileSaveDialog>, HRESULT> CreateFileSaveDialog()
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
