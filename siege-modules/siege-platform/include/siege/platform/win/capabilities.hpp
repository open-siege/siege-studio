#ifndef WIN32_CAPABILITIES_HPP
#define WIN32_CAPABILITIES_HPP
#include <siege/platform/win/com.hpp>
#include <versionhelpers.h>
#include <optional>
#include <cstdint>
#include <d2d1.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <uianimation.h>

namespace win32
{
  struct version
  {
    std::uint8_t major;
    std::uint8_t minor;
    std::uint8_t build;
    std::uint8_t revision;
  };

  inline std::optional<version> get_gdi_plus_version()
  { 
    static std::optional<version> result = [] {
      std::optional<version> result = std::nullopt;
      auto module = ::LoadLibraryExW(L"gdiplus.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        result = {
          .major = 1
        };

        if (::GetProcAddress(module, "GdipConvertToEmfPlusToFile") != nullptr)
        {
          result->minor = 1;
        }

        ::FreeLibrary(module);
        return result;
      }

      return result;
    }();
    return result;
  }

  inline std::optional<version> get_wic_version()
  {
    static std::optional<version> result = [] {
      win32::com::com_ptr<IWICImagingFactory2> temp_2{};

      if (::CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory2), temp_2.put_void()) == S_OK)
      {
        return std::make_optional(version{ .major = 2, .minor = 0 });
      }

      win32::com::com_ptr<IWICImagingFactory> temp_1{};

      if (::CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), temp_1.put_void()) == S_OK)
      {
        return std::make_optional(version{ .major = 1, .minor = 0 });
      }

      return std::optional<version>(std::nullopt);
    }();

    return result;
  }

  inline std::optional<version> get_scenic_animation_version()
  {
    static std::optional<version> result = [] {
      win32::com::com_ptr<IUIAnimationManager2> temp_2{};

      if (::CoCreateInstance(CLSID_UIAnimationManager2, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAnimationManager2), temp_2.put_void()) == S_OK)
      {
        return std::make_optional(version{ .major = 2, .minor = 0 });
      }

      win32::com::com_ptr<IUIAnimationManager> temp_1{};

      if (::CoCreateInstance(CLSID_UIAnimationManager, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAnimationManager), temp_1.put_void()) == S_OK)
      {
        return std::make_optional(version{ .major = 1, .minor = 0 });
      }

      return std::optional<version>(std::nullopt);
    }();

    return result;
  }

  inline std::optional<version> get_direct2d_version()
  {
    static std::optional<version> value = [] {
      using create_d2d_factory = HRESULT __stdcall(D2D1_FACTORY_TYPE factoryType, REFIID riid, D2D1_FACTORY_OPTIONS*, void** factory);

      auto module = ::LoadLibraryExW(L"d2d1.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        std::add_pointer_t<create_d2d_factory> creator = (std::add_pointer_t<create_d2d_factory>)::GetProcAddress(module, "D2D1CreateFactory");

        std::optional<version> result(std::nullopt);
        
        if (creator)
        {
          result = version{
            .major = 1
          };
          win32::com::com_ptr<ID2D1Factory3> factory_1_3;

          if (creator(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), nullptr, factory_1_3.put_void()) == S_OK)
          {
            result->minor = 3;
          }

          win32::com::com_ptr<ID2D1Factory2> factory_1_2 = nullptr;

          if (factory_1_3 == nullptr && creator(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory2), nullptr, factory_1_2.put_void()) == S_OK)
          {
            result->minor = 2;
          }

          win32::com::com_ptr<ID2D1Factory1> factory_1_1 = nullptr;

          if (factory_1_2 == nullptr && creator(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), nullptr, factory_1_1.put_void()) == S_OK)
          {
            result->minor = 1;
          }
        }

        ::FreeLibrary(module);
        return result;
      }

      return std::optional<version>(std::nullopt);
    }();

    return value;
  }

  inline std::optional<version> get_direct_write_version()
  {
    static std::optional<version> result = [] {
      using create_d2d_factory = HRESULT(D2D1_FACTORY_TYPE factoryType, REFIID riid, void** factory);

      auto module = ::LoadLibraryExW(L"dwrite.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        auto* creator = (std::add_pointer_t<decltype(::DWriteCreateFactory)>)::GetProcAddress(module, "DWriteCreateFactory");

        std::optional<version> result;
        if (creator)
        {
          result = {
            .major = 1
          };

          win32::com::com_ptr<IDWriteFactory3> factory_1_3;

          if (creator(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), (IUnknown**)factory_1_3.put()) == S_OK)
          {
            result->minor = 3;
          }

          win32::com::com_ptr<IDWriteFactory2> factory_1_2 = nullptr;

          if (factory_1_3 == nullptr && creator(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory2), (IUnknown**)factory_1_2.put_void()) == S_OK)
          {
            result->minor = 2;
          }

          win32::com::com_ptr<IDWriteFactory1> factory_1_1 = nullptr;

          if (factory_1_2 == nullptr && creator(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), (IUnknown**)factory_1_1.put_void()) == S_OK)
          {
            result->minor = 1;
          }
        }

        ::FreeLibrary(module);
        return result;
      }

      return std::optional<version>(std::nullopt);
    }();

    return result;
  }


}// namespace win32

#endif