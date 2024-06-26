import glob
import os
import sys
import json
import tkinter.messagebox as msg
import importlib.util
from types import SimpleNamespace
from collections import namedtuple

UiModule = namedtuple("UiModule", "name module")

def getDefaultStatus():
    result = None

    recipes = glob.glob("recipes/*.json")

    if len(recipes) > 0:
        result = "readyToInstall"

    if os.path.exists("package.json"):
        result = "readyToRun"

    if result != "readyToRun" and os.path.exists("sspm.config.json"):
        with open("sspm.config.json", "r") as configFile:
            config = json.loads(configFile.read())
            if "defaults" in config and "defaultExe" in config["defaults"]:
                result = "readyToRun" if os.path.exists(config["defaults"]["defaultExe"]) else result

    return result


def setupModule(uiModule, parent=None, model=None):
    result = SimpleNamespace()
    result.moduleLoader = moduleLoader = SimpleNamespace()
    result.commands = commands = SimpleNamespace()
    result.controls = controls = SimpleNamespace()

    moduleLoader.setupModule = setupModule
    moduleLoader.loadAndSetupWidget = loadAndSetupWidget

    if parent is None:
        result.root = root = uiModule.createRootControl(controls)
    else:
        result.root = root = uiModule.createRootControl(controls, parent)

    if model is None:
        result.model = model = SimpleNamespace()
    else:
        result.model = model

    uiModule.setupModel(root, model) if hasattr(uiModule, "setupModel") else None
    uiModule.createControls(root, controls, moduleLoader) if hasattr(uiModule, "createControls") else None
    uiModule.setupCommands(model, controls, commands) if hasattr(uiModule, "setupCommands") else None
    uiModule.bindCommandsToModel(commands, model) if hasattr(uiModule, "bindCommandsToModel") else None
    uiModule.bindControlsToModel(controls, model) if hasattr(uiModule, "bindControlsToModel") else None
    uiModule.bindCommandsToControls(commands, controls) if hasattr(uiModule, "bindCommandsToControls") else None

    return result


def loadAllModules(moduleGroup, moduleType):
    searchGlob = os.path.join("gui", moduleGroup, "**", f"{moduleType}.py")
    files = glob.glob(searchGlob, recursive=True)
    modules = {}

    for file in files:
        (key, module) = loadModuleFromFile(file, moduleGroup, moduleType)
        modules[key] = module
    return modules

def loadAllWindows():
    return loadAllModules("windows", "window")

def loadAllWidgets():
    return loadAllModules("widgets", "widget")

def loadModuleFromFile(file, moduleGroup, moduleType):
    firstPart = os.path.join("gui", moduleGroup, "")
    lastPart = os.path.join(" ", f"{moduleType}.py")
    key = file.replace(firstPart, "").replace(lastPart.strip(), "")
    spec = importlib.util.spec_from_file_location(key, file)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return UiModule._make((key, module))

def loadModule(moduleGroup, moduleType, moduleName):
    file = os.path.join("gui", moduleGroup, moduleName, f"{moduleType}.py")
    return loadModuleFromFile(file, moduleGroup, moduleType)

def loadWindow(windowName):
    return loadModule("windows", "window", windowName)

def loadWidget(widgetName):
    return loadModule("widgets", "widget", widgetName)


def loadAndSetupWidget(widgetName, parent=None, model=None):
    widget = loadWidget(widgetName)
    return setupModule(widget.module, parent, model)


defaultState = getDefaultStatus()

if defaultState is not None:
    mainWindow = loadWindow("mainWindow")

    if mainWindow is None:
        print("Cannot load main window for application, closing.")
        sys.exit()

    main = setupModule(mainWindow.module)
    main.model.currentState.set(defaultState)
    main.root.geometry("900x480")
    main.root.mainloop()
else:
    msg.showerror("No valid config files found", "There are several key files missing for the launcher to run. Please place this executable in the same directory as sspm.config.json or package.json or the recipes folder.")