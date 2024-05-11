#ifndef SIEGEPLUGINHPP
#define SIEGEPLUGINHPP

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <expected>
#include <string>
#include <vector>
#include <libloaderapi.h>
#include <siege/platform/win/core/com_collection.hpp>
#include <siege/platform/win/desktop/window_module.hpp>

namespace siege
{
	class siege_module : public win32::window_module_ref
	{
		using base = win32::window_module_ref;
		HRESULT (__stdcall *GetSupportedExtensionsProc)(win32::com::IReadOnlyCollection** formats) = nullptr;
		HRESULT (__stdcall *GetSupportedFormatCategoriesProc)(LCID, win32::com::IReadOnlyCollection** formats) = nullptr;
		HRESULT (__stdcall *GetSupportedExtensionsForCategoryProc)(const wchar_t* category, win32::com::IReadOnlyCollection** formats) = nullptr;
		HRESULT (__stdcall *IsStreamSupportedProc)(::IStream* data) = nullptr;
		HRESULT (__stdcall *GetWindowClassForStreamProc)(::IStream* data, wchar_t**) = nullptr;

	public: 
		siege_module(std::filesystem::path plugin_path) : base(nullptr)
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

			this->reset(temp);

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

			win32::com::IReadOnlyCollection* raw = nullptr;

			if (GetSupportedExtensionsProc(&raw) == S_OK)
			{
				for (auto& value : *raw)
				{
					results.emplace_back(std::wstring(value));
				}

				assert(raw->Release() == 0);
			}

			return results;
		}

		std::vector<std::wstring> GetSupportedFormatCategories(LCID locale_id) const noexcept
		{
			std::vector<std::wstring> results;

			win32::com::IReadOnlyCollection* raw = nullptr;

			if (GetSupportedFormatCategoriesProc(locale_id, &raw) == S_OK)
			{
				for (auto& value : *raw)
				{
					results.emplace_back(value);
				}
				
				assert(raw->Release() == 0);
			}

			return results;
		}

		std::vector<std::wstring> GetSupportedExtensionsForCategory(const std::wstring& category) const noexcept
		{
			std::vector<std::wstring> results;

			win32::com::IReadOnlyCollection* raw = nullptr;

			if (GetSupportedExtensionsForCategoryProc(category.c_str(), &raw) == S_OK)
			{
				for (auto& value : *raw)
				{
					results.emplace_back(value);
				}

				assert(raw->Release() == 0);
			}

			return results;
		}

		bool IsStreamSupported(IStream& data) const noexcept
		{
			return IsStreamSupportedProc(&data) == S_OK;
		}

		std::wstring GetWindowClassForStream(IStream& data) const noexcept
		{
			wchar_t* result;

			if (GetWindowClassForStreamProc(&data, &result) == S_OK)
			{
				return result;
			}

			return L"";
		}
	};

}


#endif // !SIEGEPLUGINHPP
