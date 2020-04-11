import os
import time
from threading import Thread

prefsFilename = "defaultPrefs.cs"
widthKey = "$pref::GWC::SIM_FS_WIDTH"
heightKey = "$pref::GWC::SIM_FS_HEIGHT"

dxWndFilename = "dxwnd.dxw"
cdVolumeKey = "cdvol0"

def saveCdVolumeToFile(model):
    time.sleep(0.5)
    volume: int = model.volume.get()
    if os.path.exists(dxWndFilename):
        with open(dxWndFilename, "r") as prefsFile:
            data = prefsFile.read()
            oldData = "" + data
            existingData = getExistingData(data, cdVolumeKey, "\n")
            if existingData is not None:
                data = data.replace(existingData, f"{cdVolumeKey}={volume}")

        if data != oldData:
            with open(dxWndFilename, "w") as prefsFile:
                prefsFile.write(data)
    model.currentThread = None

def saveCdVolume(model, controls, commands):
    if model.currentThread is None:
        model.currentThread = Thread(target=saveCdVolumeToFile, args=[model])
        model.currentThread.start()

def getCurrentCdVolume():
    result = 50

    if os.path.exists(dxWndFilename):
        with open(dxWndFilename, "r") as prefsFile:
            data = prefsFile.read()
        result = getSettingIntValue(getExistingData(data, cdVolumeKey, "\n"), result)

    return result

def getExistingData(data, key, endTerminator=";"):
    widthIndex = data.index(key)

    if widthIndex != -1:
        endIndex = data.index(endTerminator, widthIndex)
        if endIndex != -1:
            return data[widthIndex:endIndex]
    return None

def getSettingIntValue(oldData, default=None):
    if oldData is None:
        return default
    oldData = oldData.split("=")
    return int(oldData[-1].replace("\"", ""))

def getCurrentResolution():
    width = 1920
    height = 1080

    if os.path.exists(prefsFilename):
        with open(prefsFilename, "r") as prefsFile:
            data = prefsFile.read()
        width = getSettingIntValue(getExistingData(data, widthKey), width)
        height = getSettingIntValue(getExistingData(data, heightKey), height)

    return f"{width}x{height}"


def saveResolutionOptions(model, controls, commands):
    newResolution: str = model.selectedResolution.get()
    values = newResolution.split("x")
    width = int(values[0])
    height = int(values[1])
    if os.path.exists(prefsFilename):
        with open(prefsFilename, "r") as prefsFile:
            data = prefsFile.read()
            oldData = "" + data

            widthSetting = getExistingData(data, widthKey)
            if widthSetting is not None:
                data = data.replace(widthSetting, f"{widthKey} = \"{width}\"")

            heightSetting = getExistingData(data, heightKey)
            if heightSetting is not None:
                data = data.replace(heightSetting, f"{heightKey} = \"{height}\"")

        if data != oldData:
            with open("defaultPrefs.cs", "w") as prefsFile:
                prefsFile.write(data)