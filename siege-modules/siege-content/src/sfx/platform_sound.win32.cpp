#include <siege/content/sfx/sound.hpp>
#include <mfapi.h>

//MFStartup
//MFCreateSourceReaderFromURL
//MFCreateMFByteStreamOnStream
//MFCreateMFByteStreamOnStreamEx?
//MFCreateSourceReaderFromByteStream
//MFCreateMediaType MF_MT_MAJOR_TYPE MFMediaType_Audio MF_MT_SUBTYPE MFAudioFormat_PCM
//MFGetAttributeUINT32 MF_MT_AUDIO_BLOCK_ALIGNMENT MF_MT_AUDIO_AVG_BYTES_PER_SECOND
//MFGetAttributesAsBlob?
//MFGetAttributesAsBlobSize?
//MFShutdown

//MFCreateFile?
//MFCreateSourceResolver?

//IMFSample 
//	ConvertToContiguousBuffer IMFMediaBuffer
//IMFMediaBuffer 
//IMFMediaType 
//IMFSourceReader 


namespace siege::content::sfx
{
	platform_sound::platform_sound(std::filesystem::path)
	{
	}

	platform_sound::platform_sound(std::istream&)
	{
	}
}