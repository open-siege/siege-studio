#ifndef WIN32_DIALOGS_HPP
#define WIN32_DIALOGS_HPP
#include <expected>
#include <memory>
#include <filesystem>
#include <shobjidl.h> 
#include "win32_com.hpp"

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
		std::expected<std::unique_ptr<IShellItemEx, void(*)(IShellItemEx*)>, HRESULT> GetResult()
		{
			IShellItemEx* result = nullptr;

			auto hr = static_cast<IFileOpenDialog*>(this)->GetResult((IShellItem**)&result);

			if (hr != S_OK)
			{
				return std::unexpected(hr);
			}

			return win32::com::as_unique<IShellItemEx>(result);
		}
	};

	std::expected<std::unique_ptr<IFileOpenDialogEx, void(*)(IFileOpenDialogEx*)>, HRESULT> CreateFileOpenDialog()
	{
		IFileOpenDialogEx *pFileOpen;
					
		auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, __uuidof(::IFileOpenDialog), (void**)&pFileOpen);

		if (hr != S_OK)
		{
			return std::unexpected(hr);
		}

		return win32::com::as_unique<IFileOpenDialogEx>(pFileOpen);
	}

	std::expected<std::unique_ptr<IFileSaveDialog, void(*)(IFileSaveDialog*)>, HRESULT> CreateFileSaveDialog()
	{
		IFileSaveDialog *pFileOpen;
					
		auto hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, __uuidof(::IFileSaveDialog), (void**)&pFileOpen);

		if (hr != S_OK)
		{
			return std::unexpected(hr);
		}

		return win32::com::as_unique<IFileSaveDialog>(pFileOpen);
	}
}

#endif // !WIN32_DIALOGS_HPP
