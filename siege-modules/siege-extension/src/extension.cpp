#include <siege/platform/extension_module.hpp>
#include <siege/platform/win/core/com/client.hpp>
#include <siege/platform/shared.hpp>
#include <oleacc.h>
#include <detours.h>

extern "C"
{	
	HRESULT __stdcall ExecutableIsSupported(const wchar_t* filename) noexcept;

	HRESULT __stdcall LaunchGameWithExtension(const wchar_t* exe_path_str, std::uint32_t argc, const wchar_t** argv, PROCESS_INFORMATION* process_info) noexcept
	{
		if (!exe_path_str)
		{
			return E_POINTER;
		}

		std::filesystem::path exe_path(exe_path_str);

		if (!std::filesystem::exists(exe_path))
		{
			return E_INVALIDARG;
		}

		if (argc > 0 && !argv)
		{
			return E_POINTER;
		}

		if (!process_info)
		{
			return E_POINTER;
		}

		std::string extension_path = win32::module_ref::current_module().GetModuleFileName<char>();

		try
		{
			auto exe_is_supported = ExecutableIsSupported(exe_path.c_str());

			if (exe_is_supported != S_OK)
			{
				return E_INVALIDARG;
			}
		}
		catch(...)
		{
		
		}

		std::wstring args;
		args.reserve(argc + 3 * sizeof(std::wstring) + 3);


		args.append(1, L'"');
		args.append(exe_path.wstring());
		args.append(1, L'"');

		if (argv && argc > 0)
		{
			args.append(1, L' ');
			for (auto i = 0u; i < argc; ++i)
			{
				args.append(argv[i]);

				if (i < (argc - 1))
				{
					args.append(1, L' ');
				}
			}
		}

		STARTUPINFOW startup_info{.cb = sizeof(STARTUPINFOW)};

		if (::DetourCreateProcessWithDllExW(exe_path.c_str(), 
				args.data(), 
				nullptr, 
				nullptr, 
				FALSE, 
				DETACHED_PROCESS, 
				nullptr, 
				exe_path.parent_path().c_str(), 
				&startup_info,
				process_info,
				extension_path.data(), 
				nullptr))
		{		
			return S_OK;
		}

		auto last_error = ::GetLastError();
		return HRESULT_FROM_WIN32(last_error);
	}

	HRESULT __stdcall GetGameScriptHost(_In_ const wchar_t* game, _Outptr_ ::IDispatch** host) noexcept
	{
		if (game == nullptr || host == nullptr)
		{
			return E_POINTER;
		}

		*host = nullptr;

		std::wstring name;
		name.reserve(48);
		name.append(L"siege::extension::");
		name.append(siege::platform::to_lower(game));
		name.append(L"::ScriptHost");

		auto game_window = ::FindWindowExW(HWND_MESSAGE, nullptr, nullptr, name.c_str());

		if (game_window)
		{
			return AccessibleObjectFromWindow(game_window, OBJID_NATIVEOM, IID_IDispatch, (void**)host);
		}

		return OLE_E_NOTRUNNING;
	}
}