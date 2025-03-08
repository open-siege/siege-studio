#ifndef WIN_DESKTOP_DIRECT_WRITE_HPP
#define WIN_DESKTOP_DIRECT_WRITE_HPP

#include <siege/platform/win/com.hpp>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/drawing.hpp>
#include <string_view>
#include <siege/platform/win/capabilities.hpp>
#include <dwrite.h>

namespace win32::directwrite
{
  using namespace win32::com;
  inline auto& get_factory()
  {
    static auto module = ::LoadLibraryExW(L"dwrite.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

    thread_local com_ptr factory = [] {
      com_ptr<IDWriteFactory> temp;
      auto* creator = (std::add_pointer_t<decltype(::DWriteCreateFactory)>)::GetProcAddress(module, "DWriteCreateFactory");

      if (creator(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)temp.put()) != S_OK)
      {
        throw std::exception("Could not create DirectWrite factory");
      }

      return temp;
    }();

    return *factory;
  }

  class font_collection
  {

  };

  class text_format
  {
  public:
    struct info
    {
      std::wstring_view family_name;
      float size;
      DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
      DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
      DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
      std::wstring_view locale = L"en-us";
      std::optional<font_collection> collection = std::nullopt;
    };

    text_format(info info)
    {
      auto& factory = get_factory();
      thread_local std::wstring temp_name;
      thread_local std::wstring temp_locale;
      temp_name.assign(info.family_name);
      temp_locale.assign(info.locale);
      hresult_throw_on_error(factory.CreateTextFormat(temp_name.c_str(), nullptr, info.weight, info.style, info.stretch, info.size, temp_locale.c_str(), instance.put()));
    }

    IDWriteTextFormat& object()
    {
      return *instance;
    }

  private:
    com_ptr<IDWriteTextFormat> instance;
  };
}// namespace win32::directwrite

#endif