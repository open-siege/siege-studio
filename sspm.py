import json
import tarfile
import requests
import os
import os.path
import click
from clint.textui import progress
from os import walk
from multiprocessing.pool import ThreadPool
from time import time as timer
from threading import Lock

s_print_lock = Lock()

def s_print(*a, **b):
    """Thread safe print function"""
    with s_print_lock:
        print(*a, **b)


with open("sspm.config.json", "r") as configFile:
    config = json.loads(configFile.read())

tempDirectory = os.path.join("temp", "packages")
finalDirectory = "."

if not os.path.exists(tempDirectory):
    os.makedirs(tempDirectory)

def getAllFilesFromFolder(folder, files = None):
    if files is None:
        files = []

    for (dirpath, dirnames, filenames) in walk(folder):
        filenames = map(lambda f: os.path.join(dirpath, f), filenames)
        files.extend(filenames)
        for dir in dirnames:
            getAllFilesFromFolder(os.path.join(dirpath, dir), files)

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


def downloadPackageInformation(packageName, version, packages):
    packageInfo = getPackageInfo(packageName)

    if version is None:
        versionToUse = packageInfo["dist-tags"]["latest"]
    else:
        versionToUse = findMatchingVersion(packageInfo, version)

    if versionToUse is None:
        raise RuntimeError(f"Could not find matching version for {packageName} {version}")

    versionInfo = packageInfo["versions"][versionToUse]

    packages.append((packageInfo, versionInfo))
    processDependeciesInformation(packageInfo, versionInfo, packages)

    return packageInfo

def processDependeciesInformation(packageInfo, versionInfo, packages):
    packageInfo["expandedDependencies"] = []
    if "dependencies" in versionInfo:
        for key, value in versionInfo["dependencies"].items():
            childPackage = downloadPackageInformation(key, value, packages)
            packageInfo["expandedDependencies"].append(childPackage)

def extractTar(packageInfo, versionInfo):
    destinationDirectory = os.path.join(tempDirectory, packageInfo["name"])
    s_print(f"extracting {versionInfo['name']}")
    if not os.path.exists(destinationDirectory):
        os.makedirs(destinationDirectory)
    with tarfile.open(os.path.join(tempDirectory, getTarballName(versionInfo))) as tf:
        tf.extractall(path=destinationDirectory)

    packageInfo["extractedFiles"] = getAllFilesFromFolder(destinationDirectory)


def downloadPackageTar(versionInfo):
    if not os.path.exists(os.path.join(tempDirectory, getTarballName(versionInfo))):
        s_print(f"downloading {versionInfo['name']}")
        with open(os.path.join(tempDirectory, getTarballName(versionInfo)), "wb") as tempFile:
            with downloadTarball(versionInfo) as tarballResponse:
                tarLength = int(tarballResponse.headers.get("content-length"))

                if tarLength is None:
                    tempFile.write(tarballResponse.content)
                else:
                    for chunk in tarballResponse.iter_content(chunk_size=4096):
                        if chunk:
                            tempFile.write(chunk)
            tempFile.flush()
    else:
        s_print(f"found cached version of {versionInfo['name']}")


def downloadPackageTarForThread(rawData):
    (packageInfo, versionInfo) = rawData
    downloadPackageTar(versionInfo)
    return rawData

@click.group()
def cli():
    pass


@cli.command("install")
@click.argument("packagename")
def install(packagename):
    packages = []
    finalPackage = downloadPackageInformation(packagename, None, packages)

    pool = ThreadPool(config["numberOfConcurrentDownloads"])
    results = pool.imap_unordered(downloadPackageTarForThread, packages)

    start = timer()
    #progress.bar(, expected_size=(tarLength / 4096) + 1)
    for (packageInfo, versionInfo) in results:
        extractTar(packageInfo, versionInfo)

    s_print(f"Elapsed Time: {timer() - start}")
    copyFilesToFinalFolder(finalPackage)


if __name__ == "__main__":
    cli()
