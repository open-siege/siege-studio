#ifndef DARKSTAR_EXTENDER_MIXER_HPP
#define DARKSTAR_EXTENDER_MIXER_HPP

#include <music_player.hpp>
#define NOMINMAX
#include <windows.h>

struct AuxGetVolumeData
{
  decltype(auxGetVolume) func;
  const std::vector<music_player> &tracks;
};

struct AuxSetVolumeData
{
  decltype(auxSetVolume) func;
  std::vector<music_player> &tracks;
};

std::array<std::pair<void**, void*>, 11> GetMixerDetours();

UINT WINAPI DarkMixerGetNumDevs(decltype(mixerGetNumDevs));
UINT WINAPI DarkMixerOpen(decltype(mixerOpen), LPHMIXER mixerOut, UINT index, DWORD_PTR callback, DWORD_PTR instance, DWORD flags);
UINT WINAPI DarkMixerClose(decltype(mixerClose), HMIXER);
UINT WINAPI DarkMixerGetLineInfoA(decltype(mixerGetLineInfoA), HMIXEROBJ mixer, LPMIXERLINEA info, DWORD flags);
UINT WINAPI DarkMixerGetLineControlsA(decltype(mixerGetLineControlsA), HMIXEROBJ mixer, LPMIXERLINECONTROLSA info, DWORD flags);
UINT WINAPI DarkMixerGetControlDetailsA(decltype(mixerGetControlDetailsA), AuxGetVolumeData auxData, HMIXEROBJ mixer, LPMIXERCONTROLDETAILS info, DWORD flags);
UINT WINAPI DarkMixerSetControlDetailsA(decltype(mixerSetControlDetails), AuxSetVolumeData auxData, HMIXEROBJ mixer, LPMIXERCONTROLDETAILS info, DWORD flags);
UINT WINAPI DarkAuxGetNumDevs(decltype(auxGetNumDevs));
UINT WINAPI DarkAuxGetDevCapsA(decltype(auxGetDevCapsA), UINT_PTR index, LPAUXCAPSA out, UINT size);
UINT WINAPI DarkAuxGetVolume(AuxGetVolumeData self, UINT index, LPDWORD out);
UINT WINAPI DarkAuxSetVolume(AuxSetVolumeData self, UINT index, DWORD in);




#endif// DARKSTAR_EXTENDER_MIXER_HPP
