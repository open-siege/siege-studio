import tkinter.ttk as ttk
import tkinter as tk

def createParentAndLabel(parent, label):
    newFrame = ttk.Frame(parent)
    newFrame.pack(fill=tk.X, expand=True, padx=20, pady=20)

    newLabel = ttk.Label(newFrame, text=label)
    newLabel.pack(fill=tk.X, expand=True)

    return newFrame