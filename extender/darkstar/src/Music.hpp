#ifndef DARKSTAR_EXTENDER_MUSIC_HPP
#define DARKSTAR_EXTENDER_MUSIC_HPP

#include <array>
#include <utility>
#include <windows.h>

std::array<std::pair<void**, void*>, 12> GetMusicDetours();


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

static_assert(sizeof(DWORD_PTR) == sizeof(MciFlags), "MciFlags union is too big. Should not be bigger than DWORD_PTR.");

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


#endif// DARKSTAR_EXTENDER_MUSIC_HPP
