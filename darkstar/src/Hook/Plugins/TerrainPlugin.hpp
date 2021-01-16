//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_TERRAINPLUGIN_HPP
#define DARKSTARHOOK_TERRAINPLUGIN_HPP

#include <sstream>
#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct TerrainPlugin : GamePluginWrapper
    {
        void saveTerrain(std::string objectName, std::string volumeName)
        {
            execute(plugin, 3, { "saveTerrain", objectName.c_str(), volumeName.c_str() });
        }

        void setTerrainDetail(std::string objectName, float perspectiveDist, float screenSquareSize)
        {
            std::string perspectiveDistStr = std::to_string(perspectiveDist);
            std::string screenSquareSizeStr = std::to_string(screenSquareSize);
            execute(plugin, 4, { "setTerrainDetail", objectName.c_str(), perspectiveDistStr.c_str(), screenSquareSizeStr.c_str() });
        }

        void setTerrainVisibility(std::string objectName, float visibleDist, float hazeDist)
        {
            std::string visibleDistStr = std::to_string(visibleDist);
            std::string hazeDistStr = std::to_string(hazeDist);

            execute(plugin, 5, { "setTerrainVisibility", objectName.c_str(), visibleDistStr.c_str(), hazeDistStr.c_str() });
        }

        void setTerrainContainer(std::string objectName, std::tuple<float, float, float> gravity, float drag, float height)
        {
            std::ostringstream gravityStr;
            gravityStr << std::get<0>(gravity) << " " << std::get<1>(gravity) << " " << std::get<2>(gravity);
            std::string dragStr = std::to_string(drag);
            std::string heightStr = std::to_string(height);

            execute(plugin, 6, { "setTerrainContainer", objectName.c_str(), gravityStr.str().c_str(), dragStr.c_str(), heightStr.c_str() });
        }

        void lightTerrain(std::string objectName, std::optional<float> minX, std::optional<float> minY, std::optional<float> maxX, std::optional<float> maxY)
        {
            if (minX.has_value() && minY.has_value() && maxX.has_value() && maxY.has_value())
            {
                std::string minXStr = std::to_string(*minX);
                std::string minYStr = std::to_string(*minY);
                std::string maxXStr = std::to_string(*maxX);
                std::string maxYStr = std::to_string(*maxY);

                execute(plugin, 7, { "lightTerrain", objectName.c_str(), minXStr.c_str(), minYStr.c_str(), maxXStr.c_str(), maxYStr.c_str() });
            }
            else
            {
                execute(plugin, 7, { "lightTerrain", objectName.c_str() });
            }
        }

        void showTerrain(bool shouldShow = true)
        {
            execute(plugin, 8, { "showTerrain", toStringLower(shouldShow) });
        }
    };
}

#endif //DARKSTARHOOK_TERRAINPLUGIN_HPP
