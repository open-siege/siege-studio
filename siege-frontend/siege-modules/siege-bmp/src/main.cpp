#include <win32_controls.hpp>
#include <bit>
#include <filesystem>
#include "views/bmp_view.hpp"
#include "views/pal_view.hpp"
#include "win32_com_server.hpp"
#include "win32_com_collection.hpp"
#include "win32_stream_buf.hpp"

struct pal_mapping_window
{
    constexpr static auto formats = std::array<std::wstring_view, 1>{{L"palettes.settings.json"}};
    
    win32::hwnd_t self;
    pal_mapping_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

    auto on_create(const win32::create_message&)
    {
        auto button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						.x = 0,       
						.y = 0,
						.cx = 100,  
						.cy = 100       
						}, self, win32::button::class_name, L"Pal Mapping window");
        return 0;
    }

    auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(self, [&](auto child) {
			win32::SetWindowPos(child, sized.client_size);
		});

		return std::nullopt;
	}
};

extern "C"
{
    HRESULT __stdcall GetSupportedExtensions(_Outptr_ win32::com::ICollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static std::vector<std::wstring_view> supported_extensions = []{
                std::vector<std::wstring_view> extensions;
                extensions.reserve(32);

                std::copy(siege::views::bmp_view::formats.begin(), siege::views::bmp_view::formats.end(), std::back_inserter(extensions));
                std::copy(siege::views::pal_view::formats.begin(), siege::views::pal_view::formats.end(), std::back_inserter(extensions));
                std::copy(pal_mapping_window::formats.begin(), pal_mapping_window::formats.end(), std::back_inserter(extensions));
              
                return extensions;
            }();

        return S_OK;
    }

    HRESULT __stdcall GetSupportedFormatCategories(_In_ LCID, _Outptr_ win32::com::ICollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static auto categories = std::array<std::wstring_view, 2> {{
            L"All Images",
            L"All Palettes"
        }};

//        win32::com::CollectionRef<

        return S_OK;
    }

    HRESULT __stdcall GetSupportedExtensionsForCategory(_In_ const wchar_t* category, _Outptr_ win32::com::ICollection** formats) noexcept
    {
        if (!category)
        {
            return E_INVALIDARG;
        }

        if (!formats)
        {
            return E_POINTER;
        }

        return S_OK;
    }

    HRESULT __stdcall IsStreamSupported(_In_ IStream* data) noexcept
    {
        if (!data)
        {
            return E_INVALIDARG;
        }

        win32::com::StreamBufRef buffer(*data);
        std::istream stream(&buffer);

        if (siege::views::pal_controller::is_pal(stream))
        {
            return S_OK;
        }

        if (siege::views::bmp_controller::is_bmp(stream))
        {
            return S_OK;
        }

        return S_FALSE;
    }

    _Success_(return == S_OK || return == S_FALSE)
    static HRESULT __stdcall GetWindowClassForStream(_In_ IStream* data, _Outptr_ wchar_t** class_name) noexcept
    {
        if (!data)
        {
            return E_INVALIDARG;
        }

        if (!class_name)
        {
            return E_POINTER;
        }

        static std::wstring empty;
        *class_name = empty.data();
        
        win32::com::StreamBufRef buffer(*data);
        std::istream stream(&buffer);

        static HMODULE hModule = []{
            HMODULE temp = nullptr;
            GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
               (LPCWSTR)&GetWindowClassForStream, &temp);

            return temp;
            }();

        if(!hModule)
        {
            return S_FALSE;        
        }

        thread_local WNDCLASSEXW info{};

        if (siege::views::pal_controller::is_pal(stream))
        {
            static auto window_type_name = win32::type_name<siege::views::pal_view>();

            if (::GetClassInfoExW(hModule, window_type_name.c_str(), &info))
            {
                *class_name = window_type_name.data();
                return S_OK;
            }
        }

        if (siege::views::bmp_controller::is_bmp(stream))
        {
            static auto window_type_name = win32::type_name<siege::views::bmp_view>();

            if (::GetClassInfoExW(hModule, window_type_name.c_str(), &info))
            {
                *class_name = window_type_name.data();
                return S_OK;
            }
        }

        return S_FALSE;
    }

    BOOL WINAPI DllMain(
        HINSTANCE hinstDLL,  
        DWORD fdwReason, 
        LPVOID lpvReserved ) noexcept
    {

        if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
        {
            if (lpvReserved != nullptr)
            {
                return TRUE; // do not do cleanup if process termination scenario
            }

           if (fdwReason == DLL_PROCESS_ATTACH)
           {
                win32::RegisterClassExW<siege::views::bmp_view>(WNDCLASSEXW{.hInstance = hinstDLL });
                win32::RegisterClassExW<siege::views::pal_view>(WNDCLASSEXW{.hInstance = hinstDLL});
                win32::RegisterClassExW<pal_mapping_window>(WNDCLASSEXW{.hInstance = hinstDLL});
            }
            else if (fdwReason == DLL_PROCESS_DETACH)
            {
               win32::UnregisterClassW<siege::views::bmp_view>(hinstDLL);
               win32::UnregisterClassW<siege::views::pal_view>(hinstDLL);
               win32::UnregisterClassW<pal_mapping_window>(hinstDLL);
            }
        }

        return TRUE;
    }
}

