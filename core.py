import tarfile
import requests
import os
import hashlib
import base64
import json
from tinydb import TinyDB, where
from os import walk

def createTempDirectory(config):
    tempDirectory = getTempDirectory(config)

    if not os.path.exists(tempDirectory):
        os.makedirs(tempDirectory)

def getTempDirectory(config):
    tempDirectory = os.path.join(config["tempDir"] if "tempDir" in config else "temp", "packages")
    return tempDirectory

def getRecipesDirectory(config):
    tempDirectory = config["recipesDir"] if "recipesDir" in config else "recipes"
    return tempDirectory

def getAllRecipes(config):
    recipesDir = getRecipesDirectory(config)
    files = getAllFilesFromFolder(recipesDir)

    for file in files:
        with open(file, "r") as recipeFile:
            yield json.loads(recipeFile.read())


def getAllFilesFromFolder(folder, files = None):
    if files is None:
        files = set()

    for (dirpath, dirnames, filenames) in walk(folder):
        filenames = map(lambda f: os.path.join(dirpath, f), filenames)
        files.update(filenames)
        for dir in dirnames:
            getAllFilesFromFolder(os.path.join(dirpath, dir), files)

    return files

def getPackageInfo(config, packageName, withoutCache):
    db = TinyDB(config["cacheDbLocation"])
    packages = db.table("packages")
    results = []
    if withoutCache is False:
        results = packages.search(where("name") == packageName)

    if len(results) == 0:
        packageInfoResponse = requests.get(f"{config['packageRegistry']}/{packageName}")
        result = packageInfoResponse.json()

        if "dist-tags" in result and withoutCache is False:
            packages.insert(result)
        elif withoutCache is True:
            packages.remove(where("name") == packageName)
        return result
    else:
        return results[0]

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

def copyFilesToFinalFolder(config, finalDirectory, packageInfo, versionInfo):
    for file in versionInfo["extractedFiles"]:
        fileFolder = file.replace(os.path.join(getTempDirectory(config), f"{packageInfo['name']}-{versionInfo['version']}", "package", ""), "")
        splitData = os.path.split(fileFolder)
        fileFolder = os.path.join(finalDirectory, *splitData[:-1])
        filename: str = splitData[-1]
        if not os.path.exists(fileFolder):
            os.makedirs(fileFolder)
        if filename.endswith("package.json") or filename.startswith("postinstall."):
            continue
        config["localPrint"]("copying  " + file)
        destinationFile = os.path.join(fileFolder, filename)
        os.replace(file, destinationFile)


def downloadPackageInformation(config, packageName, version, packages: dict, withoutCache=False):
    packageInfo = getPackageInfo(config, packageName, withoutCache)

    if version is None:
        versionToUse = packageInfo["dist-tags"]["latest"]
    else:
        versionToUse = findMatchingVersion(packageInfo, version)

    if versionToUse is None:
        if withoutCache is False:
            return downloadPackageInformation(config, packageName, version, packages, True)
        raise RuntimeError(f"Could not find matching version for {packageName} {version}")

    versionInfo = packageInfo["versions"][versionToUse]
    key = f"{packageInfo['name']}@{versionInfo['version']}"

    if key not in packages:
        packages[key] = (packageInfo, versionInfo)
    processDependeciesInformation(config, packageInfo, versionInfo, packages)

    return (packageInfo, versionInfo)

def processDependeciesInformation(config, packageInfo, versionInfo, packages):
    packageInfo["expandedDependencies"] = []
    if "dependencies" in versionInfo:
        for key, value in versionInfo["dependencies"].items():
            childPackage = downloadPackageInformation(config, key, value, packages)
            packageInfo["expandedDependencies"].append(childPackage)

def extractTar(config, packageInfo, versionInfo):
    tempDirectory = getTempDirectory(config)
    destinationDirectory = os.path.join(tempDirectory, f"{packageInfo['name']}-{versionInfo['version']}")
    if not os.path.exists(destinationDirectory):
        os.makedirs(destinationDirectory)
    with tarfile.open(os.path.join(tempDirectory, getTarballName(versionInfo))) as tf:
        tf.extractall(path=destinationDirectory)

    versionInfo["extractedFiles"] = getAllFilesFromFolder(destinationDirectory)


def downloadPackageTar(config, versionInfo):
    downloadPath = os.path.join(getTempDirectory(config), getTarballName(versionInfo))

    if os.path.exists(downloadPath):
        with open(os.path.join(downloadPath), "rb") as tempFile:
            rawData = tempFile.read()
        fileHash = f"sha512-{base64.encodebytes(hashlib.sha512(rawData).digest()).decode('utf8')}"
        fileHash = "".join(fileHash.split("\n"))
        if fileHash != versionInfo["dist"]["integrity"]:
            os.remove(downloadPath)

    if not os.path.exists(downloadPath):
        with open(os.path.join(downloadPath), "wb") as tempFile:
            with downloadTarball(versionInfo) as tarballResponse:
                tarLength = int(tarballResponse.headers.get("content-length"))

                if tarLength is None:
                    tempFile.write(tarballResponse.content)
                else:
                    for chunk in tarballResponse.iter_content(chunk_size=4096):
                        if chunk:
                            tempFile.write(chunk)
            tempFile.flush()


def downloadPackageTarForThread(config, rawData):
    (packageInfo, versionInfo) = rawData
    downloadPackageTar(config, versionInfo)
    return rawData