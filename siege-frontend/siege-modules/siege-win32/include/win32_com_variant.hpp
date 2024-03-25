#ifndef WIN32_COM_VARIANT_HPP
#define WIN32_COM_VARIANT_HPP

#include <oaidl.h>

namespace win32::com
{
    //CreateStreamOverRandomAccessStream 
    //CreateStreamOnHGlobal 
    //SHCreateMemStream 
    //CreateILockBytesOnHGlobal 
    //StgCreateDocfileOnILockBytes
    //CreateFile2 
    //CreateFile2FromAppW 
    //OpenFileMappingFromApp 
    //MapViewOfFile3FromApp
    //MapViewOfFileFromApp
    //CreateFileMappingFromApp
    //MFCreateCollection 
    //MFCreateMFByteStreamOnStreamEx 
    //IWICStream 
    //CLSID_WICImagingFactory
    //CreateXmlReader
    //CreateXmlWriter

    struct OleVariant
    {
        VARIANT variant;

        OleVariant() noexcept
        {
            VariantInit(&variant);
        }

        ~OleVariant() noexcept
        {
            VariantClear(&variant);
        }
    };
}

#endif