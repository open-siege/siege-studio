#ifndef OPEN_SIEGE_MEDIA_MODULE_HPP
#define OPEN_SIEGE_MEDIA_MODULE_HPP

#include <siege/platform/win/module.hpp>
#include <filesystem>
#include <span>
#include <mmsystem.h>

#undef PlaySound

namespace siege
{
  struct media_module : private win32::module
  {
    media_module() : win32::module("winmm.dll", true)
    {
      play_sound_ascii = this->GetProcAddress<decltype(play_sound_ascii)>("PlaySoundA");
      play_sound_utf = this->GetProcAddress<decltype(play_sound_utf)>("PlaySoundW");

      if (play_sound_ascii == nullptr || play_sound_utf == nullptr)
      {
        throw std::exception("Could not load play sound functions");
      }
    }

    bool PlaySound(std::filesystem::path path, bool async = true, bool loop = false)
    {
      auto flag = SND_NODEFAULT | SND_FILENAME;

      if (async)
      {
        flag |= SND_ASYNC;
      }

      if (loop)
      {
        flag |= SND_LOOP;
      }

      return play_sound_utf(path.c_str(), nullptr, flag);
    }

    bool PlaySound(std::span<std::byte> data, bool async = true, bool loop = false)
    {
      auto flag = SND_NODEFAULT | SND_MEMORY;

      if (async)
      {
        flag |= SND_ASYNC;
      }

      if (loop)
      {
        flag |= SND_LOOP;
      }

      return play_sound_ascii((char*)data.data(), nullptr, flag);
    }

    bool StopSound()
    {
      return play_sound_utf(nullptr, nullptr, 0);
    }

  private:
    std::add_pointer_t<decltype(::PlaySoundA)> play_sound_ascii;
    std::add_pointer_t<decltype(::PlaySoundW)> play_sound_utf;
  };
}// namespace siege

#endif