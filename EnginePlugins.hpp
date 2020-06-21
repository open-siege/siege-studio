#ifndef ENGINEPLUGINS_HPP
#define ENGINEPLUGINS_HPP

#include <memory>
#include <map>
#include <utility>
#include <optional>
#include "EngineFunctions.hpp"
#include "EngineExternalTypes.hpp"
#include "PythonTypes.hpp"

namespace Engine
{
	constexpr const char* boolToString(bool someValue)
	{
		return someValue == true ? "True" : "False";
	}

	constexpr bool stringToBool(std::string_view someValue)
	{
		return someValue == "True" || someValue == "true" || someValue == "TRUE";
	}

	struct SimGuiConsolePlugin : GamePlugin
	{
		bool consoleEnable(bool shouldEnable = true)
		{

			std::array<const char*, 2> arguments{ "consoleEnable", boolToString(shouldEnable) };
			return stringToBool(executeCallback(console, 3, arguments.size(), arguments.data()));
		}

		virtual DARKCALL ~SimGuiConsolePlugin() override = default;
	};

	struct GfxPlugin : GamePlugin
	{
		void flushTextureCache(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "flushTextureCache", simCanvasName.c_str() };
			executeCallback(console, 0, arguments.size(), arguments.data());
		}

		void setGamma(const std::string& simCanvasName, float gammaValue)
		{
			auto gammaValueStr = std::to_string(gammaValue);
			std::array<const char*, 3> arguments{ "setGamma", simCanvasName.c_str(), gammaValueStr.c_str() };
			executeCallback(console, 1, arguments.size(), arguments.data());
		}

		bool setFullscreenDevice(const std::string& simCanvasName, const std::string& deviceName)
		{
			std::array<const char*, 3> arguments{ "setFullscreenDevice", simCanvasName.c_str(), deviceName.c_str() };
			return stringToBool(executeCallback(console, 2, arguments.size(), arguments.data()));
		}

		bool setWindowedDevice(const std::string& simCanvasName, const std::string& deviceName)
		{
			std::array<const char*, 3> arguments{ "setWindowedDevice", simCanvasName.c_str(), deviceName.c_str() };

			auto result = executeCallback(console, 3, arguments.size(), arguments.data());

			if (result == nullptr)
			{
				return true;
			}

			return stringToBool(result);
		}

		void listDevices()
		{
			std::array<const char*, 1> arguments{ "listDevices" };
			executeCallback(console, 4, arguments.size(), arguments.data());
		}

		bool testDevice(const std::string& deviceName)
		{
			std::array<const char*, 2> arguments{ "testDevice", deviceName.c_str() };
			auto result = executeCallback(console, 5, arguments.size(), arguments.data());

			if (result == nullptr)
			{
				return true;
			}

			return stringToBool(result);
		}

		void messageCanvasDevice(const std::string& simCanvasName, const std::string& message)
		{
			std::array<const char*, 3> arguments{ "messageCanvasDevice", simCanvasName.c_str(), message.c_str() };
			executeCallback(console, 6, arguments.size(), arguments.data());
		}

		void setScreenshotSeq(int screenShotSeq)
		{
			auto screenshotSeqStr = std::to_string(screenShotSeq);
			std::array<const char*, 2> arguments{ "setScreenShotSeq", screenshotSeqStr.c_str() };
			executeCallback(console, 7, arguments.size(), arguments.data());
		}

		void screenshot(std::optional<std::string> filename, std::optional<int> screenShotSeq = std::nullopt)
		{
			if (screenShotSeq.has_value())
			{
				setScreenshotSeq(screenShotSeq.value());
			}

			std::vector<const char*> arguments;
			arguments.reserve(2);
			arguments.push_back("screenShot");

			if (filename.has_value())
			{
				arguments.push_back(filename.value().c_str());
			}

			executeCallback(console, 8, arguments.size(), arguments.data());
		}

		void swapSurfaces(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "swapSurfaces", simCanvasName.c_str() };
			executeCallback(console, 9, arguments.size(), arguments.data());
		}


		bool isFullscreenMode(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "isFullscreenMode", simCanvasName.c_str() };
			return stringToBool(executeCallback(console, 10, arguments.size(), arguments.data()));
		}

		void setFullscreenMode(const std::string& simCanvasName, bool isFullscreen = false)
		{
			std::array<const char*, 3> arguments{ "setFullscreenMode", simCanvasName.c_str(), boolToString(isFullscreen) };
			executeCallback(console, 11, arguments.size(), arguments.data());
		}

		bool setFsResolution(const std::string& simCanvasName, const std::string& resolution)
		{
			std::array<const char*, 3> arguments{ "setFSResolution", simCanvasName.c_str(), resolution.c_str() };
			return stringToBool(executeCallback(console, 12, arguments.size(), arguments.data()));
		}

		bool isVirtualFs(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "isVirtualFS", simCanvasName.c_str() };
			return stringToBool(executeCallback(console, 13, arguments.size(), arguments.data()));
		}

		void nextRes(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "nextRes", simCanvasName.c_str() };
			executeCallback(console, 14, arguments.size(), arguments.data());
		}

		void prevRes(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "prevRes", simCanvasName.c_str() };
			executeCallback(console, 15, arguments.size(), arguments.data());
		}

		void lockWindowSize(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "lockWindowSize", simCanvasName.c_str() };
			executeCallback(console, 16, arguments.size(), arguments.data());
		}

		void unlockWindowSize(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "unlockWindowSize", simCanvasName.c_str() };
			executeCallback(console, 17, arguments.size(), arguments.data());
		}

		void setWindowSize(const std::string& simCanvasName, std::uint32_t width, std::uint32_t height)
		{
			auto widthStr = std::to_string(width);
			auto heightStr = std::to_string(height);

			std::array<const char*, 4> arguments{ "setWindowSize", simCanvasName.c_str(), widthStr.c_str(), heightStr.c_str() };
			executeCallback(console, 18, arguments.size(), arguments.data());
		}

		bool isGfxDriver(const std::string& simCanvasName, const std::string& driverName)
		{
			std::array<const char*, 3> arguments{ "isGfxDriver", simCanvasName.c_str(), driverName.c_str() };
			return stringToBool(executeCallback(console, 19, arguments.size(), arguments.data()));
		}

		void resetUpdateRegion(const std::string& simCanvasName)
		{
			std::array<const char*, 2> arguments{ "resetUpdateRegion", simCanvasName.c_str() };
			executeCallback(console, 20, arguments.size(), arguments.data());
		}

		virtual DARKCALL ~GfxPlugin() override = default;
	};

	struct TerrainPlugin : GamePlugin
	{
		virtual DARKCALL ~TerrainPlugin() override = default;
	};

	struct InteriorPlugin : GamePlugin
	{
		virtual DARKCALL ~InteriorPlugin() override = default;
	};

	struct SkyPlugin : GamePlugin
	{
		virtual DARKCALL ~SkyPlugin() override = default;
	};


	struct StarsiegePlugins
	{

	};


}

#endif
