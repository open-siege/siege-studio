#include <spanstream>
#include <siege/content/sfx/sound.hpp>
#include <siege/platform/win/core/com/base.hpp>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/stream.hpp>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

namespace siege::content::sfx
{
  void init_media_foundation()
  {
    thread_local auto result = ::MFStartup(1);
    thread_local auto closer = std::unique_ptr<HRESULT, void (*)(HRESULT*)>(&result, [](auto*) { ::MFShutdown(); });
  }

  std::vector<std::any> load(std::filesystem::path filename)
  {
    win32::com::com_ptr<IMFSourceReader> reader;
    std::vector<std::any> tracks;

    init_media_foundation();

    auto hresult = ::MFCreateSourceReaderFromURL(filename.c_str(), nullptr, reader.put());

    if (hresult == S_OK)
    {
      tracks.emplace_back(reader);
    }

    return tracks;
  }

  platform_sound::platform_sound(std::filesystem::path filename)
  {
    tracks = load(std::move(filename));
  }

  platform_sound::platform_sound(std::istream& sound_stream)
  {
    if (std::spanstream* span_stream = dynamic_cast<std::spanstream*>(&sound_stream); span_stream != nullptr)
    {
      auto span = span_stream->rdbuf()->span();
      auto view = win32::file_view(span.data());

      auto filename = view.GetMappedFilename();
      view.release();

      if (filename)
      {
        tracks = load(std::move(*filename));
      }
    }

    if (siege::platform::wave::is_wav(sound_stream))
    {
      std::vector<std::byte> sound_data;

      auto size = siege::platform::get_stream_size(sound_stream);
      sound_data.reserve(size);

      std::transform(std::istreambuf_iterator(sound_stream),
        std::istreambuf_iterator<char>(),
        std::back_inserter(sound_data),
        [](char value) {
          return (std::byte)value;
        });

      tracks.emplace_back(std::move(sound_data));
    }
  }

  std::size_t platform_sound::track_count() const
  {
    return tracks.size();
  }

  std::variant<std::monostate, std::filesystem::path, std::span<std::byte>> platform_sound::get_sound_data(std::size_t index)
  {
    if (tracks.empty())
    {
      return {};
    }

    if (index > tracks.size())
    {
      return {};
    }

    auto& item = tracks[index];

    if (item.type().hash_code() == typeid(std::vector<std::byte>).hash_code())
    {
      return std::span<std::byte>(std::any_cast<std::vector<std::byte>&>(item));
    }

    return "";
  }

  // MFStartup
  // MFCreateSourceReaderFromURL
  // MFCreateMFByteStreamOnStream
  // MFCreateMFByteStreamOnStreamEx?
  // MFCreateSourceReaderFromByteStream
  // MFCreateMediaType MF_MT_MAJOR_TYPE MFMediaType_Audio MF_MT_SUBTYPE MFAudioFormat_PCM
  // MFGetAttributeUINT32 MF_MT_AUDIO_BLOCK_ALIGNMENT MF_MT_AUDIO_AVG_BYTES_PER_SECOND
  // MFGetAttributesAsBlob?
  // MFGetAttributesAsBlobSize?
  // MFShutdown

  // MFCreateFile?
  // MFCreateSourceResolver?

  // IMFSample
  //	ConvertToContiguousBuffer IMFMediaBuffer
  // IMFMediaBuffer
  // IMFMediaType
  // IMFSourceReader


}// namespace siege::content::sfx