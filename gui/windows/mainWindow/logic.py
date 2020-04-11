import sspm
import sys
import os
import subprocess
import tkinter as tk
from functools import partial
from threading import Thread


def toggleControlsBasedOnState(model, controls, commands):
    currentState = model.currentState.get()
    if currentState == "installing":
        controls.chkShowMoreOptions.pack_forget()
        controls.pgrProgress.pack(fill=tk.X, expand=True)
        controls.txtOutput.pack(fill=tk.BOTH, expand=True)

        controls.btnInstall.configure(text="Cancel",
                                      command=commands.cancelInstall)
    elif currentState == "readyToInstall":
        controls.btnInstall.configure(text="Install",
                                      state=tk.NORMAL,
                                      command=commands.installGame)
        model.showMore.set(False)
        controls.pgrProgress.pack_forget()
        controls.txtOutput.pack_forget()
        controls.chkShowMoreOptions.pack(fill=tk.BOTH, expand=True)
    elif currentState == "readyToRun":
        controls.btnInstall.configure(text="Run",
                                      command=commands.runGame)
        controls.pgrProgress.pack_forget()
        controls.txtOutput.pack_forget()
        controls.chkShowMoreOptions.pack_forget()
        controls.pnlSettings.pack(fill=tk.BOTH, expand=True)

def toggleMoreVisibility(model, controls, commands):
    if model.showMore.get() is False:
        controls.fraMore.pack_forget()
    else:
        controls.fraMore.pack(fill=tk.BOTH, expand=True)

def printToOutput(model, controls, commands, *a, **b):
    if not model.isCancelled:
        controls.txtOutput.insert(tk.END, f"{''.join(*a)}\n")
        controls.txtOutput.see("end")
        controls.pgrProgress.step()
    else:
        model.currentState.set("readyToInstall")
        sys.exit()

def installPackage(items, installDir, model, newPrint):
    try:
        for package, version in items:
            sspm.installPackagesCore([f"{package}@{version}"], installDir, True, newPrint)
    except Exception as e:
        newPrint(e)
        model.currentState.set("readyToInstall")
        return
    model.currentState.set("readyToRun")


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
    model.currentState.set("installing")
    model.selectedRecipeInfo = selectetedRecipe

    p1 = Thread(target=installPackage, args=(selectetedRecipe["dependencies"].items(), installDir,
                                             model,
                                             partial(printToOutput, model, controls, commands)))
    p1.start()
