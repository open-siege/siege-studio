#include "Mixer.hpp"

UINT WINAPI DarkMixerGetNumDevs(decltype(mixerGetNumDevs) trueMixerGetNumDevs)
{
  return trueMixerGetNumDevs() + 1;
}

static auto* TrueMixerGetNumDevs = mixerGetNumDevs;
UINT WINAPI WrappedMixerGetNumDevs()
{
  return DarkMixerGetNumDevs(TrueMixerGetNumDevs);
}
static_assert(std::is_same_v<TrueMixerGetNumDevs, WrappedMixerGetNumDevs>());

UINT WINAPI DarkMixerOpen(decltype(mixerOpen) trueMixerOpen, LPHMIXER mixerOut, UINT index, DWORD_PTR callback, DWORD_PTR instance, DWORD flags)
{
  if (index == 0)
  {
    *mixerOut = our_mixer_id;
    return MMSYSERR_NOERROR;
  }

  return trueMixerOpen(mixerOut, index - 1, callback, instance, flags);
}

static auto* TrueMixerOpen = mixerOpen;
UINT WINAPI WrappedMixerOpen(LPHMIXER mixerOut, UINT index, DWORD_PTR callback, DWORD_PTR instance, DWORD flags)
{
  return DarkMixerOpen(TrueMixerOpen, mixerOut, index, callback, instance, flags);
}
static_assert(std::is_same_v<TrueMixerOpen, DarkMixerOpen>());


UINT WINAPI DarkMixerClose(decltype(mixerClose), HMIXER)
{
  return 0;
}

static auto* TrueMixerClose = mixerClose;
UINT WINAPI WrappedMixerClose(decltype(mixerClose) original, HMIXER mixer)
{
  return DarkMixerClose(original, mixer);
}
static_assert(std::is_same_v<TrueMixerClose, WrappedMixerClose>());


UINT WINAPI DarkMixerGetLineInfoA(decltype(mixerGetLineInfoA) trueMixerGetLineInfoA, HMIXEROBJ mixer, LPMIXERLINEA info, DWORD flags)
{

  if (mixer == our_mixer_obj)
  {
    if (!info)
    {
      return MMSYSERR_INVALPARAM;
    }

    if (flags != MIXER_GETLINEINFOF_COMPONENTTYPE)
    {
      return MMSYSERR_INVALFLAG;
    }

    if (info->dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC)
    {
      return MMSYSERR_INVALPARAM;
    }

    info->dwLineID = our_mixer_line_id;

    return MMSYSERR_NOERROR;
  }
  return trueMixerGetLineInfoA(mixer, info, flags);
}

static auto* TrueMixerGetLineInfoA = mixerGetLineInfoA;
UINT WINAPI WrappedMixerGetLineInfoA(HMIXEROBJ mixer, LPMIXERLINEA info, DWORD flags)
{
  return DarkMixerGetLineInfoA(TrueMixerGetLineInfoA, mixer, info, flags);
}
static_assert(std::is_same_v<TrueMixerGetLineInfoA, WrappedMixerGetLineInfoA>());

UINT WINAPI DarkMixerGetLineControlsA(decltype(mixerGetLineControlsA) trueMixerGetLineControlsA, HMIXEROBJ mixer, LPMIXERLINECONTROLSA info, DWORD flags)
{
  if (mixer == our_mixer_obj)
  {
    if (!info)
    {
      return MMSYSERR_INVALPARAM;
    }

    if (flags != MIXER_GETLINECONTROLSF_ONEBYTYPE)
    {
      return MMSYSERR_INVALFLAG;
    }

    if (info->dwControlType != MIXERCONTROL_CONTROLTYPE_VOLUME)
    {
      return MMSYSERR_INVALPARAM;
    }

    info->dwControlID = our_mixer_control_id;

    return MMSYSERR_NOERROR;
  }

  return trueMixerGetLineControlsA(mixer, info, flags);
}

static auto* TrueMixerGetLineControlsA = mixerGetLineControlsA;
UINT WINAPI WrappedMixerGetLineControlsA(HMIXEROBJ mixer, LPMIXERLINECONTROLSA info, DWORD flags)
{
  return DarkMixerGetLineControlsA(TrueMixerGetLineControlsA, mixer, info, flags);
}
static_assert(std::is_same_v<TrueMixerGetLineControlsA, WrappedMixerGetLineControlsA>());


UINT WINAPI DarkAuxGetVolume(AuxGetVolumeData self, UINT index, LPDWORD out)
{
  if (index == 0)
  {
    if (!out)
    {
      return FALSE;
    }

    if (tracks.empty())
    {
      *out = 0;
    }
    else
    {
      *out = DWORD(self.tracks[0].volume() * std::numeric_limits<std::uint16_t>::max());
    }

    return TRUE;
  }
  return self.func(index - 1, out);
}

std::vector<music_player> tracks;

static AuxGetVolumeData auxGetVolumeData = { auxGetVolume, tracks };
UINT WINAPI WrappedAuxGetVolume(UINT index, LPDWORD out)
{
  return DarkAuxGetVolume(auxGetVolumeData, index, out);
}

UINT WINAPI DarkMixerGetControlDetailsA(decltype(mixerGetControlDetailsA) trueMixerGetControlDetailsA, AuxGetVolumeData auxData, HMIXEROBJ mixer, LPMIXERCONTROLDETAILS info, DWORD flags)
{
  if (mixer == our_mixer_obj)
  {

    if (!info)
    {
      return MMSYSERR_INVALPARAM;
    }

    if (!info->paDetails)
    {
      return MMSYSERR_INVALPARAM;
    }

    if (info->cbDetails < sizeof(MIXERCONTROLDETAILS_UNSIGNED))
    {
      return MMSYSERR_INVALPARAM;
    }

    if (flags != MIXER_SETCONTROLDETAILSF_VALUE || flags != MIXER_GETCONTROLDETAILSF_VALUE)
    {
      return MMSYSERR_INVALFLAG;
    }

    if (info->dwControlID != our_mixer_control_id)
    {
      return MMSYSERR_INVALPARAM;
    }

    MIXERCONTROLDETAILS_UNSIGNED details;
    DarkAuxGetVolume(auxData, 0, &details.dwValue);
    std::memcpy(info->paDetails, &details, sizeof(details));

    return MMSYSERR_NOERROR;
  }


  return trueMixerGetControlDetailsA(mixer, info, flags);
}
static auto* TrueMixerGetControlDetailsA = mixerGetControlDetailsA;
UINT WINAPI WrappedMixerGetControlDetailsA(HMIXEROBJ mixer, LPMIXERCONTROLDETAILS info, DWORD flags)
{
  return DarkMixerGetControlDetailsA(TrueMixerGetControlDetailsA, auxGetVolumeData, mixer, info, flags);
}
static_assert(std::is_same_v<TrueMixerGetControlDetailsA, WrappedMixerGetControlDetailsA>());


UINT WINAPI DarkAuxSetVolume(AuxSetVolumeData self, UINT index, DWORD in)
{
  if (index == 0)
  {
    if (in > std::numeric_limits<std::uint16_t>::max())
    {
      in = std::numeric_limits<std::uint16_t>::max();
    }

    float volume = float(in) / std::numeric_limits<std::uint16_t>::max();

    for (auto& track : self.tracks)
    {
      track.volume(volume);
    }

    return TRUE;
  }

  return self.func(index - 1, in);
}

static AuxSetVolumeData auxSetVolumeData = { auxSetVolume, tracks };
UINT WINAPI WrappedAuxSetVolume(UINT index, DWORD in)
{
  return DarkAuxSetVolume(auxSetVolumeData, index, in);
}
static_assert(std::is_same_v<TrueAuxSetVolume, WrappedAuxSetVolume>());


UINT WINAPI DarkMixerSetControlDetailsA(decltype(mixerSetControlDetails) trueMixerSetControlDetails, AuxSetVolumeData auxData, HMIXEROBJ mixer, LPMIXERCONTROLDETAILS info, DWORD flags)
{
  if (mixer == our_mixer_obj)
  {

    if (!info)
    {
      return MMSYSERR_INVALPARAM;
    }

    if (!info->paDetails)
    {
      return MMSYSERR_INVALPARAM;
    }

    if (info->cbDetails < sizeof(MIXERCONTROLDETAILS_UNSIGNED))
    {
      return MMSYSERR_INVALPARAM;
    }

    if (flags != MIXER_SETCONTROLDETAILSF_VALUE || flags != MIXER_GETCONTROLDETAILSF_VALUE)
    {
      return MMSYSERR_INVALFLAG;
    }

    if (info->dwControlID != our_mixer_control_id)
    {
      return MMSYSERR_INVALPARAM;
    }

    MIXERCONTROLDETAILS_UNSIGNED details;

    std::memcpy(&details, info->paDetails, sizeof(details));

    DarkAuxSetVolume(auxData, details.dwValue);

    return MMSYSERR_NOERROR;
  }


  return trueMixerSetControlDetails(mixer, info, flags);
}
static auto* TrueMixerSetControlDetails = mixerSetControlDetails;
UINT WINAPI WrappedMixerSetControlDetailsA(HMIXEROBJ mixer, LPMIXERCONTROLDETAILS info, DWORD flags)
{
  return DarkMixerSetControlDetailsA(TrueMixerSetControlDetails, auxSetVolumeData, mixer, info, flags);
}
static_assert(std::is_same_v<TrueMixerSetControlDetails, WrappedMixerSetControlDetailsA>());

UINT WINAPI DarkAuxGetNumDevs(decltype(auxGetNumDevs) trueAuxGetNumDevs)
{
  return trueAuxGetNumDevs() + 1;
}

static auto* TrueAuxGetNumDevs = auxGetNumDevs;
UINT WINAPI WrappedAuxGetNumDevs()
{
  return DarkAuxGetNumDevs(TrueAuxGetNumDevs);
}

UINT WINAPI DarkAuxGetDevCapsA(decltype(auxGetDevCapsA) trueAuxGetDevCapsA, UINT_PTR index, LPAUXCAPSA out, UINT size)
{
  if (index == 0)
  {
    if (!out)
    {
      return FALSE;
    }

    if (size == 0)
    {
      return FALSE;
    }

    out->wTechnology = AUXCAPS_CDAUDIO;
    out->dwSupport = AUXCAPS_VOLUME;

    return TRUE;
  }

  return trueAuxGetDevCapsA(index - 1, out, size);
}

static auto* TrueAuxGetDevCapsA = auxGetDevCapsA;
UINT WINAPI WrappedAuxGetDevCapsA(decltype(auxGetDevCapsA) trueAuxGetDevCapsA, UINT_PTR index, LPAUXCAPSA out, UINT size)
{
  return DarkAuxGetDevCapsA(TrueAuxGetDevCapsA, index, out, size);
}

std::array<std::pair<void**, void*>, 11> GetMixerDetours()
{
  return std::array<std::pair<void**, void*>, 11>{ { { &(void*&)TrueMixerGetNumDevs, WrappedMixerGetNumDevs },
                                                     { &(void*&)TrueMixerOpen, WrappedMixerOpen },
                                                     { &(void*&)TrueMixerClose, WrappedMixerClose },
                                                     { &(void*&)TrueMixerGetLineInfoA, WrappedMixerGetLineInfoA },
                                                     { &(void*&)TrueMixerGetLineControlsA, WrappedMixerGetLineControlsA },
                                                     { &(void*&)TrueMixerGetControlDetailsA, WrappedMixerGetControlDetailsA },
                                                     { &(void*&)TrueMixerSetControlDetails, WrappedMixerSetControlDetailsA },
                                                     { &(void*&)TrueAuxGetNumDevs, WrappedAuxGetNumDevs },
                                                     { &(void*&)TrueAuxGetDevCapsA, WrappedAuxGetDevCapsA },
                                                     { &(void*&)TrueAuxGetVolume, WrappedAuxGetVolume },
                                                     { &(void*&)TrueAuxSetVolume, WrappedAuxSetVolume } } };
}
