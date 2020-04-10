import tkinter.ttk as ttk
import tkinter as tk
import tkinter.scrolledtext
import json
import core
import sspm
import sys
import os
import subprocess
from threading import Thread
from functools import partial
from types import SimpleNamespace

def toggleMoreVisibility(model, controls, commands):
    if model.showMore.get() is False:
        controls.fraMore.pack_forget()
    else:
        controls.fraMore.pack(fill=tk.BOTH, expand=True)

def createParentAndLabel(parent, label):
    newFrame = ttk.Frame(parent)
    newFrame.pack(fill=tk.X, expand=True, padx=20, pady=20)

    newLabel = ttk.Label(newFrame, text=label)
    newLabel.pack(fill=tk.X, expand=True)

    return newFrame


def resetToDefaultState(model, controls, commands):
    controls.btnInstall.configure(text="Install",
                                  state=tk.NORMAL,
                                  command=commands.installGame)
    model.showMore.set(False)
    controls.txtOutput.pack_forget()
    controls.chkShowMoreOptions.pack(fill=tk.BOTH, expand=True)

def printToOutput(model, controls, commands, *a, **b):
    if not model.isCancelled:
        controls.txtOutput.insert(tk.END, f"{''.join(*a)}\n")
        controls.txtOutput.see("end")
        controls.pgrProgress.step()
    else:
        commands.resetToDefaultState()
        sys.exit()

def installPackage(items, installDir, commands, newPrint):
    try:
        for package, version in items:
            sspm.installPackagesCore([f"{package}@{version}"], installDir, True, newPrint)
    except Exception as e:
        newPrint(e)
        commands.cancelInstall()
        return
    commands.updateToRunGameControls()

def updateToRunGameControls(model, controls, commands):
    controls.btnInstall.configure(text="Run",
                                  command=commands.runGame)

def runGame(model, controls, commands):
    processToRun = os.path.join(model.installDir.get(), model.selectedRecipeInfo["main"])
    subprocess.Popen([processToRun], cwd=model.installDir.get())

def cancelInstall(model, controls, commands):
    controls.btnInstall.configure(state=tk.DISABLED)
    model.isCancelled = True
    controls.txtOutput.insert(tk.END, f"Cancelling the download process. Just a moment please.")
    controls.txtOutput.see("end")

def installGame(model, controls, commands):
    selectetedRecipe = model.selectedRecipe.get()
    for recipe in model.recipes:
        if recipe["description"] == selectetedRecipe:
            selectetedRecipe = recipe
            break

    installDir = model.installDir.get()
    model.showMore.set(False)
    model.isCancelled = False
    model.selectedRecipeInfo = selectetedRecipe
    controls.chkShowMoreOptions.pack_forget()
    controls.txtOutput.pack(fill=tk.BOTH, expand=True)

    controls.btnInstall.configure(text="Cancel",
                                  command=commands.cancelInstall)

    p1 = Thread(target=installPackage, args=(selectetedRecipe["dependencies"].items(), installDir,
                                             commands,
                                             partial(printToOutput, model, controls, commands)))
    p1.start()


def createRootControl(controls):
    controls.window = tk.Tk()
    controls.window.geometry("720x320")
    controls.window.title("Starsiege Launcher")

    return controls.window

def createModel(root, model: SimpleNamespace):
    defaultInstallDir = "C:\\Dynamix\\Starsiege"
    defaultRecipe = "starsiege-retail-1.0.0-3.en"

    with open("sspm.config.json", "r") as configFile:
        config = json.loads(configFile.read())

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
    commands.installGame = partial(installGame, model, controls, commands)
    commands.toggleMoreVisibility = partial(toggleMoreVisibility, model, controls, commands)
    commands.cancelInstall = partial(cancelInstall, model, controls, commands)
    commands.resetToDefaultState = partial(resetToDefaultState, model, controls, commands)
    commands.updateToRunGameControls = partial(updateToRunGameControls, model, controls, commands)
    commands.runGame = partial(runGame, model, controls, commands)

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

    controls.fraVersion = createParentAndLabel(controls.fraMore, "Version to Install:")
    controls.cmbVersionToInstall = ttk.Combobox(controls.fraVersion)
    controls.cmbVersionToInstall.pack(fill=tk.X, expand=True)

    controls.fraLocation = createParentAndLabel(controls.fraMore, "Install Destination:")
    controls.txtInstallLocation = ttk.Entry(controls.fraLocation)
    controls.txtInstallLocation.pack(fill=tk.X, expand=True)

def bindControlsToModel(controls, model):
    controls.chkShowMoreOptions.configure(variable=model.showMore)
    controls.cmbVersionToInstall.configure(textvariable=model.selectedRecipe, values=model.recipesAsLabels)
    controls.txtInstallLocation.configure(textvariable=model.installDir)

def bindCommandsToControls(commands, controls):
    controls.btnInstall.configure(command=commands.installGame)

def setup():
    result = SimpleNamespace()
    result.model = model = SimpleNamespace()
    result.commands = commands = SimpleNamespace()
    result.controls = controls = SimpleNamespace()

    root = createRootControl(controls)
    createControls(root, controls)
    createModel(root, model)
    setupCommands(model, controls, commands)
    bindCommandsToModel(commands, model)
    bindControlsToModel(controls, model)
    bindCommandsToControls(commands, controls)

    return result


mainWindow = setup()
mainWindow.controls.window.mainloop()
