#ifndef SIEGE_STORAGE_MODULE_HPP
#define SIEGE_STORAGE_MODULE_HPP

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <expected>
#include <list>
#include <string>
#include <vector>
#include <set>
#include <siege/platform/win/core/com/collection.hpp>
#include <siege/platform/win/desktop/window_module.hpp>

namespace siege
{
	using StreamIsStorage = HRESULT __stdcall(::IStream*) noexcept;
	using CreateStorageFromStream = HRESULT __stdcall(::IStream*, ::IStorage**);

	class storage_module : public win32::module
	{
		using base = win32::module;

		StreamIsStorage* StreamIsStorageProc = nullptr;
		CreateStorageFromStream* CreateStorageFromStreamProc = nullptr;
	public: 
		storage_module(std::filesystem::path module_path) : base(module_path)
		{
			StreamIsStorageProc = GetProcAddress<decltype(StreamIsStorageProc)>("StreamIsStorage"));
			CreateStorageFromStreamProc = GetProcAddress<decltype(CreateStorageFromStreamProc)>("CreateStorageFromStream"));
	
			if (!(StreamIsStorageProc || CreateStorageFromStreamProc))
			{
				throw std::runtime_error("Could not find module functions");
			}
		}

		static std::list<storage_module> load_modules(std::filesystem::path search_path)
		{
			std::list<storage_module> loaded_modules;

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
	};

}


#endif // !SIEGEPLUGINHPP
