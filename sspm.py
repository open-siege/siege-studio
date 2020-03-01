import json
import tarfile
import requests
import os
import os.path
from clint.textui import progress
from os import walk

packageName = "starsiege-retail"
with open("sspm.config.json", "r") as configFile:
    config = json.loads(configFile.read())

tempDirectory = os.path.join("temp", "packages")
finalDirectory = "final"

if not os.path.exists(tempDirectory):
    os.makedirs(tempDirectory)

def getAllFilesFromFolder(folder, files = None):
    if files is None:
        files = []

    for (dirpath, dirnames, filenames) in walk(folder):
        filenames = map(lambda f: os.path.join(dirpath, f), filenames)
        files.extend(filenames)
        for dir in dirnames:
            getAllFilesFromFolder(dir, files)

    return files

def getPackageInfo(packageName):
    packageInfoResponse = requests.get(f"{config['packageRegistry']}/{packageName}")
    return packageInfoResponse.json()

def downloadTarball(versionInfo):
    packageInfoResponse = requests.get(versionInfo['dist']['tarball'], stream=True)
    return packageInfoResponse

def getTarballName(versionInfo):
    return os.path.split(versionInfo['dist']['tarball'])[-1]

def findMatchingVersion(packageInfo, version):
    version = version.split("^")[-1]
    for someVersion in packageInfo["versions"]:
        if someVersion == version:
            return someVersion
    return None

def copyFilesToFinalFolder(packageInfo):
    for file in packageInfo["extractedFiles"]:

        print(file)
        fileFolder = file.replace(os.path.join(tempDirectory, packageInfo["name"], "package", ""), "")
        splitData = os.path.split(fileFolder)
        fileFolder = os.path.join(finalDirectory, *splitData[:-1])
        filename = splitData[-1]
        if not os.path.exists(fileFolder):
            os.makedirs(fileFolder)
        if filename != "package.json":
            if not os.path.exists(os.path.join(fileFolder, filename)):
                os.rename(file, os.path.join(fileFolder, filename))
    for child in packageInfo["expandedDependencies"]:
        copyFilesToFinalFolder(child)


def downloadPackageWithDependencies(packageName, version = None):
    destinationDirectory = os.path.join(tempDirectory, packageName)
    if not os.path.exists(destinationDirectory):
        os.makedirs(destinationDirectory)
    packageInfo = getPackageInfo(packageName)

    if version is None:
        versionToUse = packageInfo["dist-tags"]["latest"]
    else:
        versionToUse = findMatchingVersion(packageInfo, version)

    if versionToUse is None:
        raise RuntimeError(f"Could not find matching version for {packageName} {version}")

    print(f"downloading {packageName} {versionToUse}")
    versionInfo = packageInfo["versions"][versionToUse]

    if not os.path.exists(os.path.join(tempDirectory, getTarballName(versionInfo))):
        with open(os.path.join(tempDirectory, getTarballName(versionInfo)), "wb") as tempFile:
            tarballResponse = downloadTarball(versionInfo)
            tarLength = int(tarballResponse.headers.get("content-length"))

            if tarLength is None:
                tempFile.write(tarballResponse.content)
            else:
                for chunk in progress.bar(tarballResponse.iter_content(chunk_size=1024), expected_size=(tarLength / 1024) + 1):
                    if chunk:
                        tempFile.write(chunk)
                        tempFile.flush()


    with tarfile.open(os.path.join(tempDirectory, getTarballName(versionInfo))) as tf:
        tf.extractall(path=destinationDirectory)

    print(destinationDirectory)
    packageInfo["extractedFiles"] = getAllFilesFromFolder(destinationDirectory)

    packageInfo["expandedDependencies"] = []
    if "dependencies" in versionInfo:
        for key, value in versionInfo["dependencies"].items():
            childPackage = downloadPackageWithDependencies(key, value)
            packageInfo["expandedDependencies"].append(childPackage)

    return packageInfo

finalPackage = downloadPackageWithDependencies(packageName)
copyFilesToFinalFolder(finalPackage)
