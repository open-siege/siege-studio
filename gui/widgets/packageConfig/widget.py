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

def createControls(root, controls, moduleLoader):
    cols = ('Package', 'Current Version', 'Newer Version', "Update")
    controls.tblPackages = ttk.Treeview(root, columns=cols, show='headings')
    for col in cols:
        controls.tblPackages.heading(col, text=col)

    controls.tblPackages.pack(fill=tk.BOTH, expand=True)
    controls.tblPackages.insert("", "end", values=("starsiege-retail", "1.0.0-3.en", "-", "-"))