import tkinter as tk
import tkinter.scrolledtext
import tkinter.ttk as ttk
import gui.shared as shared
import gui.widgets.runtimeConfig.logic as logic
from types import SimpleNamespace

def createRootControl(controls, parent):
    controls.frame = ttk.Frame(parent)
    controls.frame.pack_forget()

    return controls.frame

def setupModel(root, model: SimpleNamespace):
    pass

def setupCommands(model, controls, commands):
    #commands.installGame = partial(logic.installGame, model, controls, commands)
    pass

def createControls(root, controls, moduleLoader):
    controls.cmbResolution = ttk.Combobox(root)
    controls.cmbResolution.pack(fill=tk.X, expand=True)

    controls.musicVolume = ttk.Scale(root, from_=0, to=100)
    controls.musicVolume.pack(fill=tk.X, expand=True)

def bindCommandsToModel(commands, model):
    #model.showMore.trace_variable("w", lambda *args: commands.toggleMoreVisibility())
    pass

def bindControlsToModel(controls, model):
    #controls.chkShowMoreOptions.configure(variable=model.showMore)
    pass

def bindCommandsToControls(commands, controls):
#    controls.btnInstall.configure(command=commands.installGame)
    pass
