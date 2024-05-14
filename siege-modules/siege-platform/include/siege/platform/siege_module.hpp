#ifndef SIEGEPLUGINHPP
#define SIEGEPLUGINHPP

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <expected>
#include <list>
#include <string>
#include <vector>
#include <set>
#include <libloaderapi.h>
#include <siege/platform/win/core/com_collection.hpp>
#include <siege/platform/win/desktop/window_module.hpp>

namespace siege
{
	class siege_module : public win32::window_module
	{
		using base = win32::window_module;
		HRESULT (__stdcall *GetSupportedExtensionsProc)(win32::com::IReadOnlyCollection** formats) = nullptr;
		HRESULT (__stdcall *GetSupportedFormatCategoriesProc)(LCID, win32::com::IReadOnlyCollection** formats) = nullptr;
		HRESULT (__stdcall *GetSupportedExtensionsForCategoryProc)(const wchar_t* category, win32::com::IReadOnlyCollection** formats) = nullptr;
		HRESULT (__stdcall *IsStreamSupportedProc)(::IStream* data) = nullptr;
		HRESULT (__stdcall *GetWindowClassForStreamProc)(::IStream* data, wchar_t**) = nullptr;
		uint32_t DefaultIcon = 0;

	public: 
		siege_module(std::filesystem::path plugin_path) : base(plugin_path)
		{
			GetSupportedExtensionsProc = reinterpret_cast<decltype(GetSupportedExtensionsProc)>(GetProcAddress("GetSupportedExtensions"));
			GetSupportedFormatCategoriesProc = reinterpret_cast<decltype(GetSupportedFormatCategoriesProc)>(GetProcAddress("GetSupportedFormatCategories"));
			GetSupportedExtensionsForCategoryProc = reinterpret_cast<decltype(GetSupportedExtensionsForCategoryProc)>(GetProcAddress("GetSupportedExtensionsForCategory"));
			IsStreamSupportedProc = reinterpret_cast<decltype(IsStreamSupportedProc)>(GetProcAddress("IsStreamSupported"));
			GetWindowClassForStreamProc = reinterpret_cast<decltype(GetWindowClassForStreamProc)>(GetProcAddress("GetWindowClassForStream"));
		
			auto default_icon = GetProcAddress("DefaultFileIcon");

			if (default_icon)
			{
				std::memcpy(&DefaultIcon, default_icon, sizeof(DefaultIcon));
			}

			if (!(GetSupportedExtensionsProc || GetSupportedFormatCategoriesProc || GetSupportedExtensionsForCategoryProc || IsStreamSupportedProc
				|| GetWindowClassForStreamProc
				))
			{
				throw std::runtime_error("Could not find module functions");
			}
		}

		static std::list<siege_module> LoadSiegeModules(std::filesystem::path search_path)
		{
			std::list<siege_module> loaded_modules;

			for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
			{
				if (dir_entry.path().extension() == ".dll")
				{
					try
					{
						loaded_modules.emplace_back(dir_entry.path());
					}
					catch (...)
					{
					}
				}
			}

			return loaded_modules;
		}

		auto GetDefaultFileIcon() const noexcept
		{
			return DefaultIcon;
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

		std::set<std::wstring> GetSupportedFormatCategories(LCID locale_id) const noexcept
		{
			std::set<std::wstring> results;

			win32::com::IReadOnlyCollection* raw = nullptr;

			if (GetSupportedFormatCategoriesProc(locale_id, &raw) == S_OK)
			{
				for (auto& value : *raw)
				{
					results.emplace(value);
				}
				
				assert(raw->Release() == 0);
			}

			return results;
		}

		std::set<std::wstring> GetSupportedExtensionsForCategory(const std::wstring& category) const noexcept
		{
			std::set<std::wstring> results;

			win32::com::IReadOnlyCollection* raw = nullptr;

			if (GetSupportedExtensionsForCategoryProc(category.c_str(), &raw) == S_OK)
			{
				for (auto& value : *raw)
				{
					results.emplace(value);
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
