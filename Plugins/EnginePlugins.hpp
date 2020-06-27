#ifndef ENGINEPLUGINS_HPP
#define ENGINEPLUGINS_HPP

#include <memory>
#include <map>
#include <utility>
#include <optional>
#include <sstream>
#include <string>
#include "EngineFunctions.hpp"
#include "EngineExternalTypes.hpp"
#include "PythonTypes.hpp"

namespace Engine
{
	constexpr const char* toString(bool someValue)
	{
		return someValue == true ? "True" : "False";
	}

    constexpr const char* toIntString(bool someValue)
    {
        return someValue == true ? "1" : "0";
    }

    constexpr const char* toStringLower(bool someValue)
    {
        return someValue == true ? "true" : "false";
    }

	constexpr bool toBool(std::string_view someValue)
	{
		return someValue == "True" || someValue == "true" || someValue == "TRUE";
	}

    constexpr bool toForcedBool(const char* someRawValue)
    {
        if (someRawValue == nullptr)
        {
            return true;
        }

        return toBool(std::string_view{someRawValue});
    }

    constexpr std::optional<bool> toOptionalBool(const char* someRawValue)
    {
	    if (someRawValue == nullptr)
        {
	        return std::nullopt;
        }

        return toBool(std::string_view{someRawValue});
    }

	template<std::size_t Size>
	const char* execute(GamePlugin* plugin, std::int32_t callbackId, const char* (&&args)[Size])
	{
		return plugin->executeCallback(plugin->console, callbackId, Size, args);
    }

    const char* execute(GamePlugin* plugin, std::int32_t callbackId, std::vector<const char*>& args)
    {
        return plugin->executeCallback(plugin->console, callbackId, args.size(), args.data());
    }

    class GamePluginWrapper
    {
        protected:
            GamePlugin* plugin;

        public:
            GamePluginWrapper(GamePlugin* plugin) : plugin(plugin) {}
    };


	struct SimGuiConsolePlugin : GamePluginWrapper
	{
		bool consoleEnable(std::optional<bool> shouldEnable)
		{
			if (shouldEnable.has_value()) {
                return toBool(execute(plugin, 3, { "consoleEnable", toString(*shouldEnable) }));
			}
			
            return execute(plugin, 3, { "consoleEnable" });
		}
	};

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
            execute(plugin, 4, { "setTerrainDetail", simCanvasName.c_str(), perspectiveDistStr.c_str(), screenSquareSizeStr.c_str() });
        }

        void setTerrainVisibility(std::string objectName, float visibleDist, float hazeDist)
        {
            std::string visibleDistStr = std::to_string(visibleDist);
            std::string hazeDistStr = std::to_string(hazeDist);

            execute(plugin, 5, { "setTerrainVisibility", simCanvasName.c_str(), visibleDistStr.c_str(), hazeDistStr.c_str() });
        }

        void setTerrainContainer(std::string objectName, std::tuple<float, float, float> gravity, float drag, float height)
        {
            std::ostringstream gravityStr;
            gravityStr << std::get<0>(gravity) << " " std::get<1>(gravity) << " " << std::get<2>(gravity);
            std::string dragStr = std::to_string(drag);
            std::string heightStr = std::to_string(height);

            execute(plugin, 6, { "setTerrainContainer", objectName.c_str(), gravityStr.str().c_str(), dragStr.c_str(), heightStr().c_str() });
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

	struct InteriorPlugin : GamePluginWrapper
	{
        std::optional<bool> setInteriorShapeState(std::string objectName, int stateNumber)
        {
            std::string stateNumberStr = std::to_string(stateNumber);
            return toOptionalBool(execute(plugin, 0, { "setITRShapeState", objectName.c_str(), stateNumberStr.c_str() }));
        }

        std::optional<bool> setInteriorLightState((std::string objectName, int stateNumber)
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
            execute(plugin, 5, { "ITR::toggleBoundingBox" }));
        }
	};

	struct SkyPlugin : GamePluginWrapper
	{
        void globeLines()
        {
            execute(plugin, 0, { "globeLines" });
        }

        void showLowerStars(bool shouldShow = true)
        {
            execute(plugin, 1, { "showLowerStars", toStringLower(shouldShow) });
        }

        void setStarsVisibility(bool shouldShow = true)
        {
            execute(plugin, 2, { "setStarsVisibility", toStringLower(shouldShow) });
        }

        void setSkyMaterialListTag(int tag)
        {
            std::string tagStr = std::to_string(tag);
            execute(plugin, 3, { "setSkyMaterialListTag", tagStr.c_str() });
        }
	};

    struct NetPlugin : GamePluginWrapper
    {
        void netStats()
        {
            execute(plugin, 0, { "netStats" });
        }

        void playDemo(std::string demoFilename)
        {
            execute(plugin, 1, { "playDemo", demoFilename.c_str() });
        }

        void timeDemo(std::string demoFilename)
        {
            execute(plugin, 2, { "timeDemo", demoFilename.c_str() });
        }

        void connect(std::string netAddress)
        {
            execute(plugin, 3, { "connect", netAddress.c_str() });
        }

        void disconnect()
        {
            execute(plugin, 4, { "disconnect" });
        }

        void kick(int playerId, std::optional<std::string> reason = std::nullopt)
        {
            std::string playerIdStr = std::to_string(playerId);

            if (reason)
            {
                execute(plugin, 5, { "net::kick", playerIdStr.c_str(), reason->c_str() });
                return;
            }

            execute(plugin, 5, { "net::kick", playerIdStr.c_str() });
        }

        void rateChanged()
        {
            execute(plugin, 6, { "pref::PacketRate", toStringLower(shouldShow) });
        }

        void logPacketStats()
        {
            execute(plugin, 7, { "logPacketStats", toStringLower(shouldShow) });
        }

        std::string translateAddress(std::string address, std::optional<int> timeout = std::nullopt)
        {
            if (timeout)
            {
                std::string timeoutStr = std::to_string(*timeout);
                return execute(plugin, 8, { "DNet::TranslateAddress", address.c_str(), timeoutStr.c_str() });
            }

            return execute(plugin, 8, { "DNet::TranslateAddress", address.c_str(), "0" });
        }
    };

    struct SoundFXPlugin : GamePluginWrapper
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

            if (flags.empty())
            {
                execute(plugin, 6, { "sfxAdd2DProfile", toStringLower(shouldShow) });
                return;
            }

            std::vector<const char*> arguments;
            arguments.reserve(flags.size() + 3);
            arguments.push_back("sfxAdd2DProfile");
            arguments.push_back(profileIdStr.c_str());
            arguments.push_back(baseVolumeStr.c_str());
            for(auto& arg : flags)
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

            if (flags.empty())
            {
                execute(plugin, 7, { "sfxAdd3DProfile", profileIdStr.c_str(), baseVolumeStr.c_str() });
                return;
            }

            std::vector<const char*> arguments;
            arguments.reserve(flags.size() + 3);
            arguments.push_back("sfxAdd3DProfile");
            arguments.push_back(profileIdStr.c_str());
            arguments.push_back(baseVolumeStr.c_str());

            for(auto& arg : flags)
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

    struct RedbookPlugin : GamePluginWrapper
    {
        void newRedbook(bool shouldShow = true)
        {
            execute(plugin, 0, { "newRedbook", toStringLower(shouldShow) });
        }

        void rbOpen(bool shouldShow = true)
        {
            execute(plugin, 1, { "rbOpen", toStringLower(shouldShow) });
        }

        void rbClose(bool shouldShow = true)
        {
            execute(plugin, 2, { "rbClose", toStringLower(shouldShow) });
        }

        void rbEject(bool shouldShow = true)
        {
            execute(plugin, 3, { "rbEject", toStringLower(shouldShow) });
        }

        void rbRetract(bool shouldShow = true)
        {
            execute(plugin, 4, { "rbRetract", toStringLower(shouldShow) });
        }

        void rbGetStatus(bool shouldShow = true)
        {
            execute(plugin, 5, { "rbGetStatus", toStringLower(shouldShow) });
        }

        void rbGetTrackCount(bool shouldShow = true)
        {
            execute(plugin, 6, { "rbGetTrackCount", toStringLower(shouldShow) });
        }

        void rbGetTrackInfo(bool shouldShow = true)
        {
            execute(plugin, 7, { "rbGetTrackInfo", toStringLower(shouldShow) });
        }

        void rbGetTrackPosition(bool shouldShow = true)
        {
            execute(plugin, 8, { "rbGetTrackPosition", toStringLower(shouldShow) });
        }

        void rbPlay(bool shouldShow = true)
        {
            execute(plugin, 9, { "rbPlay", toStringLower(shouldShow) });
        }

        void rbStop(bool shouldShow = true)
        {
            execute(plugin, 10, { "rbStop", toStringLower(shouldShow) });
        }

        void rbPause(bool shouldShow = true)
        {
            execute(plugin, 11, { "rbPause", toStringLower(shouldShow) });
        }

        void rbResume(bool shouldShow = true)
        {
            execute(plugin, 12, { "rbResume", toStringLower(shouldShow) });
        }

        void rbSetVolume(bool shouldShow = true)
        {
            execute(plugin, 13, { "rbSetVolume", toStringLower(shouldShow) });
        }

        void rbGetVolume(bool shouldShow = true)
        {
            execute(plugin, 14, { "rbGetVolume", toStringLower(shouldShow) });
        }

        void rbSetPlayMode(bool shouldShow = true)
        {
            execute(plugin, 15, { "rbSetPlayMode", toStringLower(shouldShow) });
        }
    };

    struct MovPlayPlugin : GamePluginWrapper
    {
        void newMovPlay(bool shouldShow = true)
        {
            execute(plugin, 0, { "newMovPlay", toStringLower(shouldShow) });
        }

        void openMovie(bool shouldShow = true)
        {
            execute(plugin, 1, { "openMovie", toStringLower(shouldShow) });
        }

        void closeMovie(bool shouldShow = true)
        {
            execute(plugin, 2, { "closeMovie", toStringLower(shouldShow) });
        }

        void playMovie(bool shouldShow = true)
        {
            execute(plugin, 3, { "playMovie", toStringLower(shouldShow) });
        }

        void playMovieToComp(bool shouldShow = true)
        {
            execute(plugin, 4, { "playMovieToComp", toStringLower(shouldShow) });
        }

        void stopMovie(bool shouldShow = true)
        {
            execute(plugin, 5, { "stopMovie", toStringLower(shouldShow) });
        }

        void pauseMovie(bool shouldShow = true)
        {
            execute(plugin, 6, { "pauseMovie", toStringLower(shouldShow) });
        }
    };

    struct SimInputPlugin : GamePluginWrapper
    {
        void newInputManager(bool shouldShow = true)
        {
            execute(plugin, 0, { "newInputManager", toStringLower(shouldShow) });
        }

        void listInputDevices(bool shouldShow = true)
        {
            execute(plugin, 1, { "listInputDevices", toStringLower(shouldShow) });
        }

        void getInputDeviceInfo(bool shouldShow = true)
        {
            execute(plugin, 2, { "getInputDeviceInfo", toStringLower(shouldShow) });
        }

        void saveInputDeviceInfo(bool shouldShow = true)
        {
            execute(plugin, 3, { "saveInputDeviceInfo", toStringLower(shouldShow) });
        }

        void inputOpen(bool shouldShow = true)
        {
            execute(plugin, 4, { "inputOpen", toStringLower(shouldShow) });
        }

        void inputClose(bool shouldShow = true)
        {
            execute(plugin, 5, { "inputClose", toStringLower(shouldShow) });
        }

        void inputCapture(bool shouldShow = true)
        {
            execute(plugin, 6, { "inputCapture", toStringLower(shouldShow) });
        }

        void inputRelease(bool shouldShow = true)
        {
            execute(plugin, 7, { "inputRelease", toStringLower(shouldShow) });
        }

        void inputActivate(bool shouldShow = true)
        {
            execute(plugin, 8, { "inputActivate", toStringLower(shouldShow) });
        }

        void inputDeactivate(bool shouldShow = true)
        {
            execute(plugin, 9, { "inputDeactivate", toStringLower(shouldShow) });
        }

        void editActionMap(bool shouldShow = true)
        {
            execute(plugin, 10, { "editActionMap", toStringLower(shouldShow) });
        }

        void newActionMap(bool shouldShow = true)
        {
            execute(plugin, 11, { "newActionMap", toStringLower(shouldShow) });
        }

        void bindAction(bool shouldShow = true)
        {
            execute(plugin, 12, { "bindAction", toStringLower(shouldShow) });
        }

        void bindCommand(bool shouldShow = true)
        {
            execute(plugin, 13, { "bindCommand", toStringLower(shouldShow) });
        }

        void bind(bool shouldShow = true)
        {
            execute(plugin, 14, { "bind", toStringLower(shouldShow) });
        }

        void saveActionMap(bool shouldShow = true)
        {
            execute(plugin, 15, { "saveActionMap", toStringLower(shouldShow) });
        }
    };

    struct SimGuiPlugin : GamePluginWrapper
    {
        void GuiInspect(bool shouldShow = true)
        {
            execute(plugin, 0, { "GuiInspect", toStringLower(shouldShow) });
        }

        void EditMode(bool shouldShow = true)
        {
            execute(plugin, 1, { "GuiEditMode", toStringLower(shouldShow) });
        }

        void EditNewControl(bool shouldShow = true)
        {
            execute(plugin, 2, { "GuiEditNewControl", toStringLower(shouldShow) });
        }

        void SetCurrentAddSet(bool shouldShow = true)
        {
            execute(plugin, 3, { "GuiSetAddSet", toStringLower(shouldShow) });
        }

        void SetSelection(bool shouldShow = true)
        {
            execute(plugin, 4, { "GuiSetSelection", toStringLower(shouldShow) });
        }

        void NewContent(bool shouldShow = true)
        {
            execute(plugin, 5, { "GuiNewContentCtrl", toStringLower(shouldShow) });
        }

        void LoadContent(bool shouldShow = true)
        {
            execute(plugin, 6, { "GuiLoadContentCtrl", toStringLower(shouldShow) });
        }

        void SaveContent(bool shouldShow = true)
        {
            execute(plugin, 7, { "GuiSaveContentCtrl", toStringLower(shouldShow) });
        }

        void SaveSelection(bool shouldShow = true)
        {
            execute(plugin, 8, { "GuiSaveSelection", toStringLower(shouldShow) });
        }

        void LoadSelection(bool shouldShow = true)
        {
            execute(plugin, 9, { "GuiLoadSelection", toStringLower(shouldShow) });
        }

        void GuiJustify(bool shouldShow = true)
        {
            execute(plugin, 10, { "GuiJustify", toStringLower(shouldShow) });
        }

        void GuiToolbar(bool shouldShow = true)
        {
            execute(plugin, 11, { "GuiToolbar", toStringLower(shouldShow) });
        }

        void GuiSendToBack(bool shouldShow = true)
        {
            execute(plugin, 12, { "GuiSendToBack", toStringLower(shouldShow) });
        }

        void GuiBringToFront(bool shouldShow = true)
        {
            execute(plugin, 13, { "GuiBringToFront", toStringLower(shouldShow) });
        }

        void SendRootMessage(bool shouldShow = true)
        {
            execute(plugin, 14, { "GuiSendRootMessage", toStringLower(shouldShow) });
        }

        void SetCCursor(bool shouldShow = true)
        {
            execute(plugin, 15, { "setCursor", toStringLower(shouldShow) });
        }

        void IsCursorOn(bool shouldShow = true)
        {
            execute(plugin, 16, { "isCursorOn", toStringLower(shouldShow) });
        }

        void CursorOn(bool shouldShow = true)
        {
            execute(plugin, 17, { "cursorOn", toStringLower(shouldShow) });
        }

        void CursorOff(bool shouldShow = true)
        {
            execute(plugin, 18, { "cursorOff", toStringLower(shouldShow) });
        }

        void WindowsMouseEnable(bool shouldShow = true)
        {
            execute(plugin, 19, { "windowsMouseEnable", toStringLower(shouldShow) });
        }

        void WindowsMouseDisable(bool shouldShow = true)
        {
            execute(plugin, 20, { "windowsMouseDisable", toStringLower(shouldShow) });
        }

        void GuiPushDialog(bool shouldShow = true)
        {
            execute(plugin, 21, { "GuiPushDialog", toStringLower(shouldShow) });
        }

        void GuiPopDialog(bool shouldShow = true)
        {
            execute(plugin, 22, { "GuiPopDialog", toStringLower(shouldShow) });
        }

        void GuiSetValue(bool shouldShow = true)
        {
            execute(plugin, 23, { "Control::setValue", toStringLower(shouldShow) });
        }

        void GuiGetValue(bool shouldShow = true)
        {
            execute(plugin, 24, { "Control::getValue", toStringLower(shouldShow) });
        }

        void GuiSetActive(bool shouldShow = true)
        {
            execute(plugin, 25, { "Control::setActive", toStringLower(shouldShow) });
        }

        void GuiGetActive(bool shouldShow = true)
        {
            execute(plugin, 26, { "Control::getActive", toStringLower(shouldShow) });
        }

        void GuiSetVisible(bool shouldShow = true)
        {
            execute(plugin, 27, { "Control::setVisible", toStringLower(shouldShow) });
        }

        void GuiGetVisible(bool shouldShow = true)
        {
            execute(plugin, 28, { "Control::getVisible", toStringLower(shouldShow) });
        }

        void GuiPerformClick(bool shouldShow = true)
        {
            execute(plugin, 29, { "Control::performClick", toStringLower(shouldShow) });
        }

        void GuiSetText(bool shouldShow = true)
        {
            execute(plugin, 30, { "Control::setText", toStringLower(shouldShow) });
        }

        void GuiGetText(bool shouldShow = true)
        {
            execute(plugin, 31, { "Control::getText", toStringLower(shouldShow) });
        }

        void TextListClear(bool shouldShow = true)
        {
            execute(plugin, 32, { "TextList::Clear", toStringLower(shouldShow) });
        }

        void TextListAdd(bool shouldShow = true)
        {
            execute(plugin, 33, { "TextList::AddLine", toStringLower(shouldShow) });
        }

        void WindowsKeyboardEnable(bool shouldShow = true)
        {
            execute(plugin, 34, { "windowsKeyboardEnable", toStringLower(shouldShow) });
        }

        void WindowsKeyboardDisable(bool shouldShow = true)
        {
            execute(plugin, 35, { "windowsKeyboardDisable", toStringLower(shouldShow) });
        }
    };

    struct ToolPlugin : GamePluginWrapper
    {
        void newToolWindow(bool shouldShow = true)
        {
            execute(plugin, 0, { "newToolWindow", toStringLower(shouldShow) });
        }

        void newToolStrip(bool shouldShow = true)
        {
            execute(plugin, 1, { "newToolStrip", toStringLower(shouldShow) });
        }

        void listToolButtons(bool shouldShow = true)
        {
            execute(plugin, 2, { "listToolButtons", toStringLower(shouldShow) });
        }

        void listToolWindows(bool shouldShow = true)
        {
            execute(plugin, 3, { "listToolWindows", toStringLower(shouldShow) });
        }

        void hideToolWin(bool shouldShow = true)
        {
            execute(plugin, 4, { "hideToolWin", toStringLower(shouldShow) });
        }

        void showToolWin(bool shouldShow = true)
        {
            execute(plugin, 5, { "showToolWin", toStringLower(shouldShow) });
        }

        void showToolWinAll(bool shouldShow = true)
        {
            execute(plugin, 6, { "showToolWinAll", toStringLower(shouldShow) });
        }

        void setToolWinPos(bool shouldShow = true)
        {
            execute(plugin, 7, { "setToolWinPos", toStringLower(shouldShow) });
        }

        void addToolButton(bool shouldShow = true)
        {
            execute(plugin, 8, { "addToolButton", toStringLower(shouldShow) });
        }

        void delToolButton(bool shouldShow = true)
        {
            execute(plugin, 9, { "delToolButton", toStringLower(shouldShow) });
        }

        void addToolGap(bool shouldShow = true)
        {
            execute(plugin, 10, { "addToolGap", toStringLower(shouldShow) });
        }

        void setToolCommand(bool shouldShow = true)
        {
            execute(plugin, 11, { "setToolCommand", toStringLower(shouldShow) });
        }

        void setButtonHelp(bool shouldShow = true)
        {
            execute(plugin, 12, { "setButtonHelp", toStringLower(shouldShow) });
        }

        void isButtonDown(bool shouldShow = true)
        {
            execute(plugin, 13, { "isButtonDown", toStringLower(shouldShow) });
        }

        void addStatusBar(bool shouldShow = true)
        {
            execute(plugin, 14, { "addStatusBar", toStringLower(shouldShow) });
        }

        void delStatusBar(bool shouldShow = true)
        {
            execute(plugin, 15, { "delStatusBar", toStringLower(shouldShow) });
        }

        void setStatusField(bool shouldShow = true)
        {
            execute(plugin, 16, { "setStatusField", toStringLower(shouldShow) });
        }

        void getStatusField(bool shouldShow = true)
        {
            execute(plugin, 17, { "getStatusField", toStringLower(shouldShow) });
        }

        void clearStatusField(bool shouldShow = true)
        {
            execute(plugin, 18, { "clearStatusField", toStringLower(shouldShow) });
        }

        void setMainWindow(bool shouldShow = true)
        {
            execute(plugin, 19, { "setMainWindow", toStringLower(shouldShow) });
        }

        void editVar(bool shouldShow = true)
        {
            execute(plugin, 20, { "editVar", toStringLower(shouldShow) });
        }

        void edit2Vars(bool shouldShow = true)
        {
            execute(plugin, 21, { "edit2Vars", toStringLower(shouldShow) });
        }

        void confirmBox(bool shouldShow = true)
        {
            execute(plugin, 22, { "confirmBox", toStringLower(shouldShow) });
        }

        void openFile(bool shouldShow = true)
        {
            execute(plugin, 23, { "openFile", toStringLower(shouldShow) });
        }

        void saveFileAs(bool shouldShow = true)
        {
            execute(plugin, 24, { "saveFileAs", toStringLower(shouldShow) });
        }

        void browseBox(bool shouldShow = true)
        {
            execute(plugin, 25, { "browseBox", toStringLower(shouldShow) });
        }
    };

    struct SimTreePlugin : GamePluginWrapper
    {
        void simTreeCreate(bool shouldShow = true)
        {
            execute(plugin, 0, { "simTreeCreate", toStringLower(shouldShow) });
        }

        void simTreeAddSet(bool shouldShow = true)
        {
            execute(plugin, 1, { "simTreeAddSet", toStringLower(shouldShow) });
        }

        void simTreeRegBitmaps(bool shouldShow = true)
        {
            execute(plugin, 2, { "simTreeRegBitmaps", toStringLower(shouldShow) });
        }

        void simTreeRegClass(bool shouldShow = true)
        {
            execute(plugin, 3, { "simTreeRegClass", toStringLower(shouldShow) });
        }

        void simTreeRegScript(bool shouldShow = true)
        {
            execute(plugin, 4, { "simTreeRegScript", toStringLower(shouldShow) });
        }
    };

    struct MissionPlugin : GamePluginWrapper
    {
        void newMissionEditor(bool shouldShow = true)
        {
            execute(plugin, 0, { "MissionEditor", toStringLower(shouldShow) });
        }

        void MissionRegType(bool shouldShow = true)
        {
            execute(plugin, 1, { "MissionRegType", toStringLower(shouldShow) });
        }

        void MissionRegObject(bool shouldShow = true)
        {
            execute(plugin, 2, { "MissionRegObject", toStringLower(shouldShow) });
        }

        void MissionRegTerrain(bool shouldShow = true)
        {
            execute(plugin, 3, { "MissionRegTerrain", toStringLower(shouldShow) });
        }

        void MissionAddObject(bool shouldShow = true)
        {
            execute(plugin, 4, { "MissionAddObject", toStringLower(shouldShow) });
        }

        void MissionCreateObject(bool shouldShow = true)
        {
            execute(plugin, 5, { "MissionCreateObject", toStringLower(shouldShow) });
        }

        void MissionLoadObjects(bool shouldShow = true)
        {
            execute(plugin, 6, { "MissionLoadObjects", toStringLower(shouldShow) });
        }

        void newMissionGroup(bool shouldShow = true)
        {
            execute(plugin, 7, { "newMissionGroup", toStringLower(shouldShow) });
        }

        void newMission(bool shouldShow = true)
        {
            execute(plugin, 8, { "newMission", toStringLower(shouldShow) });
        }

        void addMissionButton(bool shouldShow = true)
        {
            execute(plugin, 9, { "addMissionButton", toStringLower(shouldShow) });
        }

        void removeMissionButton(bool shouldShow = true)
        {
            execute(plugin, 10, { "removeMissionButton", toStringLower(shouldShow) });
        }

        void removeMissionButtons(bool shouldShow = true)
        {
            execute(plugin, 11, { "removeMissionButtons", toStringLower(shouldShow) });
        }

        void setMissionButtonChecked(bool shouldShow = true)
        {
            execute(plugin, 12, { "setMissionButtonChecked", toStringLower(shouldShow) });
        }

        void setMissionButtonEnabled(bool shouldShow = true)
        {
            execute(plugin, 13, { "setMissionButtonEnabled", toStringLower(shouldShow) });
        }

        void isMissionButtonChecked(bool shouldShow = true)
        {
            execute(plugin, 14, { "isMissionButtonChecked", toStringLower(shouldShow) });
        }

        void isMissionButtonEnabled(bool shouldShow = true)
        {
            execute(plugin, 15, { "isMissionButtonEnabled", toStringLower(shouldShow) });
        }

        void setAutoSaveInterval(bool shouldShow = true)
        {
            execute(plugin, 16, { "missionSetAutoSaveInterval", toStringLower(shouldShow) });
        }

        void setAutoSaveName(bool shouldShow = true)
        {
            execute(plugin, 17, { "missionSetAutoSaveName", toStringLower(shouldShow) });
        }

        void saveObjectPersist(bool shouldShow = true)
        {
            execute(plugin, 18, { "missionSaveObjectPersist", toStringLower(shouldShow) });
        }

        void loadObjectPersist(bool shouldShow = true)
        {
            execute(plugin, 19, { "missionLoadObjectPersist", toStringLower(shouldShow) });
        }

        void undoMoveRotate(bool shouldShow = true)
        {
            execute(plugin, 20, { "missionUndoMoveRotate", toStringLower(shouldShow) });
        }
    };

    struct FearMissionPlugin : GamePluginWrapper
    {
        void Create(bool shouldShow = true)
        {
            execute(plugin, 0, { "ME::Create", toStringLower(shouldShow) });
        }

        void RegisterType(bool shouldShow = true)
        {
            execute(plugin, 1, { "ME::RegisterType", toStringLower(shouldShow) });
        }

        void SetGrabMask(bool shouldShow = true)
        {
            execute(plugin, 2, { "ME::SetGrabMask", toStringLower(shouldShow) });
        }

        void SetPlaceMask(bool shouldShow = true)
        {
            execute(plugin, 3, { "ME::SetPlaceMask", toStringLower(shouldShow) });
        }

        void SetDefaultPlaceMask(bool shouldShow = true)
        {
            execute(plugin, 4, { "ME::SetDefaultPlaceMask", toStringLower(shouldShow) });
        }

        void GetConsoleOptions(bool shouldShow = true)
        {
            execute(plugin, 5, { "ME::GetConsoleOptions", toStringLower(shouldShow) });
        }

        void ObjectToCamera(bool shouldShow = true)
        {
            execute(plugin, 6, { "ME::ObjectToCamera", toStringLower(shouldShow) });
        }

        void CameraToObject(bool shouldShow = true)
        {
            execute(plugin, 7, { "ME::CameraToObject", toStringLower(shouldShow) });
        }

        void ObjectToSC(bool shouldShow = true)
        {
            execute(plugin, 8, { "ME::ObjectToSC", toStringLower(shouldShow) });
        }

        void CreateObject(bool shouldShow = true)
        {
            execute(plugin, 9, { "MissionCreateObject", toStringLower(shouldShow) });
        }

        void onSelected(bool shouldShow = true)
        {
            execute(plugin, 10, { "ME::onSelected", toStringLower(shouldShow) });
        }

        void onUnselected(bool shouldShow = true)
        {
            execute(plugin, 11, { "ME::onUnselected", toStringLower(shouldShow) });
        }

        void onSelectionCleared(bool shouldShow = true)
        {
            execute(plugin, 12, { "ME::onSelectionCleared", toStringLower(shouldShow) });
        }

        void onUseTerrainGrid(bool shouldShow = true)
        {
            execute(plugin, 13, { "ME::onUseTerrainGrid", toStringLower(shouldShow) });
        }

        void PlaceBookmark(bool shouldShow = true)
        {
            execute(plugin, 14, { "ME::PlaceBookmark", toStringLower(shouldShow) });
        }

        void GotoBookmark(bool shouldShow = true)
        {
            execute(plugin, 15, { "ME::GotoBookmark", toStringLower(shouldShow) });
        }

        void DeleteSelection(bool shouldShow = true)
        {
            execute(plugin, 16, { "ME::DeleteSelection", toStringLower(shouldShow) });
        }

        void CopySelection(bool shouldShow = true)
        {
            execute(plugin, 17, { "ME::CopySelection", toStringLower(shouldShow) });
        }

        void CutSelection(bool shouldShow = true)
        {
            execute(plugin, 18, { "ME::CutSelection", toStringLower(shouldShow) });
        }

        void PasteSelection(bool shouldShow = true)
        {
            execute(plugin, 19, { "ME::PasteSelection", toStringLower(shouldShow) });
        }

        void PasteFile(bool shouldShow = true)
        {
            execute(plugin, 20, { "ME::PasteFile", toStringLower(shouldShow) });
        }

        void DuplicateSelection(bool shouldShow = true)
        {
            execute(plugin, 21, { "ME::DuplicateSelection", toStringLower(shouldShow) });
        }

        void CreateGroup(bool shouldShow = true)
        {
            execute(plugin, 22, { "ME::CreateGroup", toStringLower(shouldShow) });
        }

        void DropSelection(bool shouldShow = true)
        {
            execute(plugin, 23, { "ME::DropSelection", toStringLower(shouldShow) });
        }

        void Undo(bool shouldShow = true)
        {
            execute(plugin, 24, { "ME::Undo", toStringLower(shouldShow) });
        }

        void Redo(bool shouldShow = true)
        {
            execute(plugin, 25, { "ME::Redo", toStringLower(shouldShow) });
        }

        void Save(bool shouldShow = true)
        {
            execute(plugin, 26, { "ME::Save", toStringLower(shouldShow) });
        }

        void MissionLight(bool shouldShow = true)
        {
            execute(plugin, 27, { "ME::MissionLight", toStringLower(shouldShow) });
        }

        void RebuildCommandMap(bool shouldShow = true)
        {
            execute(plugin, 28, { "ME::RebuildCommandMap", toStringLower(shouldShow) });
        }
    };

    struct ESGameplayPlugin : GamePluginWrapper
    {
    };

    struct ShellOpenPlugin : GamePluginWrapper
    {
    };

    struct LandscapePlugin : GamePluginWrapper
    {
    };

    struct TedPlugin : GamePluginWrapper
    {
    };

    struct ShellOpenPlugin : GamePluginWrapper
    {
    };

    struct SimTelnetPlugin : GamePluginWrapper
    {
    };

    struct SimShapePlugin : GamePluginWrapper
    {
    };

    struct HercInfoDataPlugin : GamePluginWrapper
    {
    };

    struct DatabasePlugin : GamePluginWrapper
    {
    };

    struct ICPlugin : GamePluginWrapper
    {
    };

    struct IRCPlugin : GamePluginWrapper
    {
    };

    struct ESFPlugin : GamePluginWrapper
    {
    };

    struct ESBasePlugin : GamePluginWrapper
    {
    };

    struct ESChatPlugin : GamePluginWrapper
    {
    };

    struct DynamicDataPlugin : GamePluginWrapper
    {
    };

	class StarsiegePlugins
	{
	    std::vector<GamePlugin*> _plugins;

        public:
	        StarsiegePlugins(std::vector<GamePlugin*> plugins) : _plugins(std::move(plugins)) {}

            SimGuiConsolePlugin guiConsole()
            {
                return SimGuiConsolePlugin{_plugins[0]};
            }

            GfxPlugin gfx()
            {
				return GfxPlugin{_plugins[1]};
            }

            TerrainPlugin terrain()
            {
				return TerrainPlugin{_plugins[2]};
            }

            InteriorPlugin interior()
            {
				return InteriorPlugin{_plugins[3]};
            }

            SkyPlugin sky()
            {
                return SkyPlugin{_plugins[4]};
            }

            NetPlugin net()
            {
                return SkyPlugin{_plugins[5]};
            }

            SoundFxPlugin soundFx()
            {
                return SkyPlugin{_plugins[6]};
            }

            RedbookPlugin redbook()
            {
                return SkyPlugin{_plugins[7]};
            }

            MovPlayPlugin movPlay()
            {
                return SkyPlugin{_plugins[8]};
            }

            SimInputPlugin input()
            {
                return SkyPlugin{_plugins[9]};
            }

            SimGuiPlugin gui()
            {
                return SkyPlugin{_plugins[10]};
            }

            ToolPlugin tool()
            {
                return SkyPlugin{_plugins[11]};
            }

            SimTreePlugin tree()
            {
                return SkyPlugin{_plugins[12]};
            }

            MisionPlugin mission()
            {
                return SkyPlugin{_plugins[13]};
            }

            FearMissionPlugin fearMission()
            {
                return SkyPlugin{_plugins[14]};
            }
	};


}

#endif
