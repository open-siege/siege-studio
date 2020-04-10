import tkinter as tk
import tkinter.scrolledtext
import tkinter.ttk as ttk
import json
import core
import gui.shared as shared
import gui.windows.mainWindow.logic as logic
from functools import partial
from types import SimpleNamespace

def createRootControl(controls):
    controls.window = tk.Tk()
    controls.window.geometry("720x320")
    controls.window.title("Starsiege Launcher")

    return controls.window

def createModel(root, model: SimpleNamespace):
    with open("sspm.config.json", "r") as configFile:
        config = json.loads(configFile.read())

    defaultInstallDir = config["defaults"]["installDirectory"]
    defaultRecipe = config["defaults"]["recipe"]

    model.showMore = tk.BooleanVar(root, value=False)
    model.installDir = tk.StringVar(root, value=defaultInstallDir)
    model.selectedRecipe = tk.StringVar(root)
    model.recipes = [*core.getAllRecipes(config)]
    model.recipesAsLabels = [recipe["description"] for recipe in model.recipes]
    model.recipesAsLabels.sort()

    for recipe in model.recipes:
        if recipe["name"] == defaultRecipe:
            model.selectedRecipe.set(recipe["description"])
            break

def setupCommands(model, controls, commands):
    commands.installGame = partial(logic.installGame, model, controls, commands)
    commands.toggleMoreVisibility = partial(logic.toggleMoreVisibility, model, controls, commands)
    commands.cancelInstall = partial(logic.cancelInstall, model, controls, commands)
    commands.resetToDefaultState = partial(logic.resetToDefaultState, model, controls, commands)
    commands.updateToRunGameControls = partial(logic.updateToRunGameControls, model, controls, commands)
    commands.runGame = partial(logic.runGame, model, controls, commands)

def bindCommandsToModel(commands, model):
    model.showMore.trace_variable("w", lambda *args: commands.toggleMoreVisibility())

def createControls(root, controls):
    frame = ttk.Frame(root)
    frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
    controls.btnInstall = ttk.Button(frame, text="Install")
    controls.btnInstall.pack(fill=tk.BOTH, expand=True)

    controls.pgrProgress = pgrProgress = ttk.Progressbar(frame, length=100, orient=tk.HORIZONTAL, mode="determinate")
    pgrProgress.pack(fill=tk.X, expand=True)

    controls.chkShowMoreOptions = ttk.Checkbutton(frame, text="Show More", onvalue=True, offvalue=False)
    controls.chkShowMoreOptions.pack(fill=tk.BOTH, expand=True)

    controls.fraMore = ttk.Labelframe(frame, text="More")
    controls.fraMore.pack_forget()

    controls.txtOutput = tk.scrolledtext.ScrolledText(frame)
    controls.txtOutput.pack_forget()

    controls.fraVersion = shared.createParentAndLabel(controls.fraMore, "Version to Install:")
    controls.cmbVersionToInstall = ttk.Combobox(controls.fraVersion)
    controls.cmbVersionToInstall.pack(fill=tk.X, expand=True)

    controls.fraLocation = shared.createParentAndLabel(controls.fraMore, "Install Destination:")
    controls.txtInstallLocation = ttk.Entry(controls.fraLocation)
    controls.txtInstallLocation.pack(fill=tk.X, expand=True)

def bindControlsToModel(controls, model):
    controls.chkShowMoreOptions.configure(variable=model.showMore)
    controls.cmbVersionToInstall.configure(textvariable=model.selectedRecipe, values=model.recipesAsLabels)
    controls.txtInstallLocation.configure(textvariable=model.installDir)

def bindCommandsToControls(commands, controls):
    controls.btnInstall.configure(command=commands.installGame)
