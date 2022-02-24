#include <string_view>
#include <vector>
#include <windows.h>
#include <music_player.hpp>
//auxGetNumDevs
//auxGetDevCaps
//auxGetVolume
//auxSetVolume
//mixerGetNumDevs
//mixerOpen
//mixerClose
//mixerGetLineInfo
//mixerGetLineControls
//mixerGetControlDetails
//mciSendCommand
//mciGetErrorString


enum struct MciMessage : UINT
{
  open = MCI_OPEN,
  set = MCI_SET,
  close = MCI_CLOSE,
  status = MCI_STATUS,
  play = MCI_PLAY,
  stop = MCI_STOP,
  pause = MCI_PAUSE
};

union MciMessageData
{
  MCI_OPEN_PARMS open;
  MCI_SET_PARMS set;
  MCI_GENERIC_PARMS close;
  MCI_STATUS_PARMS status;
  MCI_PLAY_PARMS play;
  MCI_GENERIC_PARMS stop;
  MCI_GENERIC_PARMS pause;
};

enum struct MciOpenFlags : DWORD_PTR
{
  open_type = MCI_OPEN_TYPE,
  open_type_id = MCI_OPEN_TYPE_ID
};

inline MciOpenFlags operator|(MciOpenFlags a, MciOpenFlags b)
{
  return static_cast<MciOpenFlags>(static_cast<DWORD_PTR>(a) | static_cast<DWORD_PTR>(b));
}

inline MciOpenFlags operator&(MciOpenFlags a, MciOpenFlags b)
{
  return static_cast<MciOpenFlags>(static_cast<DWORD_PTR>(a) & static_cast<DWORD_PTR>(b));
}

enum struct MciSetFlags : DWORD_PTR
{
  time_format = MCI_SET_TIME_FORMAT,
  door_open = MCI_SET_DOOR_OPEN,
  door_close = MCI_SET_DOOR_CLOSED
};

inline MciSetFlags operator|(MciSetFlags a, MciSetFlags b)
{
  return static_cast<MciSetFlags>(static_cast<DWORD_PTR>(a) | static_cast<DWORD_PTR>(b));
}

inline MciSetFlags operator&(MciSetFlags a, MciSetFlags b)
{
  return static_cast<MciSetFlags>(static_cast<DWORD_PTR>(a) & static_cast<DWORD_PTR>(b));
}

enum struct MciStatusFlags : DWORD_PTR
{
  item_status = MCI_STATUS_ITEM,
  track = MCI_TRACK
};

inline MciStatusFlags operator|(MciStatusFlags a, MciStatusFlags b)
{
  return static_cast<MciStatusFlags>(static_cast<DWORD_PTR>(a) | static_cast<DWORD_PTR>(b));
}

inline MciStatusFlags operator&(MciStatusFlags a, MciStatusFlags b)
{
  return static_cast<MciStatusFlags>(static_cast<DWORD_PTR>(a) & static_cast<DWORD_PTR>(b));
}

union MciFlags
{
  MciOpenFlags open;
  MciSetFlags set;
  MciStatusFlags status;
};

const static auto cd_audio = std::string_view(reinterpret_cast<LPCSTR>(MCI_DEVTYPE_CD_AUDIO));

enum struct MciStatusMode : DWORD_PTR
{
  not_ready = MCI_MODE_NOT_READY,
  paused = MCI_MODE_PAUSE,
  playing = MCI_MODE_PLAY,
  stopped = MCI_MODE_STOP,
  opened = MCI_MODE_OPEN,
  recording = MCI_MODE_RECORD,
  seeking = MCI_MODE_SEEK
};

enum struct MciStatusCommand
{
  mode = MCI_STATUS_MODE,
  position = MCI_STATUS_POSITION,
  track_count = MCI_STATUS_NUMBER_OF_TRACKS,
  length = MCI_STATUS_LENGTH
};


static std::vector<int> device;
static std::vector<int> tracks;
const static inline auto our_device_id = reinterpret_cast<MCIDEVICEID>(&device);

DWORD WINAPI OpenCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (!message_data)
  {
    return 0;
  }

  std::string_view device_type = message_data->open.lpstrDeviceType;

  if(device_type == cd_audio && (flags.open & (MciOpenFlags::open_type | MciOpenFlags::open_type_id)) ==
  (MciOpenFlags::open_type | MciOpenFlags::open_type_id))
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
    //TODO call the real implementation;
    return 0;
  }

  if((flags.set & MciSetFlags::door_open) == MciSetFlags::door_open)
  {
    //TODO multiple flags could be passed, so instead of returning, we should set a flag.
    return 1;
  }

  if((flags.set & MciSetFlags::door_close) == MciSetFlags::door_close)
  {
    //TODO multiple flags could be passed, so instead of returning, we should set a flag.
    return 1;
  }


  if((flags.set & MciSetFlags::time_format) == MciSetFlags::time_format && message_data->set.dwTimeFormat == MCI_FORMAT_MILLISECONDS)
  {
    //TODO multiple flags could be passed, so instead of returning, we should set a flag.
    return 1;
  }

  return 0;
}

DWORD WINAPI CloseCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (device_id != our_device_id)
  {
    //TODO call the real implementation;
    return 0;
  }

  //TODO close our media device

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
    //TODO call the real implementation;
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
      if((flags.status & MciStatusFlags::track) == MciStatusFlags::track)
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
      if((flags.status & MciStatusFlags::track) == MciStatusFlags::track)
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
    //TODO call the real implementation;
    return 0;
  }
  return 1;
}

DWORD WINAPI StopCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (device_id != our_device_id)
  {
    //TODO call the real implementation;
    return 0;
  }
  return 1;
}

DWORD WINAPI PauseCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (device_id != our_device_id)
  {
    //TODO call the real implementation;
    return 0;
  }
  return 1;
}

//TODO see what the possible errorIDs are
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

  return 0;
}

BOOL WINAPI  DarkMciGetErrorStringA(MCIERROR,LPSTR,UINT)
{
  return FALSE;
}

UINT WINAPI DarkMixerGetNumDevs(void)
{
  return 0;
}

UINT WINAPI DarkMixerOpen(LPHMIXER,UINT,DWORD_PTR,DWORD_PTR,DWORD)
{
  return 0;
}

UINT WINAPI DarkMixerClose(HMIXER)
{
  return 0;
}

UINT WINAPI DarkMixerGetLineInfoA(HMIXEROBJ,LPMIXERLINEA,DWORD)
{
  return 0;
}

UINT WINAPI DarkMixerGetLineControlsA(HMIXEROBJ,LPMIXERLINECONTROLSA,DWORD)
{
  return 0;
}

UINT WINAPI DarkMixerGetControlDetailsA(HMIXEROBJ,LPMIXERCONTROLDETAILS,DWORD)
{
  return 0;
}


UINT WINAPI DarkAuxGetNumDevs(void)
{
  return 0;
}

UINT WINAPI DarkAuxGetDevCapsA(UINT_PTR,LPAUXCAPSA,UINT)
{
  return 0;
}

UINT WINAPI DarkAuxSetVolume(UINT,DWORD)
{
  return 0;
}

UINT WINAPI DarkAuxGetVolume(UINT,LPDWORD)
{
  return 0;
}
