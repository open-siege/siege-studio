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
    controls.txtOutput.pack_forget()
    controls.chkShowMoreOptions.pack(fill=tk.BOTH, expand=True)
    commands.toggleMoreVisibility()

def printToOutput(model, controls, commands, *a, **b):
    if not model.isCancelled:
        controls.txtOutput.insert(tk.END, f"{''.join(*a)}\n")
        controls.txtOutput.see("end")
        pgrProgress.step()
    else:
        commands.resetToDefaultState()
        sys.exit()

def installPackage(items, installDir, commands, newPrint):
    try:
        for package, version in items:
            sspm.installCore([f"{package}@{version}"], installDir, True, newPrint)
    except Exception as e:
        newPrint(e)
        commands.cancelInstall()
        return
    commands.updateToRunGameControls()

def updateToRunGameControls(model, controls, commands):
    controls.btnInstall.configure(text="Run",
                                  command=commands.runGame)

def runGame(model, controls, commands):
    processToRun = os.path.join(installLocation.get(), model.selectedRecipeInfo["main"])
    subprocess.Popen([processToRun], cwd=installLocation.get())

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

    installDir = installLocation.get()
    model.showMore.set(False)
    model.isCancelled = False
    model.selectedRecipeInfo = selectetedRecipe
    commands.toggleMoreVisibility()
    controls.chkShowMoreOptions.pack_forget()
    controls.txtOutput.pack(fill=tk.BOTH, expand=True)

    controls.btnInstall.configure(text="Cancel",
                                  command=commands.cancelInstall)

    p1 = Thread(target=installPackage, args=(selectetedRecipe["dependencies"].items(), installDir,
                                             commands,
                                             partial(printToOutput, model, controls, commands)))
    p1.start()


model = SimpleNamespace()
commands = SimpleNamespace()
controls = SimpleNamespace()

controls.window = tk.Tk()
controls.window.geometry("720x320")
controls.window.title("Starsiege Launcher")

with open("sspm.config.json", "r") as configFile:
    config = json.loads(configFile.read())

defaultInstallDir = "C:\\Dynamix\\Starsiege"
defaultRecipe = "starsiege-retail-1.0.0-3.en"

model.showMore = tk.BooleanVar(controls.window, value=False)
model.installDir = tk.StringVar(controls.window, value=defaultInstallDir)
model.selectedRecipe = tk.StringVar(controls.window)
model.recipes = [*core.getAllRecipes(config)]
model.recipesAsLabels = [recipe["description"] for recipe in model.recipes]
model.recipesAsLabels.sort()

for recipe in model.recipes:
    if recipe["name"] == defaultRecipe:
        model.selectedRecipe.set(recipe["description"])
        break


commands.installGame = partial(installGame, model, controls, commands)
commands.toggleMoreVisibility = partial(toggleMoreVisibility, model, controls, commands)
commands.cancelInstall = partial(cancelInstall, model, controls, commands)
commands.resetToDefaultState = partial(resetToDefaultState, model, controls, commands)
commands.updateToRunGameControls = partial(updateToRunGameControls, model, controls, commands)
commands.runGame = partial(runGame, model, controls, commands)

frame = ttk.Frame(controls.window)
frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
controls.btnInstall = ttk.Button(frame, text="Install")
controls.btnInstall.pack(fill=tk.BOTH, expand=True)

controls.pgrProgress = pgrProgress = ttk.Progressbar(frame, length=100, orient=tk.HORIZONTAL, mode="determinate")
pgrProgress.pack(fill=tk.X, expand=True)

controls.chkShowMoreOptions = ttk.Checkbutton(frame, text="Show More", variable=model.showMore, onvalue=True, offvalue=False)
controls.chkShowMoreOptions.pack(fill=tk.BOTH, expand=True)
controls.chkShowMoreOptions.configure(command=commands.toggleMoreVisibility)

controls.fraMore = ttk.Labelframe(frame, text="More")
controls.fraMore.pack_forget()

controls.txtOutput = tk.scrolledtext.ScrolledText(frame)
controls.txtOutput.pack_forget()

versionFrame = createParentAndLabel(controls.fraMore, "Version to Install:")
controls.cmbVersionToInstall = ttk.Combobox(versionFrame, textvariable=model.selectedRecipe, values=model.recipesAsLabels)
controls.cmbVersionToInstall.pack(fill=tk.X, expand=True)

locationFrame = createParentAndLabel(controls.fraMore, "Install Destination:")
installLocation = ttk.Entry(locationFrame, textvariable=model.installDir)
installLocation.pack(fill=tk.X, expand=True)

controls.btnInstall.configure(command=commands.installGame)

controls.window.mainloop()
