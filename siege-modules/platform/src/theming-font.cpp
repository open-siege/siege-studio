#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/drawing.hpp>
#include <map>
#include <string>

namespace win32
{
  namespace
  {
    LONG resolve_default_font_height(UINT dpi)
    {
      NONCLIENTMETRICSW ncm{ .cbSize = sizeof(NONCLIENTMETRICSW) };

      HMODULE user32 = nullptr;
      ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, L"user32.dll", &user32);
      auto sysparam_for_dpi = (std::add_pointer_t<decltype(::SystemParametersInfoForDpi)>)::GetProcAddress(user32, "SystemParametersInfoForDpi");

      if (sysparam_for_dpi && sysparam_for_dpi(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0, dpi))
      {
        return ncm.lfMessageFont.lfHeight;
      }

      if (::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0))
      {
        return ncm.lfMessageFont.lfHeight;
      }

      return -::MulDiv(9, dpi, USER_DEFAULT_SCREEN_DPI);
    }
  }

  gdi::font_ref load_font(LOGFONTW font_info, std::wstring_view font_name)
  {
    thread_local std::map<std::wstring, gdi::font> loaded_fonts;

    UINT dpi = win32::get_current_dpi();

    if (font_info.lfHeight == 0)
    {
      font_info.lfHeight = resolve_default_font_height(dpi);
    }
    else if (font_info.lfHeight > 0)
    {
      font_info.lfHeight = ::MulDiv(font_info.lfHeight, dpi, USER_DEFAULT_SCREEN_DPI);
    }

    if (!font_info.lfCharSet)
    {
      font_info.lfCharSet = DEFAULT_CHARSET;
    }

    if (!font_info.lfQuality)
    {
      font_info.lfQuality = CLEARTYPE_QUALITY;
    }

    if (!font_info.lfOutPrecision && font_info.lfQuality == CLEARTYPE_QUALITY)
    {
      font_info.lfOutPrecision = OUT_OUTLINE_PRECIS;
    }

    if (!font_name.empty() && font_name.size() < 32)
    {
      std::memcpy(font_info.lfFaceName, font_name.data(), font_name.size() * sizeof(wchar_t));
    }

    thread_local std::wstring key;
    key.reserve(sizeof(font_info));
    key.clear();

    if (font_info.lfFaceName)
    {
      key.append(font_info.lfFaceName);
    }

    key.append(1, (wchar_t)font_info.lfWidth);
    key.append(1, (wchar_t)font_info.lfHeight);
    key.append(1, (wchar_t)font_info.lfOrientation);
    key.append(1, (wchar_t)font_info.lfWeight);
    key.append(1, (wchar_t)font_info.lfCharSet);
    key.append(1, (wchar_t)font_info.lfOutPrecision);
    key.append(1, (wchar_t)font_info.lfQuality);
    key.append(1, (wchar_t)font_info.lfPitchAndFamily);

    auto existing_font = loaded_fonts.find(key);

    if (existing_font != loaded_fonts.end())
    {
      return gdi::font_ref(existing_font->second);
    }

    gdi::font font(::CreateFontW(font_info.lfHeight,
      font_info.lfWidth,
      font_info.lfEscapement,
      font_info.lfOrientation,
      font_info.lfWeight,
      font_info.lfItalic,
      font_info.lfUnderline,
      font_info.lfStrikeOut,
      font_info.lfCharSet,
      font_info.lfOutPrecision,
      font_info.lfClipPrecision,
      font_info.lfQuality,
      font_info.lfPitchAndFamily,
      font_info.lfFaceName));
    auto result = loaded_fonts.emplace(key, std::move(font));
    
    return gdi::font_ref(result.first->second);
  }

  std::optional<SIZE> get_font_size_for_string(gdi::font_ref font, std::wstring_view text)
  {
    auto mem_dc = CreateCompatibleDC(GetDC(nullptr));
    auto old_font = SelectFont(mem_dc, font);
    SIZE char_size{};
    auto result = GetTextExtentPoint32W(mem_dc, text.data(), text.size(), &char_size);

    SelectFont(mem_dc, old_font);
    DeleteDC(mem_dc);

    if (result)
    {
      return char_size;
    }

    return std::nullopt;

  }
}// namespace win32