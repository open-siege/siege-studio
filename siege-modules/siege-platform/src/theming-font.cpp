#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <map>
#include <string>

namespace win32
{
  HFONT load_font(LOGFONTW font_info)
  {
    thread_local std::map<std::wstring, win32::auto_handle<HFONT, gdi_deleter>> loaded_fonts;

    if (!font_info.lfCharSet)
    {
      font_info.lfCharSet = DEFAULT_CHARSET;
    }

    if (!font_info.lfQuality)
    {
      font_info.lfCharSet = CLEARTYPE_QUALITY;
    }

    if (!font_info.lfOutPrecision && font_info.lfQuality == CLEARTYPE_QUALITY)
    {
      font_info.lfOutPrecision = OUT_OUTLINE_PRECIS;
    }

    thread_local std::wstring key;
    key.reserve(sizeof(font_info));
    key.assign(key.size(), '\0');

    if (font_info.lfFaceName)
    {
      key.append(font_info.lfFaceName);
    }

    key.append(1, font_info.lfWidth);
    key.append(1, font_info.lfHeight);
    key.append(1, font_info.lfOrientation);
    key.append(1, font_info.lfWeight);
    key.append(1, font_info.lfCharSet);
    key.append(1, font_info.lfOutPrecision);
    key.append(1, font_info.lfQuality);
    key.append(1, font_info.lfPitchAndFamily);

    auto existing_font = loaded_fonts.find(key);

    if (existing_font != loaded_fonts.end())
    {
      return existing_font->second;
    }

    win32::auto_handle<HFONT, gdi_deleter> font(::CreateFontW(font_info.lfHeight,
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
    
    return result.first->second;
  }

  std::optional<SIZE> get_font_size_for_string(HFONT font, std::wstring_view text)
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