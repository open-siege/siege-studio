//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_SOUNDFXPLUGIN_HPP
#define DARKSTARHOOK_SOUNDFXPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct SoundFxPlugin : GamePluginWrapper
    {
        bool newSfx()
        {
            return toForcedBool(execute(plugin, 0, { "newSfx" }));
        }

        bool sfxOpen()
        {
            return toForcedBool(execute(plugin, 1, { "sfxOpen" }));
        }

        bool sfxClose()
        {
            return toForcedBool(execute(plugin, 2, { "sfxClose" }));
        }

        bool sfxQuery(int driverId)
        {
            std::string driverIdStr = std::to_string(driverId);
            return toBool(execute(plugin, 3, { "sfxQuery", driverIdStr.c_str() }));
        }

        bool sfxSetFormat(int frequency, int bitDepth, bool isStereo)
        {
            std::string frequencyStr = std::to_string(frequency);
            std::string bitDepthStr = std::to_string(bitDepth);

            return toForcedBool(execute(plugin, 4, { "sfxSetFormat", frequencyStr.c_str(), bitDepthStr.c_str(), toIntString(isStereo) }));
        }

        bool sfxGetFormat()
        {
            return toForcedBool(execute(plugin, 5, { "sfxGetFormat" }));
        }

        void sfxAdd2dProfile(int profileId, float baseVolume, std::optional<std::vector<std::string>> flags)
        {
            std::string profileIdStr = std::to_string(profileId);
            std::string baseVolumeStr = std::to_string(baseVolume);

            if (flags->empty())
            {
                execute(plugin, 6, { "sfxAdd2DProfile", profileIdStr.c_str(), baseVolumeStr.c_str() });
                return;
            }

            std::vector<const char*> arguments;
            arguments.reserve(flags->size() + 3);
            arguments.push_back("sfxAdd2DProfile");
            arguments.push_back(profileIdStr.c_str());
            arguments.push_back(baseVolumeStr.c_str());

            for(auto& arg : *flags)
            {
                arguments.push_back(arg.c_str());
            }

            execute(plugin, 6, arguments);
        }

        void sfxAdd3dProfile(int profileId, float baseVolume, float minDistance, float maxDistance,
                             std::optional<float> coneVolume,
                             std::optional<float> coneInside,
                             std::optional<float> coneOutside,
                             std::optional<std::tuple<float, float, float>> vector,
                             std::optional<std::vector<std::string>> flags)
        {
            std::string profileIdStr = std::to_string(profileId);
            std::string baseVolumeStr = std::to_string(baseVolume);

            if (flags->empty())
            {
                execute(plugin, 7, { "sfxAdd3DProfile", profileIdStr.c_str(), baseVolumeStr.c_str() });
                return;
            }

            std::vector<const char*> arguments;
            arguments.reserve(flags->size() + 3);
            arguments.push_back("sfxAdd3DProfile");
            arguments.push_back(profileIdStr.c_str());
            arguments.push_back(baseVolumeStr.c_str());

            for(auto& arg : *flags)
            {
                arguments.push_back(arg.c_str());
            }

            execute(plugin, 7, arguments);
        }

        void sfxAddPair(bool shouldShow = true)
        {
            execute(plugin, 8, { "sfxAddPair", toStringLower(shouldShow) });
        }

        void sfxRemoveProfile(bool shouldShow = true)
        {
            execute(plugin, 9, { "sfxRemoveProfile", toStringLower(shouldShow) });
        }

        void sfxRemovePair(bool shouldShow = true)
        {
            execute(plugin, 10, { "sfxRemovePair", toStringLower(shouldShow) });
        }

        void sfxPlay(bool shouldShow = true)
        {
            execute(plugin, 11, { "sfxPlay", toStringLower(shouldShow) });
        }

        void sfxStop(bool shouldShow = true)
        {
            execute(plugin, 12, { "sfxStop", toStringLower(shouldShow) });
        }

        void sfxMute(bool shouldShow = true)
        {
            execute(plugin, 13, { "sfxMute", toStringLower(shouldShow) });
        }

        void sfxSetPosition(bool shouldShow = true)
        {
            execute(plugin, 14, { "sfxSetPosition", toStringLower(shouldShow) });
        }

        void sfxSetPan(bool shouldShow = true)
        {
            execute(plugin, 15, { "sfxSetPan", toStringLower(shouldShow) });
        }

        void sfxSetListenerPosition(bool shouldShow = true)
        {
            execute(plugin, 16, { "sfxSetListenerPosition", toStringLower(shouldShow) });
        }

        void sfxSetVolume(bool shouldShow = true)
        {
            execute(plugin, 17, { "sfxSetVolume", toStringLower(shouldShow) });
        }

        void sfxSetMaxBuffers(bool shouldShow = true)
        {
            execute(plugin, 18, { "sfxSetMaxBuffers", toStringLower(shouldShow) });
        }

        void sfxGetMaxBuffers(bool shouldShow = true)
        {
            execute(plugin, 19, { "sfxGetMaxBuffers", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_SOUNDFXPLUGIN_HPP
