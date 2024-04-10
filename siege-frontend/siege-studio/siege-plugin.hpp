#ifndef SIEGEPLUGINHPP
#define SIEGEPLUGINHPP

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <expected>
#include <string>
#include <vector>
#include <libloaderapi.h>
#include "win32_com_collection.hpp"

namespace siege
{
	struct siege_plugin
	{
		std::unique_ptr<HINSTANCE__, void(*)(HINSTANCE)> plugin;

		HRESULT (__stdcall *GetSupportedExtensionsProc)(win32::com::ICollection** formats) = nullptr;
		HRESULT (__stdcall *GetSupportedFormatCategoriesProc)(LCID, win32::com::ICollection** formats) = nullptr;
		HRESULT (__stdcall *GetSupportedExtensionsForCategoryProc)(const wchar_t* category, win32::com::ICollection** formats) = nullptr;
		HRESULT (__stdcall *IsStreamSupportedProc)(::IStream* data) = nullptr;
		HRESULT (__stdcall *GetWindowClassForStreamProc)(::IStream data, wchar_t**) = nullptr;


		siege_plugin(std::filesystem::path plugin_path) : plugin(nullptr, [](auto handle) { assert(FreeLibrary(handle) == TRUE); })
		{
			if (!std::filesystem::exists(plugin_path))
			{
				throw std::invalid_argument("plugin_path");
			}

			auto temp = LoadLibraryW(plugin_path.c_str());

			if (!temp)
			{
				throw std::runtime_error("Could not load dll");
			}

			plugin.reset(temp);

			GetSupportedExtensionsProc = reinterpret_cast<decltype(GetSupportedExtensionsProc)>(::GetProcAddress(temp, "GetSupportedExtensions"));
			GetSupportedFormatCategoriesProc = reinterpret_cast<decltype(GetSupportedFormatCategoriesProc)>(::GetProcAddress(temp, "GetSupportedFormatCategories"));
			GetSupportedExtensionsForCategoryProc = reinterpret_cast<decltype(GetSupportedExtensionsForCategoryProc)>(::GetProcAddress(temp, "GetSupportedExtensionsForCategory"));
			IsStreamSupportedProc = reinterpret_cast<decltype(IsStreamSupportedProc)>(::GetProcAddress(temp, "IsStreamSupported"));
			GetWindowClassForStreamProc = reinterpret_cast<decltype(GetWindowClassForStreamProc)>(::GetProcAddress(temp, "GetWindowClassForStream"));
		

			if (!(GetSupportedExtensionsProc || GetSupportedFormatCategoriesProc || GetSupportedExtensionsForCategoryProc || IsStreamSupportedProc
				|| GetWindowClassForStreamProc
				))
			{
				throw std::runtime_error("Could not find module functions");
			}
		}

		std::vector<std::wstring> GetSupportedExtensions() const noexcept
		{
			std::vector<std::wstring> results;


			return results;
		}

		std::vector<std::wstring> GetSupportedFormatCategories(LCID) const noexcept
		{
			std::vector<std::wstring> results;


			return results;
		}

		std::vector<std::wstring> GetSupportedExtensionsForCategory(const std::wstring&) const noexcept
		{
			std::vector<std::wstring> results;

			return results;
		}

		bool IsStreamSupported(IStream& data) const noexcept
		{
			return IsStreamSupportedProc(&data) == S_OK;
		}

		std::wstring GetWindowClassForStream(IStream& data) const noexcept
		{
			std::wstring result;

			return result;
		}
	};

}


#endif // !SIEGEPLUGINHPP
