#include <string_view>
#include <vector>
#include <windows.h>
#include <music_player.hpp>
#include "Music.hpp"

static std::vector<int> device;
static std::vector<int> tracks;
const static inline auto our_device_id = reinterpret_cast<MCIDEVICEID>(&device);
const static auto cd_audio = std::string_view(reinterpret_cast<LPCSTR>(MCI_DEVTYPE_CD_AUDIO));

static auto* TrueMciSendCommandA = mciSendCommandA;
static auto* TrueMciGetErrorStringA = mciGetErrorStringA;
static auto* TrueMixerGetNumDevs = mixerGetNumDevs;
static auto* TrueMixerOpen = mixerOpen;
static auto* TrueMixerClose = mixerClose;
static auto* TrueMixerGetLineInfoA = mixerGetLineInfoA;
static auto* TrueMixerGetLineControlsA = mixerGetLineControlsA;
static auto* TrueMixerGetControlDetailsA = mixerGetControlDetailsA;
static auto* TrueAuxGetNumDevs = auxGetNumDevs;
static auto* TrueAuxGetDevCapsA = auxGetDevCapsA;
static auto* TrueAuxGetVolume = auxGetVolume;
static auto* TrueAuxSetVolume = auxSetVolume;

DWORD WINAPI OpenCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data);
DWORD WINAPI SetCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data);
DWORD WINAPI CloseCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data);
DWORD WINAPI StatusCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data);
DWORD WINAPI PlayCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data);
DWORD WINAPI StopCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data);
DWORD WINAPI PauseCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data);
DWORD WINAPI WrappedMciSendCommandA(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data);

// TODO see what the possible errorIDs are
DWORD WINAPI DarkMciSendCommandA(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  switch (message)
  {
  case MciMessage::open:
    return OpenCommand(device_id, message, flags, message_data);
  case MciMessage::set:
    return SetCommand(device_id, message, flags, message_data);
  case MciMessage::close:
    return CloseCommand(device_id, message, flags, message_data);
  case MciMessage::status:
    return StatusCommand(device_id, message, flags, message_data);
  case MciMessage::play:
    return PlayCommand(device_id, message, flags, message_data);
  case MciMessage::stop:
    return StopCommand(device_id, message, flags, message_data);
  case MciMessage::pause:
    return PauseCommand(device_id, message, flags, message_data);
  }

  return WrappedMciSendCommandA(device_id, message, flags, message_data);
}

BOOL WINAPI DarkMciGetErrorStringA(MCIERROR err, LPSTR str, UINT other)
{
  return TrueMciGetErrorStringA(err, str, other);
}

UINT WINAPI DarkMixerGetNumDevs(void)
{
  return TrueMixerGetNumDevs();
}

UINT WINAPI DarkMixerOpen(LPHMIXER, UINT, DWORD_PTR, DWORD_PTR, DWORD)
{
  return 0;
}

UINT WINAPI DarkMixerClose(HMIXER)
{
  return 0;
}

UINT WINAPI DarkMixerGetLineInfoA(HMIXEROBJ, LPMIXERLINEA, DWORD)
{
  return 0;
}

UINT WINAPI DarkMixerGetLineControlsA(HMIXEROBJ, LPMIXERLINECONTROLSA, DWORD)
{
  return 0;
}

UINT WINAPI DarkMixerGetControlDetailsA(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD)
{
  return 0;
}


UINT WINAPI DarkAuxGetNumDevs(void)
{
  return 0;
}

UINT WINAPI DarkAuxGetDevCapsA(UINT_PTR, LPAUXCAPSA, UINT)
{
  return 0;
}

UINT WINAPI DarkAuxGetVolume(UINT, LPDWORD)
{
  return 0;
}

UINT WINAPI DarkAuxSetVolume(UINT, DWORD)
{
  return 0;
}


std::array<std::pair<void**, void*>, 12> GetMusicDetours()
{
  return std::array<std::pair<void**, void*>, 12>{ { { &(void*&)TrueMciSendCommandA, DarkMciSendCommandA },
    { &(void*&)TrueMciGetErrorStringA, DarkMciGetErrorStringA },
    { &(void*&)TrueMixerGetNumDevs, DarkMixerGetNumDevs },
    { &(void*&)TrueMixerOpen, DarkMixerOpen },
    { &(void*&)TrueMixerClose, DarkMixerClose },
    { &(void*&)TrueMixerGetLineInfoA, DarkMixerGetLineInfoA },
    { &(void*&)TrueMixerGetLineControlsA, DarkMixerGetLineControlsA },
    { &(void*&)TrueMixerGetControlDetailsA, DarkMixerGetControlDetailsA },
    { &(void*&)TrueAuxGetNumDevs, DarkAuxGetNumDevs },
    { &(void*&)TrueAuxGetDevCapsA, DarkAuxGetDevCapsA },
    { &(void*&)TrueAuxGetVolume, DarkAuxGetVolume },
    { &(void*&)TrueAuxSetVolume, DarkAuxSetVolume } } };
}

DWORD WINAPI OpenCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (!message_data)
  {
    return 0;
  }

  std::string_view device_type = message_data->open.lpstrDeviceType;

  if (device_type == cd_audio && (flags.open & (MciOpenFlags::open_type | MciOpenFlags::open_type_id)) == (MciOpenFlags::open_type | MciOpenFlags::open_type_id))
  {
    message_data->open.wDeviceID = our_device_id;
    return 1;
  }
  return 1;
}

DWORD WINAPI SetCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (!message_data)
  {
    return 0;
  }

  if (device_id != our_device_id)
  {
    // TODO call the real implementation;
    return 0;
  }

  if ((flags.set & MciSetFlags::door_open) == MciSetFlags::door_open)
  {
    // TODO multiple flags could be passed, so instead of returning, we should set a flag.
    return 1;
  }

  if ((flags.set & MciSetFlags::door_close) == MciSetFlags::door_close)
  {
    // TODO multiple flags could be passed, so instead of returning, we should set a flag.
    return 1;
  }


  if ((flags.set & MciSetFlags::time_format) == MciSetFlags::time_format && message_data->set.dwTimeFormat == MCI_FORMAT_MILLISECONDS)
  {
    // TODO multiple flags could be passed, so instead of returning, we should set a flag.
    return 1;
  }

  return 0;
}

DWORD WINAPI CloseCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (device_id != our_device_id)
  {
    // TODO call the real implementation;
    return 0;
  }

  // TODO close our media device

  return 1;
}

DWORD WINAPI StatusCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (!message_data)
  {
    return 0;
  }

  if (device_id != our_device_id)
  {
    // TODO call the real implementation;
    return 0;
  }

  switch (static_cast<MciStatusCommand>(message_data->status.dwItem))
  {
  case MciStatusCommand::mode:
    // TODO integrate with the music library to get this info.
    message_data->status.dwReturn = static_cast<DWORD>(MciStatusMode::paused);
    return 1;
  case MciStatusCommand::track_count:
    message_data->status.dwReturn = static_cast<DWORD>(tracks.size());
    return 1;
  case MciStatusCommand::position:
    if ((flags.status & MciStatusFlags::track) == MciStatusFlags::track)
    {
      if (message_data->status.dwTrack > tracks.size())
      {
        return 0;
      }

      message_data->status.dwReturn = static_cast<DWORD>(tracks[message_data->status.dwTrack]);
      return 1;
    }

    return 1;
  case MciStatusCommand::length:
    if ((flags.status & MciStatusFlags::track) == MciStatusFlags::track)
    {
      if (message_data->status.dwTrack > tracks.size())
      {
        return 0;
      }

      message_data->status.dwReturn = static_cast<DWORD>(tracks[message_data->status.dwTrack]);
      return 1;
    }

    return 1;
  }

  return 1;
}

DWORD WINAPI PlayCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (device_id != our_device_id)
  {
    // TODO call the real implementation;
    return 0;
  }
  return 1;
}

DWORD WINAPI StopCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (device_id != our_device_id)
  {
    // TODO call the real implementation;
    return 0;
  }
  return 1;
}

DWORD WINAPI PauseCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (device_id != our_device_id)
  {
    // TODO call the real implementation;
    return 0;
  }
  return 1;
}


DWORD WINAPI WrappedMciSendCommandA(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (device_id == our_device_id)
  {
    return 0;
  }
  DWORD_PTR new_flags = 0;
  std::copy(reinterpret_cast<std::byte*>(&flags), reinterpret_cast<std::byte*>(&flags) + sizeof(flags), reinterpret_cast<std::byte*>(&new_flags));

  return TrueMciSendCommandA(device_id, static_cast<UINT>(message), new_flags, reinterpret_cast<DWORD_PTR>(message_data));
}
