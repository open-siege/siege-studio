import sspm
import sys
import os
import subprocess
import tkinter as tk
from functools import partial
from threading import Thread

def toggleMoreVisibility(model, controls, commands):
    if model.showMore.get() is False:
        controls.fraMore.pack_forget()
    else:
        controls.fraMore.pack(fill=tk.BOTH, expand=True)

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
