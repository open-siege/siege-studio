#include <bit>
#include <filesystem>
#include <memory>
#include <siege/platform/win/desktop/window_impl.hpp>
#include <siege/platform/win/core/com/collection.hpp>
#include <siege/platform/win/core/com/stream_buf.hpp>
#include <siege/platform/win/desktop/window_module.hpp>
#include "views/exe_view.hpp"

using namespace siege::views;

extern "C"
{
    extern const std::uint32_t DefaultFileIcon = SIID_APPLICATION;

    HRESULT __stdcall GetSupportedExtensions(_Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static std::vector<std::wstring_view> supported_extensions = []{
                std::vector<std::wstring_view> extensions;
                extensions.reserve(16);

                std::copy(exe_controller::exe_formats.begin(), exe_controller::exe_formats.end(), std::back_inserter(extensions));
                std::copy(exe_controller::lib_formats.begin(), exe_controller::lib_formats.end(), std::back_inserter(extensions));
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
                L"All Executables",
                L"All Libraries",
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

        if (category_str == L"All Executables")
        {
          *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(exe_controller::exe_formats)>>(exe_controller::exe_formats).release();
        }
        else if (category_str == L"All Libraries")
        {
          *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(exe_controller::lib_formats)>>(exe_controller::lib_formats).release();
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

        if (exe_controller::is_exe(stream))
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
            
            if (exe_controller::is_exe(stream))
            {
                static auto window_type_name = win32::type_name<exe_view>();

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
                this_module.RegisterClassExW(win32::window_meta_class<exe_view>());
           }
           else if (fdwReason == DLL_PROCESS_DETACH)
           {
                this_module.UnregisterClassW<exe_view>();
           }
        }

        return TRUE;
    }
}