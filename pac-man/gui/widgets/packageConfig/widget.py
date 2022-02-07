import tkinter as tk
import tkinter.scrolledtext
import tkinter.ttk as ttk
import gui.shared as shared
import gui.widgets.packageConfig.logic as logic
from types import SimpleNamespace

def createRootControl(controls, parent):
    controls.frame = ttk.Frame(parent)
    controls.frame.pack_forget()

    return controls.frame

def setupModel(root, model: SimpleNamespace):
    model.packages = logic.getPackageVersions()

def bindControlsToModel(controls, model):
    controls.tblPackages.delete(*controls.tblPackages.get_children())
    for package in model.packages:
        controls.tblPackages.insert("", "end", values=package)


def createControls(root, controls, moduleLoader):
    controls.fraPackages = shared.createParentAndLabel(root, "Packages:")
    controls.fraPackages.pack(fill=tk.BOTH, expand=True)

    cols = ("Package", "Current Version", "Newer Version", "Update")
    controls.tblPackages = ttk.Treeview(controls.fraPackages, columns=cols, show='headings')
    for col in cols:
        controls.tblPackages.heading(col, text=col)

    controls.tblPackages.pack(fill=tk.BOTH, expand=True)