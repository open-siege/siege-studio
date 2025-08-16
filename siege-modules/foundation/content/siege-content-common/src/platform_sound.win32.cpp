#include <spanstream>
#include <siege/content/sfx/sound.hpp>
#include <siege/platform/win/com.hpp>
#include <siege/platform/win/file.hpp>
#include <siege/platform/stream.hpp>

namespace siege::content::sfx
{
  platform_sound::platform_sound(std::filesystem::path filename)
  {

  }

  platform_sound::platform_sound(std::istream& sound_stream)
  {
    if (std::spanstream* span_stream = dynamic_cast<std::spanstream*>(&sound_stream); span_stream != nullptr)
    {
      auto span = span_stream->rdbuf()->span();
      auto view = win32::file_view(span.data());

      auto filename = view.GetMappedFilename();
      view.release();
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

}// namespace siege::content::sfx