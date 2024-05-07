#include <siege/platform/win/desktop/win32_common_controls.hpp>
#include <siege/platform/win/desktop/win32_class.hpp>
#include <bit>
#include <filesystem>
#include <memory>
#include "views/bmp_view.hpp"
#include "views/pal_view.hpp"
#include <siege/platform/win/core/com_collection.hpp>
#include <siege/platform/win/core/com_stream_buf.hpp>
#include <siege/platform/win/desktop/window_module.hpp>

struct pal_mapping_window
{
    constexpr static auto formats = std::array<std::wstring_view, 1>{{L"palettes.settings.json"}};
    
    win32::hwnd_t self;
    pal_mapping_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

    auto on_create(const win32::create_message&)
    {
        /*auto button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						.x = 0,       
						.y = 0,
						.cx = 100,  
						.cy = 100       
						}, self, win32::button::class_name, L"Pal Mapping window");*/
        return 0;
    }

    auto on_size(win32::size_message sized)
	{
		return std::nullopt;
	}
};

extern "C"
{
    HRESULT __stdcall GetSupportedExtensions(_Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
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

;
        *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view>>(supported_extensions).release();

        return S_OK;
    }

    HRESULT __stdcall GetSupportedFormatCategories(_In_ LCID, _Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static auto categories = std::array<std::wstring_view, 2> {{
            L"All Images",
            L"All Palettes"
        }};
        
        *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(categories)>>(categories).release();

        return S_OK;
    }

    HRESULT __stdcall GetSupportedExtensionsForCategory(_In_ const wchar_t* category, _Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!category)
        {
            return E_INVALIDARG;
        }

        if (!formats)
        {
            return E_POINTER;
        }

        std::wstring_view category_str = category;

        if (category_str == L"All Images")
        {
            *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(siege::views::bmp_view::formats)>>(siege::views::bmp_view::formats).release();
        }
        else if (category_str == L"All Palettes")
        {
            *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(siege::views::pal_view::formats)>>(siege::views::pal_view::formats).release();
        }
        else
        {
            *formats = std::make_unique<win32::com::OwningCollection<std::wstring_view>>().release();
        }

        return S_OK;
    }

    _Success_(return == S_OK || return == S_FALSE)
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
    HRESULT __stdcall GetWindowClassForStream(_In_ IStream* data, _Outptr_ wchar_t** class_name) noexcept
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
        
        try
        {
            static auto this_module =  win32::window_module_ref::current_module();
            
            if (siege::views::pal_controller::is_pal(stream))
            {
                static auto window_type_name = win32::type_name<siege::views::pal_view>();

                if (this_module.GetClassInfoExW(window_type_name))
                {
                    *class_name = window_type_name.data();
                    return S_OK;
                }
            }

            if (siege::views::bmp_controller::is_bmp(stream))
            {
                static auto window_type_name = win32::type_name<siege::views::bmp_view>();

                if (this_module.GetClassInfoExW(window_type_name))
                {
                    *class_name = window_type_name.data();
                    return S_OK;
                }
            }

            return S_FALSE;
        }
        catch(...)
        {
            return S_FALSE;
        }
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

            win32::window_module_ref this_module(hinstDLL);

           if (fdwReason == DLL_PROCESS_ATTACH)
           {
               this_module.RegisterClassExW(win32::window_meta_class<siege::views::bmp_view>());
               this_module.RegisterClassExW(win32::window_meta_class<siege::views::pal_view>());
               this_module.RegisterClassExW(win32::window_meta_class<pal_mapping_window>());
            }
            else if (fdwReason == DLL_PROCESS_DETACH)
            {
               this_module.UnregisterClassW<siege::views::bmp_view>();
               this_module.UnregisterClassW<siege::views::pal_view>();
               this_module.UnregisterClassW<pal_mapping_window>();

               this_module.UnregisterClassW<siege::views::bmp_view>();
               this_module.UnregisterClassW<siege::views::pal_view>();
               this_module.UnregisterClassW<pal_mapping_window>();
            }
        }

        return TRUE;
    }
}

