import glob
import os
import sys
import importlib.util
from types import SimpleNamespace


def setupUiModule(uiModule):
    result = SimpleNamespace()
    result.model = model = SimpleNamespace()
    result.commands = commands = SimpleNamespace()
    result.controls = controls = SimpleNamespace()

    result.root = root = uiModule.createRootControl(controls)

    uiModule.createControls(root, controls)
    uiModule.createModel(root, model)
    uiModule.setupCommands(model, controls, commands)
    uiModule.bindCommandsToModel(commands, model)
    uiModule.bindControlsToModel(controls, model)
    uiModule.bindCommandsToControls(commands, controls)

    return result


searchGlob = os.path.join("gui", "windows", "**", "window.py")
firstPart = os.path.join("gui", "windows", "")
lastPart = os.path.join(" ", "window.py")
files = glob.glob(searchGlob, recursive=True)
modules = {}

for file in files:
    key = file.replace(firstPart, "").replace(lastPart.strip(), "")
    spec = importlib.util.spec_from_file_location(key, file)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    modules[key] = module


mainModule = modules["mainWindow"] if "mainWindow" in modules else None

if mainModule is None:
    print("Cannot load main window for application, closing.")
    sys.exit()

main = setupUiModule(mainModule)
main.root.mainloop()