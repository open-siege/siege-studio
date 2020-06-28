//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_INTERIORPLUGIN_HPP
#define DARKSTARHOOK_INTERIORPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct InteriorPlugin : GamePluginWrapper
    {
        std::optional<bool> setInteriorShapeState(std::string objectName, int stateNumber)
        {
            std::string stateNumberStr = std::to_string(stateNumber);
            return toOptionalBool(execute(plugin, 0, { "setITRShapeState", objectName.c_str(), stateNumberStr.c_str() }));
        }

        std::optional<bool> setInteriorLightState(std::string objectName, int stateNumber)
        {
            std::string stateNumberStr = std::to_string(stateNumber);
            return toOptionalBool(execute(plugin, 1, { "setITRLightState", objectName.c_str(), stateNumberStr.c_str() }));
        }

        std::optional<bool> animateInteriorLight(std::string objectName, std::string lightName, int loopCount)
        {
            std::string loopCountStr = std::to_string(loopCount);
            return toOptionalBool(execute(plugin, 2, { "animateInteriorLight", objectName.c_str(), lightName.c_str(), loopCountStr.c_str() }));
        }

        std::optional<bool> stopInteriorLightAnim(std::string objectName, std::string lightName)
        {
            return toOptionalBool(execute(plugin, 3, { "stopInteriorLightAnim", objectName.c_str(), lightName.c_str() }));
        }

        std::optional<bool> resetInteriorLight(std::string objectName, std::string lightName)
        {
            return toOptionalBool(execute(plugin, 4, { "setITRShapeState", objectName.c_str(), lightName.c_str() }));
        }

        void toggleBoundingBox()
        {
            execute(plugin, 5, { "ITR::toggleBoundingBox" });
        }
    };
}


#endif //DARKSTARHOOK_INTERIORPLUGIN_HPP
