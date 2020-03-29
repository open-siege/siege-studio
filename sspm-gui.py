import tkinter.ttk as ttk
import tkinter as tk
import tkinter.scrolledtext
import json
import core
import sspm
from threading import Thread
from functools import partial

# Queue must be global

def showHideMore(showMoreVar: tk.BooleanVar, moreFrame: ttk.Labelframe):
    if showMoreVar.get() is False:
        moreFrame.pack_forget()
    else:
        moreFrame.pack(fill=tk.BOTH, expand=True)

def createParentAndLabel(parent, label):
    newFrame = ttk.Frame(parent)
    newFrame.pack(fill=tk.X, expand=True, padx=20, pady=20)

    newLabel = ttk.Label(newFrame, text=label)
    newLabel.pack(fill=tk.X, expand=True)

    return newFrame

def printToOutput(installOutput: tk.scrolledtext.ScrolledText, *a, **b):
    installOutput.insert(tk.INSERT, f"{''.join(*a)}\n")

def installPackage(items, installDir, newPrint):
    for package, version in items:
        sspm.installCore([f"{package}@{version}"], installDir, True, newPrint)

def installGame(recipes, versionToinstall, installLocation, showMoreVar:tk.BooleanVar, boundShowHide, showMoreOptions, installOutput):
    selectetedRecipe = recipes[versionToinstall.current()]
    installDir = installLocation.get()
    showMoreVar.set(False)
    boundShowHide()
    showMoreOptions.pack_forget()
    installOutput.pack(fill=tk.BOTH, expand=True)

    p1 = Thread(target=installPackage, args=(selectetedRecipe["dependencies"].items(), installDir, partial(printToOutput, installOutput)))
    p1.start()


window = tk.Tk()
window.geometry("720x320")
window.title("Starsiege Launcher")

with open("sspm.config.json", "r") as configFile:
    config = json.loads(configFile.read())

recipes = recipes = [*core.getAllRecipes(config)]
recipesAsLabels = [recipe["description"] for recipe in recipes]
defaultInstallDir = "C:\\Dynamix\\Starsiege"
showMoreVar = tk.BooleanVar(window, value=False)
installDirValue = tk.StringVar(window, value=defaultInstallDir)

frame = ttk.Frame(window)
frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
install = ttk.Button(frame, text="Install")
install.pack(fill=tk.BOTH, expand=True)

showMoreOptions = ttk.Checkbutton(frame, text="Show More", variable=showMoreVar, onvalue=True, offvalue=False)
showMoreOptions.pack(fill=tk.BOTH, expand=True)

moreFrame = ttk.Labelframe(frame, text="More")
moreFrame.pack_forget()

installOutput = tk.scrolledtext.ScrolledText(frame)
installOutput.pack_forget()

boundShowHide = partial(showHideMore, showMoreVar, moreFrame)
showMoreOptions.configure(command=boundShowHide)

versionFrame = createParentAndLabel(moreFrame, "Version to Install:")
versionToinstall = ttk.Combobox(versionFrame,
                            values=recipesAsLabels)
versionToinstall.pack(fill=tk.X, expand=True)

locationFrame = createParentAndLabel(moreFrame, "Install Destination:")
installLocation = ttk.Entry(locationFrame, textvariable=installDirValue)
installLocation.pack(fill=tk.X, expand=True)

install.configure(command=partial(installGame, recipes, versionToinstall, installDirValue, showMoreVar, boundShowHide, showMoreOptions, installOutput))

window.mainloop()
