//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_GFXPLUGIN_HPP
#define DARKSTARHOOK_GFXPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct GfxPlugin : GamePluginWrapper
    {
        void flushTextureCache(std::string simCanvasName)
        {
            execute(plugin, 0, {"flushTextureCache", simCanvasName.c_str()});
        }

        void setGamma(std::string simCanvasName, float gammaValue)
        {
            auto gammaValueStr = std::to_string(gammaValue);
            execute(plugin, 1, {"setGamma", simCanvasName.c_str(), gammaValueStr.c_str()});
        }

        bool setFullscreenDevice(std::string simCanvasName, std::string deviceName)
        {
            return toBool(execute(plugin, 2, { "setFullscreenDevice", simCanvasName.c_str(), deviceName.c_str() }));
        }

        bool setWindowedDevice(std::string simCanvasName, std::string deviceName)
        {
            auto result = execute(plugin, 3, { "setWindowedDevice", simCanvasName.c_str(), deviceName.c_str() });

            if (result == nullptr)
            {
                return true;
            }

            return toBool(result);
        }

        void listDevices()
        {
            execute(plugin, 4, { "listDevices" });
        }

        bool testDevice(std::string deviceName)
        {
            auto result = execute(plugin, 5, { "testDevice", deviceName.c_str() });

            if (result == nullptr)
            {
                return true;
            }

            return toBool(result);
        }

        void messageCanvasDevice(std::string simCanvasName, std::string message)
        {
            execute(plugin, 6, { "messageCanvasDevice", simCanvasName.c_str(), message.c_str() });
        }

        void setScreenshotSeq(int screenShotSeq)
        {
            auto screenshotSeqStr = std::to_string(screenShotSeq);
            execute(plugin, 7, { "setScreenShotSeq", screenshotSeqStr.c_str() });
        }

        void screenshot(std::optional<std::string> filename, std::optional<int> screenShotSeq = std::nullopt)
        {
            if (screenShotSeq.has_value())
            {
                setScreenshotSeq(*screenShotSeq);
            }

            if (filename.has_value())
            {
                execute(plugin, 8, { "screenShot", filename->c_str() });
            }
            else
            {
                execute(plugin, 8, { "screenShot" });
            }
        }

        void swapSurfaces(std::string simCanvasName)
        {
            execute(plugin, 9, { "swapSurfaces", simCanvasName.c_str() });
        }


        bool isFullscreenMode(std::string simCanvasName)
        {
            return toBool(execute(plugin, 10, { "isFullscreenMode", simCanvasName.c_str() }));
        }

        void setFullscreenMode(std::string simCanvasName, bool isFullscreen = false)
        {
            execute(plugin, 11, { "setFullscreenMode", simCanvasName.c_str(), toString(isFullscreen) });
        }

        bool setFsResolution(std::string simCanvasName, std::string resolution)
        {
            return toBool(execute(plugin, 12, { "setFSResolution", simCanvasName.c_str(), resolution.c_str() }));
        }

        bool isVirtualFs(std::string simCanvasName)
        {
            return toBool(execute(plugin, 13, { "isVirtualFS", simCanvasName.c_str() }));
        }

        void nextRes(std::string simCanvasName)
        {
            execute(plugin, 14, { "nextRes", simCanvasName.c_str() });
        }

        void prevRes(std::string simCanvasName)
        {
            execute(plugin, 15, { "prevRes", simCanvasName.c_str() });
        }

        void lockWindowSize(std::string simCanvasName)
        {
            execute(plugin, 16, { "lockWindowSize", simCanvasName.c_str() });
        }

        void unlockWindowSize(std::string simCanvasName)
        {
            execute(plugin, 17, { "unlockWindowSize", simCanvasName.c_str() });
        }

        void setWindowSize(std::string simCanvasName, std::uint32_t width, std::uint32_t height)
        {
            auto widthStr = std::to_string(width);
            auto heightStr = std::to_string(height);
            execute(plugin, 18, { "setWindowSize", simCanvasName.c_str(), widthStr.c_str(), heightStr.c_str() });
        }

        bool isGfxDriver(std::string simCanvasName, std::string driverName)
        {
            return toBool(execute(plugin, 19, { "isGfxDriver", simCanvasName.c_str(), driverName.c_str() }));
        }

        void resetUpdateRegion(std::string simCanvasName)
        {
            execute(plugin, 20, { "resetUpdateRegion", simCanvasName.c_str() });
        }
    };
}

#endif //DARKSTARHOOK_GFXPLUGIN_HPP
