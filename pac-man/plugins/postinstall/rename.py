import os
import json

def canExecute(postInstallValue):
    if postInstallValue == "postinstall.rename.json":
        return True
    return False

def execute(postInstallScriptPath, destDir):
    with open(postInstallScriptPath, "r") as scriptFile:
        config = json.loads(scriptFile.read())
        for originalName, newName in config.items():
            if os.path.exists(os.path.join(destDir, originalName)):
                print(f"renaming {originalName} to {newName}")
                os.replace(os.path.join(destDir, originalName), os.path.join(destDir, newName))
            else:
                print("cannot find " + originalName)



