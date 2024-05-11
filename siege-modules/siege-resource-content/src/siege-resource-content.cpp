#include <siege/platform/win/desktop/win32_class.hpp>

#include <bit>
#include <filesystem>
#include <cassert>
#include <atomic>
#include "views/vol_view.hpp"
#include <siege/platform/win/core/com_collection.hpp>
#include <siege/platform/win/core/com_stream_buf.hpp>

extern "C"
{
    const std::uint32_t DefaultFileIcon = SIID_ZIPFILE;

    HRESULT __stdcall GetSupportedExtensions(_Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static std::vector<std::wstring_view> supported_extensions = []{
                std::vector<std::wstring_view> extensions;
                extensions.reserve(32);

                std::copy(siege::views::vol_view::formats.begin(), siege::views::vol_view::formats.end(), std::back_inserter(extensions));
              
                return extensions;
            }();

        *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view>>(supported_extensions).release();

        return S_OK;
    }

    HRESULT __stdcall GetSupportedFormatCategories(_In_ LCID, _Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static auto categories = std::array<std::wstring_view, 1> {{
            L"All Archives"
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

        if (category_str == L"All Archives")
        {
            *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(siege::views::vol_view::formats)>>(siege::views::vol_view::formats).release();
        }
        else
        {
            *formats = std::make_unique<win32::com::OwningCollection<std::wstring_view>>().release();
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

        if (siege::views::vol_controller::is_vol(stream))
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
            
            if (siege::views::vol_controller::is_vol(stream))
            {
                static auto window_type_name = win32::type_name<siege::views::vol_view>();

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
        HINSTANCE hinstDLL,  // handle to DLL module
        DWORD fdwReason,     // reason for calling function
        LPVOID lpvReserved )  // reserved
    {

        if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
        {
            if (lpvReserved != nullptr)
            {
                return TRUE; // do not do cleanup if process termination scenario
            }

            static win32::hwnd_t info_instance = nullptr;

            static std::wstring module_file_name(255, '\0');
            GetModuleFileNameW(hinstDLL, module_file_name.data(), module_file_name.size());

           std::filesystem::path module_path(module_file_name.data());

           win32::window_module_ref this_module(hinstDLL);
           if (fdwReason == DLL_PROCESS_ATTACH)
           {
               this_module.RegisterClassExW(win32::window_meta_class<siege::views::vol_view>());

            }
            else if (fdwReason == DLL_PROCESS_DETACH)
            {
               this_module.UnregisterClassW<siege::views::vol_view>();
            }
        }

        return TRUE;
    }
}