import glob
import os
import sys
import importlib.util
from types import SimpleNamespace
from collections import namedtuple

UiModule = namedtuple("UiModule", "name module")

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

mainWindow = loadWindow("mainWindow")

if mainWindow is None:
    print("Cannot load main window for application, closing.")
    sys.exit()

main = setupModule(mainWindow.module)
main.model.currentState.set("readyToRun")
main.root.geometry("900x480")
main.root.mainloop()