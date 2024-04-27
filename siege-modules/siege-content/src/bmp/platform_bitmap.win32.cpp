#include <siege/content/bmp/bitmap.hpp>
#include <exception>

namespace siege::content::bmp
{

    platform_bitmap::platform_bitmap(windows_bmp_data bitmap)
    {
        if (bitmap.info.bit_depth == 24)
        {
            bitmap.info.bit_depth = 4 * 8;
        }

        for (auto& color : bitmap.colours)
        {
            std::swap(color.red, color.blue);
        }

        BITMAPINFO info{};
        static_assert(sizeof(info.bmiHeader) == sizeof(bitmap.info));
        static_assert(std::is_trivially_copyable_v<decltype(bitmap.info)>);
        std::memcpy(&info.bmiHeader, &bitmap.info, sizeof(::BITMAPINFOHEADER));
     

        auto wnd_dc = ::GetDC(nullptr);


        if (bitmap.indexes.empty())
        {
            handle = ::CreateDIBitmap(wnd_dc, &info.bmiHeader, CBM_INIT, bitmap.colours.data(), &info, DIB_RGB_COLORS);
        }
        else
        {
            std::vector<std::uint16_t> indexes(bitmap.indexes.begin(), bitmap.indexes.end());

            handle = ::CreateDIBitmap(wnd_dc, &info.bmiHeader, CBM_INIT, indexes.data(), &info, DIB_PAL_COLORS);
        }

        if (!handle)
        {
            auto error = ::GetLastError();

            if (error == ERROR_INVALID_PARAMETER)
            {
                throw std::invalid_argument("gdi_bitmap bitmap");
            }

            if (error == ERROR_NOT_ENOUGH_MEMORY)
            {
                throw std::bad_alloc();
            }

            if (!error)
            {
                throw std::system_error(std::error_code(error, std::generic_category()));
            }
        }
    }

    platform_bitmap::platform_bitmap(gdi_bitmap bitmap)
    {
        static_assert(sizeof(gdi_bitmap) == sizeof(::BITMAP));
        static_assert(offsetof(gdi_bitmap, gdi_bitmap::type) == offsetof(::BITMAP, ::BITMAP::bmType));
        static_assert(offsetof(gdi_bitmap, gdi_bitmap::width) == offsetof(::BITMAP, ::BITMAP::bmWidth));
        static_assert(offsetof(gdi_bitmap, gdi_bitmap::height) == offsetof(::BITMAP, ::BITMAP::bmHeight));
        static_assert(offsetof(gdi_bitmap, gdi_bitmap::stride) == offsetof(::BITMAP, ::BITMAP::bmWidthBytes));
        static_assert(offsetof(gdi_bitmap, gdi_bitmap::planes) == offsetof(::BITMAP, ::BITMAP::bmPlanes));
        static_assert(offsetof(gdi_bitmap, gdi_bitmap::bit_depth) == offsetof(::BITMAP, ::BITMAP::bmBitsPixel));
        static_assert(offsetof(gdi_bitmap, gdi_bitmap::bytes) == offsetof(::BITMAP, ::BITMAP::bmBits));

        ::BITMAP temp;
        std::memcpy(&temp, &bitmap, sizeof(::BITMAP));

        handle = ::CreateBitmapIndirect(&temp);

        if (!handle)
        {
            auto error = ::GetLastError();

            if (error == ERROR_INVALID_PARAMETER)
            {
                throw std::invalid_argument("gdi_bitmap bitmap");
            }

            if (error == ERROR_NOT_ENOUGH_MEMORY)
            {
                throw std::bad_alloc();
            }

            if (!error)
            {
                throw std::system_error(std::error_code(error, std::generic_category()));
            }
        }
    }

    platform_bitmap::~platform_bitmap()
    {
        ::DeleteObject(handle);
    }
}