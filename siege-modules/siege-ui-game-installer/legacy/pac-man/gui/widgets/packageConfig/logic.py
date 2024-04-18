import os
import json

defaultPackageName = "package.json"

def getPackageVersions():
    result = []
    if os.path.exists(defaultPackageName):
        with open(defaultPackageName, "r") as packageFile:
            packageData = json.loads(packageFile.read())

            if "dependencies" in packageData:
                for item in packageData["dependencies"]:
                    result.append((item, packageData["dependencies"][item], "-", "-"))

    return result