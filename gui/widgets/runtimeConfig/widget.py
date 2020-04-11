import tkinter as tk
import tkinter.scrolledtext
import tkinter.ttk as ttk
import gui.shared as shared
import gui.widgets.runtimeConfig.logic as logic
from functools import partial
from types import SimpleNamespace

def createRootControl(controls, parent):
    controls.frame = ttk.Frame(parent)
    controls.frame.pack_forget()

    return controls.frame

def setupModel(root, model: SimpleNamespace):
    model.resolutions = ["1920x1080", "1600x900", "1440x900", "1366x768", "1280x1024", "1280x960", "1280x720",
                         "1024x768", "800x600", "640x480", "640x400"]
    model.masterServers = [("master.thesiegehub.com", "☑"),("master.siegehub.com", "☑")]
    model.volume = tk.IntVar(root, value=logic.getCurrentCdVolume())
    model.selectedResolution = tk.StringVar(root)
    model.selectedResolution.set(logic.getCurrentResolution())
    model.currentThread = None

def setupCommands(model, controls, commands):
    commands.saveResolutionOptions = partial(logic.saveResolutionOptions, model, controls, commands)
    commands.saveCdVolume = partial(logic.saveCdVolume, model, controls, commands)

def bindControlsToModel(controls, model):
    controls.cmbResolution.configure(textvariable=model.selectedResolution, values=model.resolutions)
    controls.musicVolume.configure(variable=model.volume)
    controls.tblPackages.delete(*controls.tblPackages.get_children())
    for server in model.masterServers:
        controls.tblPackages.insert("", "end", values=server)

def bindCommandsToModel(commands, model):
    model.selectedResolution.trace_variable("w", lambda *args: commands.saveResolutionOptions())
    model.volume.trace_variable("w", lambda *args: commands.saveCdVolume())

def createControls(root, controls, moduleLoader):
    controls.fraResolution = shared.createParentAndLabel(root, "Resolution:")
    controls.cmbResolution = ttk.Combobox(controls.fraResolution)
    controls.cmbResolution.pack(fill=tk.X, expand=True)

    controls.fraMusicVolume = shared.createParentAndLabel(root, "Virtual CD Music Volume:")
    controls.musicVolume = ttk.Scale(controls.fraMusicVolume, from_=0, to=100)
    controls.musicVolume.pack(fill=tk.X, expand=True)

    controls.fraMasterServers = shared.createParentAndLabel(root, "Master Servers:")
    controls.fraMasterServers.pack(fill=tk.BOTH, expand=True)
    cols = ("Domain/Address", "Enabled")
    controls.tblPackages = ttk.Treeview(controls.fraMasterServers, columns=cols, show='headings')
    for col in cols:
        controls.tblPackages.heading(col, text=col)
    controls.tblPackages.column("Enabled", anchor=tk.CENTER)
    controls.tblPackages.pack(fill=tk.BOTH, expand=True)
