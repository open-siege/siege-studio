#include <string_view>
#include <vector>
#include <future>
#include <filesystem>
#include <algorithm>
#include <numeric>
#include <limits>
#include <cstdint>
#include <cstring>
#include <music_player.hpp>
#include "Music.hpp"


template<typename Dest, typename Src>
Dest copy(Src* in)
{
  static_assert(sizeof(Src) >= sizeof(Dest), "Src should be bigger or the same size as dest");
  Dest temp;
  std::memcpy(&temp, in, sizeof(Dest));
  return temp;
}


namespace fs = std::filesystem;
static std::vector<music_player> tracks;
const static auto our_device_id = copy<MCIDEVICEID>(&tracks);
const static auto our_mixer_id = copy<HMIXER>(&tracks);
const static auto our_mixer_obj = copy<HMIXEROBJ>(&tracks);
const static auto our_mixer_line_id = copy<DWORD>(&tracks);
const static auto our_mixer_control_id = copy<DWORD>(&tracks);
const static auto cd_audio = std::string_view(reinterpret_cast<LPCSTR>(MCI_DEVTYPE_CD_AUDIO));

static auto* TrueMciSendCommandA = mciSendCommandA;
static auto* TrueMciGetErrorStringA = mciGetErrorStringA;


static std::atomic_bool is_playing = false;
static std::future<void> currently_playing;

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
  if (message != MciMessage::open && device_id != our_device_id)
  {
    return WrappedMciSendCommandA(device_id, message, flags, message_data);
  }

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

std::array<std::pair<void**, void*>, 2> GetMusicDetours()
{
  return std::array<std::pair<void**, void*>, 2>{ { { &(void*&)TrueMciSendCommandA, DarkMciSendCommandA },
    { &(void*&)TrueMciGetErrorStringA, DarkMciGetErrorStringA } } };
}

std::size_t number_of_files_in_directory(const std::filesystem::path& path)
{
  using std::filesystem::directory_iterator;
  return std::distance(directory_iterator(path), directory_iterator{});
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

    auto music_path = fs::current_path() / "music";

    if (tracks.empty() && fs::exists(music_path))
    {
      tracks.reserve(number_of_files_in_directory(music_path));
      for (const std::filesystem::directory_entry& dir_entry : std::filesystem::directory_iterator{ music_path })
      {
        if (dir_entry.is_regular_file())
        {
          tracks.emplace_back();
          auto& track = tracks.back();
          track.load(dir_entry.path());
        }
      }
    }


    message_data->open.wDeviceID = our_device_id;
    return 1;
  }

  return WrappedMciSendCommandA(device_id, message, flags, message_data);
}

DWORD WINAPI SetCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (!message_data)
  {
    return 0;
  }

  DWORD result = 0;
  bool should_open = false;
  bool should_close = false;

  if ((flags.set & MciSetFlags::door_open) == MciSetFlags::door_open)
  {
    should_open = true;
  }

  if ((flags.set & MciSetFlags::door_close) == MciSetFlags::door_close)
  {
    should_close = true;
  }

  if (should_open && should_close)
  {
    return 0;
  }

  if (should_open || should_close)
  {
    std::for_each(tracks.begin(), tracks.end(), [](auto& track) { track.stop(); });
    result = 1;
  }


  if ((flags.set & MciSetFlags::time_format) == MciSetFlags::time_format && message_data->set.dwTimeFormat == MCI_FORMAT_MILLISECONDS)
  {
    result = 1;
  }

  return result;
}

DWORD WINAPI CloseCommand(MCIDEVICEID, MciMessage, MciFlags, MciMessageData*)
{
  std::for_each(tracks.begin(), tracks.end(), [](auto& track) { track.stop(); });
  tracks.clear();
  return 1;
}

DWORD WINAPI StatusCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  if (!message_data)
  {
    return 0;
  }

  switch (static_cast<MciStatusCommand>(message_data->status.dwItem))
  {
  case MciStatusCommand::mode: {
    if (tracks.empty())
    {
      message_data->status.dwReturn = static_cast<DWORD>(MciStatusMode::stopped);
      return 1;
    }

    auto playing = std::find_if(tracks.begin(), tracks.end(), [](const auto& track) { return track.state() == music_player_state::playing; });

    if (playing != tracks.end())
    {
      message_data->status.dwReturn = static_cast<DWORD>(MciStatusMode::playing);
      return 1;
    }

    auto paused = std::find_if(tracks.begin(), tracks.end(), [](const auto& track) { return track.state() == music_player_state::paused; });

    if (paused != tracks.end())
    {
      message_data->status.dwReturn = static_cast<DWORD>(MciStatusMode::paused);
      return 1;
    }

    message_data->status.dwReturn = static_cast<DWORD>(MciStatusMode::stopped);
    return 1;
  }
  case MciStatusCommand::track_count:
    message_data->status.dwReturn = static_cast<DWORD>(tracks.size());
    return 1;
  case MciStatusCommand::position: {
    auto end = tracks.end();
    if ((flags.status & MciStatusFlags::track) == MciStatusFlags::track)
    {
      if (message_data->status.dwTrack > tracks.size())
      {
        return 0;
      }

      std::advance(tracks.begin(), message_data->status.dwTrack);
    }
    else
    {
      end = std::find_if(tracks.begin(), tracks.end(), [](const auto& track) { return track.state() == music_player_state::playing || track.state() == music_player_state::paused; });
    }

    if (end == tracks.end())
    {
      message_data->status.dwReturn = 0;
      return 1;
    }

    auto length = std::accumulate(tracks.begin(), end, 0u, [](const auto result, const auto& track) { return result + track.length(); });

    message_data->status.dwReturn = static_cast<DWORD>(length + end->tell());
    return 1;
  }
  case MciStatusCommand::length: {
    if ((flags.status & MciStatusFlags::track) == MciStatusFlags::track)
    {
      if (message_data->status.dwTrack > tracks.size())
      {
        return 0;
      }

      message_data->status.dwReturn = static_cast<DWORD>(tracks[message_data->status.dwTrack].length());
      return 1;
    }

    auto length = std::accumulate(tracks.begin(), tracks.end(), 0u, [](const auto result, const auto& track) { return result + track.length(); });
    message_data->status.dwReturn = static_cast<DWORD>(length);
    return 1;
  }
  }
  return 1;
}

DWORD WINAPI PlayCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  MciMessageData defaults;

  auto length = std::accumulate(tracks.begin(), tracks.end(), 0u, [](const auto result, const auto& track) { return result + track.length(); });

  if (!message_data)
  {
    defaults.play = MCI_PLAY_PARMS{};
    defaults.play.dwFrom = 0;

    defaults.play.dwTo = length;
    message_data = &defaults;
  }

  if (defaults.play.dwFrom < defaults.play.dwTo)
  {
    return 0;
  }

  if (defaults.play.dwTo > length)
  {
    defaults.play.dwTo = length;
  }

  if (defaults.play.dwFrom > length)
  {
    defaults.play.dwFrom = 0;
  }

  std::for_each(tracks.begin(), tracks.end(), [](auto& track) { track.pause(); });

  std::vector<std::tuple<std::size_t, std::size_t, std::reference_wrapper<music_player>>> mapped_tracks;
  mapped_tracks.reserve(tracks.size());


  std::size_t position = 0;

  for (auto i = 0u; i < tracks.size(); ++i)
  {
    mapped_tracks.emplace_back(std::make_tuple(position, position + tracks[i].length(), std::ref(tracks[i])));
    position += tracks[i].length();
  }

  is_playing = true;
  currently_playing = std::async(std::launch::async, [play = defaults.play, mapped_tracks = std::move(mapped_tracks)]() {
    if (!is_playing)
    {
      return;
    }

    if (mapped_tracks.empty())
    {
      return;
    }

    auto first = mapped_tracks.begin();
    auto last = mapped_tracks.end();

    first = std::find_if(mapped_tracks.begin(), mapped_tracks.end(), [&play](auto& track) {
      auto [start, end, ref] = track;
      return end - play.dwFrom > 0;
    });

    if (first == mapped_tracks.end())
    {
      return;
    }

    last = std::find_if(first, mapped_tracks.end(), [&play](auto& track) {
      auto [start, end, ref] = track;
      return end <= play.dwTo;
    });

    if (last != mapped_tracks.end())
    {
      std::advance(last, 1);
    }

    auto start = 0u;

    std::get<2>(*first).get().seek(play.dwFrom - start);

    for (; first != last; ++first)
    {
      auto [start, end, ref] = *first;

      auto delta = end - play.dwTo;

      auto wait_time = delta > 0 ? delta : ref.get().length() - ref.get().tell();

      ref.get().play();
      std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));

      if (delta <= 0)
      {
        ref.get().stop();
      }

      if (!is_playing)
      {
        break;
      }
    }
  });


  return 1;
}

DWORD WINAPI StopCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  is_playing = false;
  std::for_each(tracks.begin(), tracks.end(), [](auto& track) { track.stop(); });
  return 1;
}

DWORD WINAPI PauseCommand(MCIDEVICEID device_id, MciMessage message, MciFlags flags, MciMessageData* message_data)
{
  is_playing = false;
  std::for_each(tracks.begin(), tracks.end(), [](auto& track) { track.pause(); });
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
