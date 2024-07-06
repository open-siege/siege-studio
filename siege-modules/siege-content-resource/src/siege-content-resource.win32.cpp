#include <siege/platform/win/desktop/window_impl.hpp>
#include <siege/platform/win/core/hresult.hpp>
#include "views/vol_view.hpp"

using namespace siege::views;
using storage_info = siege::platform::storage_info;

extern "C"
{
    extern const std::uint32_t default_file_icon = SIID_ZIPFILE;

    std::errc get_window_class_for_stream(storage_info* data, wchar_t** class_name) noexcept
    {
    if (!data)
    {
        return std::errc::invalid_argument;
    }

    if (!class_name)
    {
        return std::errc::invalid_argument;
    }

    static std::wstring empty;
    *class_name = empty.data();

    auto stream = siege::platform::create_istream(*data);

    try
    {
        static auto this_module = win32::window_module_ref::current_module();

        if (siege::views::vol_controller::is_vol(*stream))
        {
        static auto window_type_name = win32::type_name<siege::views::vol_view>();

        if (this_module.GetClassInfoExW(window_type_name))
        {
            *class_name = window_type_name.data();
            return std::errc(0);
        }
        }

        return std::errc::not_supported;
    }
    catch (...)
    {
        return std::errc::not_supported;
    }
    }

    BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,// handle to DLL module
    DWORD fdwReason,// reason for calling function
    LPVOID lpvReserved)// reserved
    {

    if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
    {
        if (lpvReserved != nullptr)
        {
        return TRUE;// do not do cleanup if process termination scenario
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